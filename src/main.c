#include "MyMain.h"
#include "common/common.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern void startWatchdogTask();

void timer_callback(void *arg) { processTimer100Hz(); }

void app_main() {
  esp_log_level_set("*", ESP_LOG_DEBUG); // Hiện toàn bộ log DEBUG trở xuống
  // freertos
  startAllTask();
  startWatchdogTask(); // Bảo vệ các task khỏi taskfreezer trong libcommon.a
  vTaskDelay(pdMS_TO_TICKS(1000));
  // timer 100hz
  const esp_timer_create_args_t timer_args = {.callback = &timer_callback,
                                              .name = "100HzTimer"};
  esp_timer_handle_t timer_handle;
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle));
  // 1kHz = 1ms = 1,000 microseconds
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 1000)); // 1,000 µs
  vTaskDelay(pdMS_TO_TICKS(100));

}