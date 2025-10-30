// main/Exercire_4.c
#include <stdio.h>
#include <inttypes.h>              // สำหรับ PRIu64 (พิมพ์ uint64_t ได้ถูกต้อง)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"

static const char *TAG = "PERF_EX4";
static SemaphoreHandle_t mutex;

// =====================================================
// Monitor Task – Show runtime & memory info
// =====================================================
static void monitor_task(void *pv)
{
    // เผื่อบัฟเฟอร์สำหรับรายชื่อทาสก์ (ขึ้นกับจำนวนทาสก์ในระบบ)
    static char buffer[2048];

    while (1)
    {
        size_t free_heap     = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        size_t min_free_heap = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);

        // ใช้ %zu ให้ตรงกับชนิด size_t
        ESP_LOGI(TAG, "Free Heap: %zu bytes | Min Heap: %zu bytes", free_heap, min_free_heap);

        // vTaskList() จะมีให้ใช้ต่อเมื่อเปิดสองออปชันใน menuconfig:
        // - Enable trace facility
        // - Enable FreeRTOS stats formatting functions
        // โค้ดนี้ครอบเงื่อนไขไว้เพื่อกันลิงก์เออร์เรอร์แม้ยังไม่เปิดออปชันดังกล่าว
#if (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
        vTaskList(buffer);
        printf("\nTask\t\tState\tPrio\tStack\tNum\n");
        printf("************************************\n");
        printf("%s\n", buffer);
#else
        printf("\n[vTaskList disabled] เปิดใน menuconfig:\n"
               "  FreeRTOS → Kernel → Enable trace facility\n"
               "  FreeRTOS → Kernel → Enable FreeRTOS stats formatting functions\n");
#endif

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// =====================================================
// Compute Task – Measure task execution time
// =====================================================
static void compute_task(void *pv)
{
    while (1)
    {
        uint64_t start = esp_timer_get_time();
        volatile int dummy = 0;
        for (int i = 0; i < 50000; i++) {
            dummy += i;
        }
        uint64_t end = esp_timer_get_time();

        // ใช้ PRIu64 เพื่อความถูกต้องของชนิดข้อมูล
        ESP_LOGI(TAG, "Task execution time: %" PRIu64 " µs", (end - start));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// =====================================================
// Watchdog Task – Feed system watchdog
// =====================================================
static void watchdog_task(void *pv)
{
    // ต้องเปิด Task Watchdog ใน menuconfig → ESP System Settings → Task Watchdog
    // หากยังไม่เปิด esp_task_wdt_add(NULL) อาจคืน ESP_ERR_INVALID_STATE
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    while (1)
    {
        ESP_LOGI(TAG, "Watchdog feed OK");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// =====================================================
// app_main() – System setup
// =====================================================
void app_main(void)
{
    ESP_LOGI(TAG, "Starting Exercise 4: Performance Optimization and Monitoring");

    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
    }

    ESP_LOGI(TAG, "Using system watchdog instance");

    // แยกคอร์เพื่อเดโม (Compute ที่ core0, Monitor ที่ core1)
    xTaskCreatePinnedToCore(compute_task,  "ComputeTask", 3072, NULL, 8, NULL, 0);
    xTaskCreatePinnedToCore(monitor_task,  "MonitorTask", 4096, NULL, 6, NULL, 1);
    xTaskCreate            (watchdog_task, "WatchdogTask", 2048, NULL, 5, NULL);
}
