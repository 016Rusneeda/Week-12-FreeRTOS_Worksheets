#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_random.h"

static const char *TAG = "MUTEX_LAB";

// =========================
// CHANGES for EXP2 (Race Condition Demo)
// Set 0 = disable mutex (equivalent to commenting out take/give)
// Set 1 = enable mutex normally
// =========================
#define USE_MUTEX 0

// LED pins for different tasks
#define LED_TASK1 GPIO_NUM_2
#define LED_TASK2 GPIO_NUM_4
#define LED_TASK3 GPIO_NUM_5
#define LED_CRITICAL GPIO_NUM_18

// Mutex handle
SemaphoreHandle_t xMutex;

// Shared resources (these need protection!)
typedef struct {
    uint32_t counter;
    char shared_buffer[100];
    uint32_t checksum;
    uint32_t access_count;
} shared_resource_t;

shared_resource_t shared_data = {0, "", 0, 0};

// Statistics for race condition detection
typedef struct {
    uint32_t successful_access;
    uint32_t failed_access;
    uint32_t corruption_detected;
    uint32_t priority_inversions;
} access_stats_t;

access_stats_t stats = {0, 0, 0, 0};

// ===== Helper macros to simulate "commenting out" mutex =====
static inline bool LOCK(TickType_t to_ticks) {
#if USE_MUTEX
    return (xSemaphoreTake(xMutex, to_ticks) == pdTRUE);
#else
    // When disabled, always "succeed" without taking a mutex
    (void)to_ticks;
    return true;
#endif
}
static inline void UNLOCK(void) {
#if USE_MUTEX
    xSemaphoreGive(xMutex);
#else
    // No-op when disabled
#endif
}

// Function to calculate simple checksum
uint32_t calculate_checksum(const char* data, uint32_t counter) {
    uint32_t sum = counter;
    for (int i = 0; data[i] != '\0'; i++) {
        sum += (uint32_t)data[i] * (i + 1);
    }
    return sum;
}

// Critical section function (simulates accessing shared resource)
void access_shared_resource(int task_id, const char* task_name, gpio_num_t led_pin) {
    char temp_buffer[100];
    uint32_t temp_counter;
    uint32_t expected_checksum;

    ESP_LOGI(TAG, "[%s] Requesting access to shared resource...", task_name);

    // =========================
    // CHANGES for EXP2:
    // Instead of directly calling xSemaphoreTake/xSemaphoreGive,
    // use LOCK()/UNLOCK() which are no-ops when USE_MUTEX=0
    // =========================
    if (LOCK(pdMS_TO_TICKS(5000))) {
        ESP_LOGI(TAG, "[%s] %s - entering critical section",
                 task_name, USE_MUTEX ? "✓ Mutex acquired" : "✓ (NO MUTEX) direct access");
        stats.successful_access++;

        // Turn on LED to show critical section access
        gpio_set_level(led_pin, 1);
        gpio_set_level(LED_CRITICAL, 1);

        // === CRITICAL SECTION BEGINS ===

        // Read current state
        temp_counter = shared_data.counter;
        strcpy(temp_buffer, shared_data.shared_buffer);
        expected_checksum = shared_data.checksum;

        // Verify data integrity before modification
        uint32_t calculated_checksum = calculate_checksum(temp_buffer, temp_counter);
        if (calculated_checksum != expected_checksum && shared_data.access_count > 0) {
            ESP_LOGE(TAG, "[%s] ⚠️  DATA CORRUPTION DETECTED!", task_name);
            ESP_LOGE(TAG, "Expected checksum: %lu, Calculated: %lu",
                     (unsigned long)expected_checksum, (unsigned long)calculated_checksum);
            stats.corruption_detected++;
        }

        ESP_LOGI(TAG, "[%s] Current state - Counter: %lu, Buffer: '%s'",
                 task_name, (unsigned long)temp_counter, temp_buffer);

        // Simulate some processing time (this makes race conditions more likely)
        vTaskDelay(pdMS_TO_TICKS(500 + (esp_random() % 1000)));

        // Modify shared data
        shared_data.counter = temp_counter + 1;
        snprintf(shared_data.shared_buffer, sizeof(shared_data.shared_buffer),
                 "Modified by %s #%lu", task_name, (unsigned long)shared_data.counter);
        shared_data.checksum = calculate_checksum(shared_data.shared_buffer, shared_data.counter);
        shared_data.access_count++;

        ESP_LOGI(TAG, "[%s] ✓ Modified - Counter: %lu, Buffer: '%s'",
                 task_name, (unsigned long)shared_data.counter, shared_data.shared_buffer);

        // More processing time
        vTaskDelay(pdMS_TO_TICKS(200 + (esp_random() % 500)));

        // === CRITICAL SECTION ENDS ===

        // Turn off LEDs
        gpio_set_level(led_pin, 0);
        gpio_set_level(LED_CRITICAL, 0);

        UNLOCK();
        ESP_LOGI(TAG, "[%s] %s",
                 task_name, USE_MUTEX ? "Mutex released" : "(NO MUTEX) exit critical section");

    } else {
        // This branch is only possible when USE_MUTEX=1 and timeout occurs
        ESP_LOGW(TAG, "[%s] ✗ Failed to acquire mutex (timeout)", task_name);
        stats.failed_access++;

        // Flash LED to indicate failed access
        for (int i = 0; i < 3; i++) {
            gpio_set_level(led_pin, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(led_pin, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// High priority task
void high_priority_task(void *pvParameters) {
    ESP_LOGI(TAG, "High Priority Task started (Priority: %d)", uxTaskPriorityGet(NULL));
    while (1) {
        access_shared_resource(1, "HIGH_PRI", LED_TASK1);
        vTaskDelay(pdMS_TO_TICKS(5000 + (esp_random() % 3000))); // 5-8 seconds
    }
}

// Medium priority task
void medium_priority_task(void *pvParameters) {
    ESP_LOGI(TAG, "Medium Priority Task started (Priority: %d)", uxTaskPriorityGet(NULL));
    while (1) {
        access_shared_resource(2, "MED_PRI", LED_TASK2);
        vTaskDelay(pdMS_TO_TICKS(3000 + (esp_random() % 2000))); // 3-5 seconds
    }
}

// Low priority task
void low_priority_task(void *pvParameters) {
    ESP_LOGI(TAG, "Low Priority Task started (Priority: %d)", uxTaskPriorityGet(NULL));
    while (1) {
        access_shared_resource(3, "LOW_PRI", LED_TASK3);
        vTaskDelay(pdMS_TO_TICKS(2000 + (esp_random() % 1000))); // 2-3 seconds
    }
}

// Priority inversion simulation task
void priority_inversion_task(void *pvParameters) {
    ESP_LOGI(TAG, "Priority Inversion Monitor started");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Check every 10 seconds
        ESP_LOGI(TAG, "🔄 Simulating CPU-intensive background work...");
        uint32_t start_time = xTaskGetTickCount();
        for (volatile int i = 0; i < 1000000; i++) {
            // Busy wait to consume CPU
        }
        uint32_t end_time = xTaskGetTickCount();
        ESP_LOGI(TAG, "Background work completed in %lu ms",
                 (unsigned long)((end_time - start_time) * portTICK_PERIOD_MS));
    }
}

// System monitor and statistics task
void monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "System monitor started");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(15000)); // Every 15 seconds

        ESP_LOGI(TAG, "\n═══ MUTEX SYSTEM MONITOR ═══");
#if USE_MUTEX
        ESP_LOGI(TAG, "Mutex Available: %s",
                 (xMutex && uxSemaphoreGetCount(xMutex)) ? "YES" : "NO (Held by task)");
#else
        ESP_LOGI(TAG, "Mutex State: DISABLED (Race Condition mode)");
#endif
        ESP_LOGI(TAG, "Shared Resource State:");
        ESP_LOGI(TAG, "  Counter: %lu", (unsigned long)shared_data.counter);
        ESP_LOGI(TAG, "  Buffer: '%s'", shared_data.shared_buffer);
        ESP_LOGI(TAG, "  Access Count: %lu", (unsigned long)shared_data.access_count);
        ESP_LOGI(TAG, "  Checksum: %lu", (unsigned long)shared_data.checksum);

        // Verify current data integrity
        uint32_t current_checksum = calculate_checksum(shared_data.shared_buffer,
                                                       shared_data.counter);
        if (current_checksum != shared_data.checksum && shared_data.access_count > 0) {
            ESP_LOGE(TAG, "⚠️  CURRENT DATA CORRUPTION DETECTED!");
            stats.corruption_detected++;
        }

        ESP_LOGI(TAG, "Access Statistics:");
        ESP_LOGI(TAG, "  Successful: %lu", (unsigned long)stats.successful_access);
        ESP_LOGI(TAG, "  Failed:     %lu", (unsigned long)stats.failed_access);
        ESP_LOGI(TAG, "  Corrupted:  %lu", (unsigned long)stats.corruption_detected);
        ESP_LOGI(TAG, "  Success Rate: %.1f%%",
                 stats.successful_access + stats.failed_access > 0 ?
                 (float)stats.successful_access /
                 (stats.successful_access + stats.failed_access) * 100 : 0);
        ESP_LOGI(TAG, "══════════════════════════\n");
    }
}

// (Optional) Extra race condition demo — keep OFF to avoid overlapping effects
#define ENABLE_RACE_DEMO 0
void race_condition_demo_task(void *pvParameters) {
#if ENABLE_RACE_DEMO
    static bool demo_enabled = false;
    ESP_LOGI(TAG, "Race Condition Demo task started (initially disabled)");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000)); // Every 30 seconds
        if (!demo_enabled) {
            ESP_LOGW(TAG, "\n🎭 DEMO: Forcing unsafe access without mutex");
            demo_enabled = true;
            uint32_t temp = shared_data.counter;
            vTaskDelay(pdMS_TO_TICKS(100));
            shared_data.counter = temp + 100;
            strcpy(shared_data.shared_buffer, "UNSAFE ACCESS - DEMO ONLY");
            ESP_LOGW(TAG, "Demo complete - race condition may have occurred\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            demo_enabled = false;
        }
    }
#else
    (void)pvParameters;
    vTaskDelete(NULL);
#endif
}

void app_main(void) {
    ESP_LOGI(TAG, "Mutex and Critical Sections Lab Starting...");

    // Configure LED pins
    gpio_set_direction(LED_TASK1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_TASK2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_TASK3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CRITICAL, GPIO_MODE_OUTPUT);

    // Turn off all LEDs
    gpio_set_level(LED_TASK1, 0);
    gpio_set_level(LED_TASK2, 0);
    gpio_set_level(LED_TASK3, 0);
    gpio_set_level(LED_CRITICAL, 0);

#if USE_MUTEX
    // Create mutex
    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex!");
    } else {
        ESP_LOGI(TAG, "Mutex created successfully");
    }
#else
    // In EXP2 mode we deliberately do NOT create or use a mutex
    xMutex = NULL;
    ESP_LOGW(TAG, "EXP2 MODE: MUTEX DISABLED — expecting race conditions");
#endif

    // Initialize shared resource
    shared_data.counter = 0;
    strcpy(shared_data.shared_buffer, "Initial state");
    shared_data.checksum = calculate_checksum(shared_data.shared_buffer, shared_data.counter);
    shared_data.access_count = 0;

    // Create tasks with different priorities
    xTaskCreate(high_priority_task,      "HighPri",  3072, NULL, 2, NULL);
    xTaskCreate(medium_priority_task,    "MedPri",   3072, NULL, 3, NULL);
    xTaskCreate(low_priority_task,       "LowPri",   3072, NULL, 5, NULL);
    xTaskCreate(priority_inversion_task, "PrioInv",  2048, NULL, 4, NULL);
    xTaskCreate(monitor_task,            "Monitor",  3072, NULL, 1, NULL);
    xTaskCreate(race_condition_demo_task,"RaceDemo", 2048, NULL, 1, NULL);

    ESP_LOGI(TAG, "All tasks created with priorities:");
    ESP_LOGI(TAG, "  High Priority Task: 5");
    ESP_LOGI(TAG, "  Priority Inversion: 4");
    ESP_LOGI(TAG, "  Medium Priority:    3");
    ESP_LOGI(TAG, "  Low Priority:       2");
    ESP_LOGI(TAG, "  Monitor & Demo:     1");
    ESP_LOGI(TAG, "\nSystem operational - watch for %s!",
             USE_MUTEX ? "mutex contention" : "DATA CORRUPTION (Race Condition)");

    // LED startup sequence
    for (int i = 0; i < 2; i++) {
        gpio_set_level(LED_TASK1, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_TASK1, 0);
        gpio_set_level(LED_TASK2, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_TASK2, 0);
        gpio_set_level(LED_TASK3, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_TASK3, 0);
        gpio_set_level(LED_CRITICAL, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_CRITICAL, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
