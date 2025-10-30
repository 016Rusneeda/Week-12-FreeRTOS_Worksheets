#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_random.h"
#include "esp_system.h"

static const char *TAG = "TIMER_APPS";

// ===== Pins =====
#define STATUS_LED       GPIO_NUM_2
#define WATCHDOG_LED     GPIO_NUM_4
#define PATTERN_LED_1    GPIO_NUM_5
#define PATTERN_LED_2    GPIO_NUM_18
#define PATTERN_LED_3    GPIO_NUM_19
#define SENSOR_POWER     GPIO_NUM_21
// NOTE: ADC1_CHANNEL_0 = GPIO36 (ไม่ต้อง define SENSOR_PIN แยก)

// ===== Periods (ms) =====
#define WATCHDOG_TIMEOUT_MS     5000
#define WATCHDOG_FEED_MS        2000
#define PATTERN_BASE_MS         500
#define SENSOR_SAMPLE_MS        1000
#define STATUS_UPDATE_MS        3000

// ===== Patterns =====
typedef enum {
    PATTERN_OFF = 0,
    PATTERN_SLOW_BLINK,
    PATTERN_FAST_BLINK,
    PATTERN_HEARTBEAT,
    PATTERN_SOS,
    PATTERN_RAINBOW,
    PATTERN_MAX
} led_pattern_t;

// ===== Sensor/Data =====
typedef struct {
    float value;
    uint32_t timestamp_ms;
    bool valid;
} sensor_data_t;

// ===== Health =====
typedef struct {
    uint32_t watchdog_feeds;
    uint32_t watchdog_timeouts;
    uint32_t pattern_changes;
    uint32_t sensor_readings;
    uint32_t system_uptime_sec;
    bool system_healthy;
} system_health_t;

// ===== Timers =====
static TimerHandle_t watchdog_timer;
static TimerHandle_t feed_timer;
static TimerHandle_t pattern_timer;
static TimerHandle_t sensor_timer;
static TimerHandle_t status_timer;

// ===== Queues =====
static QueueHandle_t event_queue;     // คิวรวม event จาก timers → worker
static QueueHandle_t sensor_out_q;    // ผล sensor (optional)

// ===== Pattern State =====
static led_pattern_t current_pattern = PATTERN_OFF;
static int pattern_step = 0;
typedef struct {
    int step;
    bool state;
} pattern_state_t;
static pattern_state_t pattern_state = {0, false};

// ===== Health/ADC =====
static system_health_t health_stats = {0,0,0,0,0,true};
static esp_adc_cal_characteristics_t *adc_chars = NULL;

// ===== Event System =====
typedef enum {
    EV_WATCHDOG_TIMEOUT,
    EV_WATCHDOG_FEED_FLASH,
    EV_PATTERN_TICK,
    EV_STATUS_TICK,
    EV_SENSOR_TICK,
    EV_CHANGE_PATTERN,        // param = new pattern
} event_type_t;

typedef struct {
    event_type_t type;
    uint32_t     param;
} app_event_t;

// ====== Forward Declarations (สำคัญ) ======
static void recovery_callback(TimerHandle_t t);
static void change_led_pattern(led_pattern_t new_pattern);
static float read_sensor_value(void);

// ====== Utility ======
static inline uint32_t ticks_to_ms(TickType_t t) {
    return t * portTICK_PERIOD_MS;
}

static void set_pattern_leds(bool l1, bool l2, bool l3) {
    gpio_set_level(PATTERN_LED_1, l1);
    gpio_set_level(PATTERN_LED_2, l2);
    gpio_set_level(PATTERN_LED_3, l3);
}

// ===================== TIMER CALLBACKS (no delay, no heavy work) =====================
static void watchdog_timeout_callback(TimerHandle_t timer) {
    // ส่งอีเวนต์ให้ worker แทนการกระพริบทันที
    app_event_t ev = {.type = EV_WATCHDOG_TIMEOUT, .param = 0};
    (void)xQueueSend(event_queue, &ev, 0);
}

static void feed_watchdog_callback(TimerHandle_t timer) {
    static int feed_count = 0;
    feed_count++;

    if (feed_count == 15) { // จำลองแฮงก์ 8 วิ
        ESP_LOGW(TAG, "🐛 Simulating system hang - stopping watchdog feeds for 8 seconds");
        (void)xTimerStop(feed_timer, 0);

        TimerHandle_t recovery_timer = xTimerCreate("Recovery",
                                                    pdMS_TO_TICKS(8000),
                                                    pdFALSE,
                                                    (void*)0,
                                                    recovery_callback);
        if (recovery_timer) (void)xTimerStart(recovery_timer, 0);
        return;
    }

    health_stats.watchdog_feeds++;
    ESP_LOGI(TAG, "🍖 Feeding watchdog (feed #%lu)", (unsigned long)health_stats.watchdog_feeds);

    // รีเซ็ต watchdog
    (void)xTimerReset(watchdog_timer, 0);

    // ให้ worker กระพริบ STATUS_LED 50ms
    app_event_t ev = {.type = EV_WATCHDOG_FEED_FLASH, .param = 50};
    (void)xQueueSend(event_queue, &ev, 0);
}

static void pattern_timer_callback(TimerHandle_t timer) {
    app_event_t ev = {.type = EV_PATTERN_TICK, .param = 0};
    (void)xQueueSend(event_queue, &ev, 0);
}

static void sensor_timer_callback(TimerHandle_t timer) {
    app_event_t ev = {.type = EV_SENSOR_TICK, .param = 0};
    (void)xQueueSend(event_queue, &ev, 0);
}

static void status_timer_callback(TimerHandle_t timer) {
    app_event_t ev = {.type = EV_STATUS_TICK, .param = 0};
    (void)xQueueSend(event_queue, &ev, 0);
}

// ===================== WORKER TASK =====================
static void worker_task(void *arg) {
    ESP_LOGI(TAG, "Worker task started");
    app_event_t ev;
    float temp_sum = 0.0f;
    int sample_count = 0;

    const char* pattern_names[] = {
        "OFF", "SLOW_BLINK", "FAST_BLINK", "HEARTBEAT", "SOS", "RAINBOW"
    };

    while (1) {
        if (xQueueReceive(event_queue, &ev, portMAX_DELAY) != pdTRUE) continue;

        switch (ev.type) {
        case EV_WATCHDOG_TIMEOUT: {
            health_stats.watchdog_timeouts++;
            health_stats.system_healthy = false;
            ESP_LOGE(TAG, "🚨 WATCHDOG TIMEOUT! Feeds=%lu Timeouts=%lu",
                     (unsigned long)health_stats.watchdog_feeds,
                     (unsigned long)health_stats.watchdog_timeouts);

            // flash WATCHDOG_LED เร็ว 10 ครั้ง (ทำใน task ได้)
            for (int i = 0; i < 10; i++) {
                gpio_set_level(WATCHDOG_LED, 1);
                vTaskDelay(pdMS_TO_TICKS(50));
                gpio_set_level(WATCHDOG_LED, 0);
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            ESP_LOGW(TAG, "In production: esp_restart() would be called here");
            (void)xTimerReset(watchdog_timer, 0);
            health_stats.system_healthy = true;
            break;
        }

        case EV_WATCHDOG_FEED_FLASH: {
            // flash STATUS_LED นิดเดียว
            uint32_t dur = ev.param;
            gpio_set_level(STATUS_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(dur));
            gpio_set_level(STATUS_LED, 0);
            break;
        }

        case EV_PATTERN_TICK: {
            static uint32_t pattern_cycle = 0;
            pattern_cycle++;

            switch (current_pattern) {
            case PATTERN_OFF:
                set_pattern_leds(0,0,0);
                (void)xTimerChangePeriod(pattern_timer, pdMS_TO_TICKS(1000), portMAX_DELAY);
                break;

            case PATTERN_SLOW_BLINK:
                pattern_state.state = !pattern_state.state;
                set_pattern_leds(pattern_state.state, 0, 0);
                (void)xTimerChangePeriod(pattern_timer, pdMS_TO_TICKS(1000), portMAX_DELAY);
                ESP_LOGI(TAG, "💡 Slow Blink: %s", pattern_state.state ? "ON" : "OFF");
                break;

            case PATTERN_FAST_BLINK:
                pattern_state.state = !pattern_state.state;
                set_pattern_leds(0, pattern_state.state, 0);
                (void)xTimerChangePeriod(pattern_timer, pdMS_TO_TICKS(200), portMAX_DELAY);
                break;

            case PATTERN_HEARTBEAT: {
                int step = pattern_step % 10;
                bool pulse = (step < 2) || (step >= 3 && step < 5);
                set_pattern_leds(0, 0, pulse);
                pattern_step++;
                (void)xTimerChangePeriod(pattern_timer, pdMS_TO_TICKS(100), portMAX_DELAY);
                if (step == 9) ESP_LOGI(TAG, "💓 Heartbeat pulse");
                break;
            }

            case PATTERN_SOS: {
                static const char* sos = "...---...";
                static int sos_pos = 0;
                bool is_dash = (sos[sos_pos] == '-');

                // dot=200ms, dash=600ms
                int duration = is_dash ? 600 : 200;
                set_pattern_leds(1,1,1);
                vTaskDelay(pdMS_TO_TICKS(duration));
                set_pattern_leds(0,0,0);

                sos_pos = (sos_pos + 1) % (int)strlen(sos);
                if (sos_pos == 0) {
                    ESP_LOGI(TAG, "🆘 SOS Pattern Complete");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
                // schedule next tick ASAP
                (void)xTimerChangePeriod(pattern_timer, 1, portMAX_DELAY);
                break;
            }

            case PATTERN_RAINBOW: {
                int rainbow_step = pattern_step % 8;
                bool l1 = (rainbow_step & 1) != 0;
                bool l2 = (rainbow_step & 2) != 0;
                bool l3 = (rainbow_step & 4) != 0;
                set_pattern_leds(l1, l2, l3);
                pattern_step++;
                if (rainbow_step == 7) ESP_LOGI(TAG, "🌈 Rainbow cycle complete");
                (void)xTimerChangePeriod(pattern_timer, pdMS_TO_TICKS(300), portMAX_DELAY);
                break;
            }

            default:
                set_pattern_leds(0,0,0);
                break;
            }

            // เปลี่ยน pattern ทุก 50 รอบ
            if ((pattern_cycle % 50) == 0) {
                led_pattern_t np = (current_pattern + 1) % PATTERN_MAX;
                ESP_LOGI(TAG, "🎨 Changing pattern: %s -> %s",
                         pattern_names[current_pattern], pattern_names[np]);
                change_led_pattern(np);
            }
            break;
        }

        case EV_SENSOR_TICK: {
            // อ่าน sensor ใน task (ปลอดภัย)
            float v = read_sensor_value();
            uint32_t now_ms = ticks_to_ms(xTaskGetTickCount());
            bool valid = (v >= 0 && v <= 50);

            if (valid) {
                health_stats.sensor_readings++;
                ESP_LOGI(TAG, "🌡️ Sensor: %.2f°C at %lu ms", v, (unsigned long)now_ms);

                // moving average 10 ตัวอย่าง
                temp_sum += v; sample_count++;
                if (sample_count >= 10) {
                    float avg = temp_sum / (float)sample_count;
                    ESP_LOGI(TAG, "📊 Temperature Average: %.2f°C", avg);

                    if (avg > 35.0f) {
                        ESP_LOGW(TAG, "🔥 High temperature warning!");
                        change_led_pattern(PATTERN_FAST_BLINK);
                    } else if (avg < 15.0f) {
                        ESP_LOGW(TAG, "🧊 Low temperature warning!");
                        change_led_pattern(PATTERN_SOS);
                    }
                    temp_sum = 0.0f; sample_count = 0;
                }
            } else {
                ESP_LOGW(TAG, "Invalid sensor reading: %.2f", v);
            }

            // Adaptive sampling period
            TickType_t newp;
            if (v > 40.0f) newp = pdMS_TO_TICKS(500);
            else if (v > 25.0f) newp = pdMS_TO_TICKS(1000);
            else newp = pdMS_TO_TICKS(2000);
            (void)xTimerChangePeriod(sensor_timer, newp, portMAX_DELAY);

            // ส่งออกผล (optional)
            if (sensor_out_q) {
                sensor_data_t out = {.value=v, .timestamp_ms=now_ms, .valid=valid};
                (void)xQueueSend(sensor_out_q, &out, 0);
            }
            break;
        }

        case EV_STATUS_TICK: {
            health_stats.system_uptime_sec = ticks_to_ms(xTaskGetTickCount()) / 1000;
            ESP_LOGI(TAG, "\n═══════ SYSTEM STATUS ═══════");
            ESP_LOGI(TAG, "Uptime: %lu seconds", (unsigned long)health_stats.system_uptime_sec);
            ESP_LOGI(TAG, "System Health: %s", health_stats.system_healthy ? "✅ HEALTHY" : "❌ ISSUES");
            ESP_LOGI(TAG, "Watchdog Feeds: %lu", (unsigned long)health_stats.watchdog_feeds);
            ESP_LOGI(TAG, "Watchdog Timeouts: %lu", (unsigned long)health_stats.watchdog_timeouts);
            ESP_LOGI(TAG, "Pattern Changes: %lu", (unsigned long)health_stats.pattern_changes);
            ESP_LOGI(TAG, "Sensor Readings: %lu", (unsigned long)health_stats.sensor_readings);
            ESP_LOGI(TAG, "Current Pattern: %d", current_pattern);
            ESP_LOGI(TAG, "Timer States:");
            ESP_LOGI(TAG, "  Watchdog: %s", xTimerIsTimerActive(watchdog_timer) ? "ACTIVE" : "INACTIVE");
            ESP_LOGI(TAG, "  Feed:     %s", xTimerIsTimerActive(feed_timer) ? "ACTIVE" : "INACTIVE");
            ESP_LOGI(TAG, "  Pattern:  %s", xTimerIsTimerActive(pattern_timer) ? "ACTIVE" : "INACTIVE");
            ESP_LOGI(TAG, "  Sensor:   %s", xTimerIsTimerActive(sensor_timer) ? "ACTIVE" : "INACTIVE");
            ESP_LOGI(TAG, "════════════════════════════\n");

            // flash STATUS_LED 200ms
            gpio_set_level(STATUS_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(STATUS_LED, 0);
            break;
        }

        case EV_CHANGE_PATTERN: {
            change_led_pattern((led_pattern_t)ev.param);
            break;
        }

        default:
            break;
        }
    }
}

// ===================== Helpers =====================
static void change_led_pattern(led_pattern_t new_pattern) {
    current_pattern = new_pattern;
    pattern_step = 0;
    pattern_state.step = 0;
    pattern_state.state = false;
    health_stats.pattern_changes++;
    // reset ทันที
    (void)xTimerReset(pattern_timer, 0);
}

static float read_sensor_value(void) {
    // Power on sensor and settle
    gpio_set_level(SENSOR_POWER, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // ADC read
    uint32_t raw = adc1_get_raw(ADC1_CHANNEL_0);
    uint32_t mv = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    // แปลงเป็น 0-50°C + noise
    float v = (mv / 1000.0f) * 50.0f;
    v += (float)((int)(esp_random() % 100) - 50) / 100.0f;

    gpio_set_level(SENSOR_POWER, 0);
    return v;
}

static void recovery_callback(TimerHandle_t t) {
    ESP_LOGI(TAG, "🔄 System recovered - resuming watchdog feeds");
    (void)xTimerStart(feed_timer, 0);
    (void)xTimerDelete(t, 0);
}

// ===================== INIT =====================
static void init_hardware(void) {
    // LEDs
    gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(WATCHDOG_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(PATTERN_LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PATTERN_LED_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(PATTERN_LED_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(SENSOR_POWER, GPIO_MODE_OUTPUT);

    gpio_set_level(STATUS_LED, 0);
    gpio_set_level(WATCHDOG_LED, 0);
    gpio_set_level(PATTERN_LED_1, 0);
    gpio_set_level(PATTERN_LED_2, 0);
    gpio_set_level(PATTERN_LED_3, 0);
    gpio_set_level(SENSOR_POWER, 0);

    // ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);

    ESP_LOGI(TAG, "Hardware init done");
}

static bool create_timers(void) {
    watchdog_timer = xTimerCreate("WatchdogTimer",
                                  pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS),
                                  pdFALSE, (void*)1, watchdog_timeout_callback);
    feed_timer     = xTimerCreate("FeedTimer",
                                  pdMS_TO_TICKS(WATCHDOG_FEED_MS),
                                  pdTRUE,  (void*)2, feed_watchdog_callback);
    pattern_timer  = xTimerCreate("PatternTimer",
                                  pdMS_TO_TICKS(PATTERN_BASE_MS),
                                  pdTRUE,  (void*)3, pattern_timer_callback);
    sensor_timer   = xTimerCreate("SensorTimer",
                                  pdMS_TO_TICKS(SENSOR_SAMPLE_MS),
                                  pdTRUE,  (void*)4, sensor_timer_callback);
    status_timer   = xTimerCreate("StatusTimer",
                                  pdMS_TO_TICKS(STATUS_UPDATE_MS),
                                  pdTRUE,  (void*)5, status_timer_callback);

    if (!watchdog_timer || !feed_timer || !pattern_timer || !sensor_timer || !status_timer) {
        ESP_LOGE(TAG, "Failed to create one or more timers");
        return false;
    }
    return true;
}

static bool create_queues_and_tasks(void) {
    event_queue  = xQueueCreate(32, sizeof(app_event_t));
    sensor_out_q = xQueueCreate(16, sizeof(sensor_data_t));
    if (!event_queue || !sensor_out_q) {
        ESP_LOGE(TAG, "Failed to create queues");
        return false;
    }

    // worker
    if (xTaskCreate(worker_task, "Worker", 4096, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create worker task");
        return false;
    }
    return true;
}

static void start_system(void) {
    ESP_LOGI(TAG, "Starting timers...");
    (void)xTimerStart(watchdog_timer, 0);
    (void)xTimerStart(feed_timer, 0);
    (void)xTimerStart(pattern_timer, 0);
    (void)xTimerStart(sensor_timer, 0);
    (void)xTimerStart(status_timer, 0);

    // start with slow blink
    change_led_pattern(PATTERN_SLOW_BLINK);

    ESP_LOGI(TAG, "🚀 System started");
    ESP_LOGI(TAG, "GPIO2=Status, GPIO4=Watchdog, GPIO5/18/19=Pattern LEDs");
}

// ===================== app_main =====================
void app_main(void) {
    ESP_LOGI(TAG, "Timer Applications Lab Starting...");

    init_hardware();

    if (!create_queues_and_tasks()) return;
    if (!create_timers()) return;

    start_system();

    // startup LED sequence (ใน context ของ app_main task)
    for (int i = 0; i < 2; i++) {
        gpio_set_level(PATTERN_LED_1, 1); vTaskDelay(pdMS_TO_TICKS(120));
        gpio_set_level(PATTERN_LED_2, 1); vTaskDelay(pdMS_TO_TICKS(120));
        gpio_set_level(PATTERN_LED_3, 1); vTaskDelay(pdMS_TO_TICKS(120));
        gpio_set_level(STATUS_LED,    1); gpio_set_level(WATCHDOG_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_set_level(PATTERN_LED_1, 0);
        gpio_set_level(PATTERN_LED_2, 0);
        gpio_set_level(PATTERN_LED_3, 0);
        gpio_set_level(STATUS_LED,    0);
        gpio_set_level(WATCHDOG_LED,  0);
        vTaskDelay(pdMS_TO_TICKS(180));
    }

    ESP_LOGI(TAG, "System operational - monitoring started");
}
