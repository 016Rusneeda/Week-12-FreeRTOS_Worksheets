#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "SW_TIMERS";

// LED pins for different timers
#define LED_BLINK     GPIO_NUM_2      // Fast blink timer
#define LED_HEARTBEAT GPIO_NUM_4      // Heartbeat timer
#define LED_STATUS    GPIO_NUM_5      // Status timer
#define LED_ONESHOT   GPIO_NUM_18     // One-shot timer

// Timer periods (in milliseconds)
#define BLINK_PERIOD      500
#define HEARTBEAT_PERIOD  2000
#define STATUS_PERIOD     5000
#define ONESHOT_DELAY     3000

// ------------- Stats -------------
typedef struct {
    uint32_t blink_count;
    uint32_t heartbeat_count;
    uint32_t status_count;
    uint32_t oneshot_count;
    uint32_t dynamic_count;
} timer_stats_t;
static timer_stats_t stats = {0};

// ------------- LED states -------------
static bool led_blink_state = false;

// ------------- Timers -------------
static TimerHandle_t xBlinkTimer;
static TimerHandle_t xHeartbeatTimer;
static TimerHandle_t xStatusTimer;
static TimerHandle_t xOneShotTimer;
static TimerHandle_t xDynamicTimer = NULL;

// ------------- Event queue (worker) -------------
typedef enum {
    EV_BLINK_TOGGLE,
    EV_HEARTBEAT_BEAT,   // double blink pattern
    EV_STATUS_FLASH,     // short flash
    EV_ONESHOT_FIRE,     // 5 quick flashes
    EV_DYNAMIC_FLASH     // flash all LEDs briefly
} event_type_t;

typedef struct {
    event_type_t type;
    uint32_t     param; // optional (e.g., duration)
} timer_event_t;

static QueueHandle_t xTimerEventQueue;

// ========= Forward declarations for callbacks (FIX compile error) =========
static void dynamic_timer_callback(TimerHandle_t xTimer);
static void blink_timer_callback(TimerHandle_t xTimer);
static void heartbeat_timer_callback(TimerHandle_t xTimer);
static void status_timer_callback(TimerHandle_t xTimer);
static void oneshot_timer_callback(TimerHandle_t xTimer);

// ========= Worker task =========
static void timer_worker_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Timer worker task started");
    timer_event_t ev;

    for (;;) {
        if (xQueueReceive(xTimerEventQueue, &ev, portMAX_DELAY) == pdTRUE) {
            switch (ev.type) {
            case EV_BLINK_TOGGLE:
                // toggle LED_BLINK
                led_blink_state = !led_blink_state;
                gpio_set_level(LED_BLINK, led_blink_state);
                break;

            case EV_HEARTBEAT_BEAT:
                // Double blink pattern: on-off-on-off (100ms pace)
                gpio_set_level(LED_HEARTBEAT, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_HEARTBEAT, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_HEARTBEAT, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_HEARTBEAT, 0);
                break;

            case EV_STATUS_FLASH:
                // Single short flash (200ms)
                gpio_set_level(LED_STATUS, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_set_level(LED_STATUS, 0);
                break;

            case EV_ONESHOT_FIRE:
                // 5 quick flashes (50ms)
                for (int i = 0; i < 5; i++) {
                    gpio_set_level(LED_ONESHOT, 1);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    gpio_set_level(LED_ONESHOT, 0);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                break;

            case EV_DYNAMIC_FLASH:
                // Flash all LEDs briefly (300ms), then restore blink LED state
                gpio_set_level(LED_BLINK,   1);
                gpio_set_level(LED_HEARTBEAT, 1);
                gpio_set_level(LED_STATUS,  1);
                gpio_set_level(LED_ONESHOT, 1);
                vTaskDelay(pdMS_TO_TICKS(300));
                gpio_set_level(LED_BLINK,   led_blink_state);
                gpio_set_level(LED_HEARTBEAT, 0);
                gpio_set_level(LED_STATUS,  0);
                gpio_set_level(LED_ONESHOT, 0);
                break;

            default:
                break;
            }
        }
    }
}

// ========= Timer callbacks (NO delays inside) =========
static void blink_timer_callback(TimerHandle_t xTimer)
{
    stats.blink_count++;

    // Log (lightweight)
    ESP_LOGI(TAG, "💫 Blink Timer: Toggle #%lu", (unsigned long)stats.blink_count);

    // Send event to worker to actually toggle the LED (with no delay here)
    timer_event_t ev = {.type = EV_BLINK_TOGGLE, .param = 0};
    (void)xQueueSend(xTimerEventQueue, &ev, 0);

    // Every 20 blinks, start one-shot timer (allowed from callback with 0 block time)
    if ((stats.blink_count % 20) == 0) {
        ESP_LOGI(TAG, "🚀 Starting one-shot timer (3 second delay)");
        if (xTimerStart(xOneShotTimer, 0) != pdPASS) {
            ESP_LOGW(TAG, "Failed to start one-shot timer");
        }
    }
}

static void heartbeat_timer_callback(TimerHandle_t xTimer)
{
    stats.heartbeat_count++;
    ESP_LOGI(TAG, "💓 Heartbeat Timer: Beat #%lu", (unsigned long)stats.heartbeat_count);

    // Ask worker to execute double blink (no blocking here)
    timer_event_t ev = {.type = EV_HEARTBEAT_BEAT, .param = 0};
    (void)xQueueSend(xTimerEventQueue, &ev, 0);

    // Randomly adjust blink timer period (allowed from callback, block=0)
    if ((esp_random() % 4) == 0) { // 25% chance
        uint32_t new_period_ms = 300 + (esp_random() % 400); // 300-700ms
        ESP_LOGI(TAG, "🔧 Adjusting blink period to %lums", (unsigned long)new_period_ms);
        if (xTimerChangePeriod(xBlinkTimer, pdMS_TO_TICKS(new_period_ms), 0) != pdPASS) {
            ESP_LOGW(TAG, "Failed to change blink timer period");
        }
    }
}

static void status_timer_callback(TimerHandle_t xTimer)
{
    stats.status_count++;

    // Print statistics (ok, no blocking)
    ESP_LOGI(TAG, "📊 Status Timer: Update #%lu", (unsigned long)stats.status_count);
    ESP_LOGI(TAG, "═══ TIMER STATISTICS ═══");
    ESP_LOGI(TAG, "Blink events:     %lu", (unsigned long)stats.blink_count);
    ESP_LOGI(TAG, "Heartbeat events: %lu", (unsigned long)stats.heartbeat_count);
    ESP_LOGI(TAG, "Status updates:   %lu", (unsigned long)stats.status_count);
    ESP_LOGI(TAG, "One-shot events:  %lu", (unsigned long)stats.oneshot_count);
    ESP_LOGI(TAG, "Dynamic events:   %lu", (unsigned long)stats.dynamic_count);
    ESP_LOGI(TAG, "═══════════════════════");

    // Show timer states and periods
    ESP_LOGI(TAG, "Timer States:");
    ESP_LOGI(TAG, "  Blink:     %s (Period: %lums)",
             xTimerIsTimerActive(xBlinkTimer) ? "ACTIVE" : "INACTIVE",
             (unsigned long)(xTimerGetPeriod(xBlinkTimer) * portTICK_PERIOD_MS));
    ESP_LOGI(TAG, "  Heartbeat: %s (Period: %lums)",
             xTimerIsTimerActive(xHeartbeatTimer) ? "ACTIVE" : "INACTIVE",
             (unsigned long)(xTimerGetPeriod(xHeartbeatTimer) * portTICK_PERIOD_MS));
    ESP_LOGI(TAG, "  Status:    %s (Period: %lums)",
             xTimerIsTimerActive(xStatusTimer) ? "ACTIVE" : "INACTIVE",
             (unsigned long)(xTimerGetPeriod(xStatusTimer) * portTICK_PERIOD_MS));
    ESP_LOGI(TAG, "  One-shot:  %s",
             xTimerIsTimerActive(xOneShotTimer) ? "ACTIVE" : "INACTIVE");

    // Ask worker to do the short status flash
    timer_event_t ev = {.type = EV_STATUS_FLASH, .param = 0};
    (void)xQueueSend(xTimerEventQueue, &ev, 0);
}

static void oneshot_timer_callback(TimerHandle_t xTimer)
{
    stats.oneshot_count++;
    ESP_LOGI(TAG, "⚡ One-shot Timer: Event #%lu", (unsigned long)stats.oneshot_count);

    // Ask worker to perform the 5 quick flashes
    timer_event_t ev = {.type = EV_ONESHOT_FIRE, .param = 0};
    (void)xQueueSend(xTimerEventQueue, &ev, 0);

    // Create a dynamic timer with random period (1-4 sec), one-shot
    uint32_t random_period_ms = 1000 + (esp_random() % 3000);
    ESP_LOGI(TAG, "🎲 Creating dynamic timer (period: %lums)", (unsigned long)random_period_ms);

    xDynamicTimer = xTimerCreate("DynamicTimer",
                                 pdMS_TO_TICKS(random_period_ms),
                                 pdFALSE,        // One-shot
                                 (void*)0,
                                 dynamic_timer_callback);
    if (xDynamicTimer != NULL) {
        if (xTimerStart(xDynamicTimer, 0) != pdPASS) {
            ESP_LOGW(TAG, "Failed to start dynamic timer");
        }
    }
}

static void dynamic_timer_callback(TimerHandle_t xTimer)
{
    stats.dynamic_count++;
    ESP_LOGI(TAG, "🌟 Dynamic Timer: Event #%lu", (unsigned long)stats.dynamic_count);

    // Ask worker to flash all LEDs briefly
    timer_event_t ev = {.type = EV_DYNAMIC_FLASH, .param = 0};
    (void)xQueueSend(xTimerEventQueue, &ev, 0);

    // Delete this dynamic timer (allowed from callback, block=0)
    if (xTimerDelete(xTimer, 0) != pdPASS) {
        ESP_LOGW(TAG, "Failed to delete dynamic timer");
    } else {
        ESP_LOGI(TAG, "Dynamic timer deleted");
    }
    xDynamicTimer = NULL;
}

// ========= Control task (random maintenance) =========
static void timer_control_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Timer control task started");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(15000)); // Every 15 seconds

        ESP_LOGI(TAG, "\n🎛️  TIMER CONTROL: Performing maintenance...");
        int action = esp_random() % 3;

        switch (action) {
        case 0:
            ESP_LOGI(TAG, "⏸️  Stopping heartbeat timer for 5 seconds");
            (void)xTimerStop(xHeartbeatTimer, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(5000));
            ESP_LOGI(TAG, "▶️  Restarting heartbeat timer");
            (void)xTimerStart(xHeartbeatTimer, portMAX_DELAY);
            break;

        case 1:
            ESP_LOGI(TAG, "🔄 Reset status timer");
            (void)xTimerReset(xStatusTimer, portMAX_DELAY);
            break;

        case 2: {
            ESP_LOGI(TAG, "⚙️  Changing blink timer period");
            uint32_t new_period = 200 + (esp_random() % 600); // 200-800ms
            (void)xTimerChangePeriod(xBlinkTimer, pdMS_TO_TICKS(new_period), portMAX_DELAY);
            ESP_LOGI(TAG, "New blink period: %lums", (unsigned long)new_period);
            break;
        }
        default:
            break;
        }
        ESP_LOGI(TAG, "Maintenance completed\n");
    }
}

// ========= app_main =========
void app_main(void)
{
    ESP_LOGI(TAG, "Software Timers Lab Starting...");

    // Configure LED pins
    gpio_set_direction(LED_BLINK,     GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_HEARTBEAT, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_STATUS,    GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ONESHOT,   GPIO_MODE_OUTPUT);

    // Turn off all LEDs
    gpio_set_level(LED_BLINK,     0);
    gpio_set_level(LED_HEARTBEAT, 0);
    gpio_set_level(LED_STATUS,    0);
    gpio_set_level(LED_ONESHOT,   0);

    // Create event queue for worker
    xTimerEventQueue = xQueueCreate(16, sizeof(timer_event_t));
    if (xTimerEventQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create timer event queue");
        return;
    }

    // Create worker task
    xTaskCreate(timer_worker_task, "TimerWorker", 3072, NULL, 3, NULL);

    ESP_LOGI(TAG, "Creating software timers...");

    // Create timers (auto-reload except one-shot)
    xBlinkTimer = xTimerCreate("BlinkTimer",
                               pdMS_TO_TICKS(BLINK_PERIOD),
                               pdTRUE,   // Auto-reload
                               (void*)1,
                               blink_timer_callback);

    xHeartbeatTimer = xTimerCreate("HeartbeatTimer",
                                   pdMS_TO_TICKS(HEARTBEAT_PERIOD),
                                   pdTRUE, // Auto-reload
                                   (void*)2,
                                   heartbeat_timer_callback);

    xStatusTimer = xTimerCreate("StatusTimer",
                                pdMS_TO_TICKS(STATUS_PERIOD),
                                pdTRUE,  // Auto-reload
                                (void*)3,
                                status_timer_callback);

    xOneShotTimer = xTimerCreate("OneShotTimer",
                                 pdMS_TO_TICKS(ONESHOT_DELAY),
                                 pdFALSE, // One-shot
                                 (void*)4,
                                 oneshot_timer_callback);

    if (xBlinkTimer && xHeartbeatTimer && xStatusTimer && xOneShotTimer) {
        ESP_LOGI(TAG, "All timers created successfully");

        // Start periodic timers
        ESP_LOGI(TAG, "Starting timers...");
        (void)xTimerStart(xBlinkTimer,     0);
        (void)xTimerStart(xHeartbeatTimer, 0);
        (void)xTimerStart(xStatusTimer,    0);
        // one-shot will be started by blink callback every 20 toggles

        // Control task
        xTaskCreate(timer_control_task, "TimerControl", 2048, NULL, 2, NULL);

        ESP_LOGI(TAG, "Timer system operational!");
        ESP_LOGI(TAG, "LED map:");
        ESP_LOGI(TAG, "  GPIO2  - Blink (toggles ~500ms)");
        ESP_LOGI(TAG, "  GPIO4  - Heartbeat (double blink every ~2s)");
        ESP_LOGI(TAG, "  GPIO5  - Status (flash every ~5s + stats log)");
        ESP_LOGI(TAG, "  GPIO18 - One-shot (5 quick flashes when triggered)");
    } else {
        ESP_LOGE(TAG, "Failed to create one or more timers");
        ESP_LOGE(TAG, "Check CONFIG_FREERTOS_USE_TIMERS=y in sdkconfig");
    }

    // Startup LED sequence (optional, runs in app_main task)
    for (int i = 0; i < 2; i++) {
        gpio_set_level(LED_BLINK, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(LED_HEARTBEAT, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(LED_STATUS, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(LED_ONESHOT, 1);
        vTaskDelay(pdMS_TO_TICKS(300));
        // off
        gpio_set_level(LED_BLINK, 0);
        gpio_set_level(LED_HEARTBEAT, 0);
        gpio_set_level(LED_STATUS, 0);
        gpio_set_level(LED_ONESHOT, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
