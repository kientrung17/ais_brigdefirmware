#include "common/common.h"
#include "common.h"
#include "esp_mac.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "flashmanager.h"
#include "common/storeflashmanager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "taskmanager.h"
#include <inttypes.h>

// Danh sách tên các task quan trọng cần được bảo vệ khỏi taskfreezer
static const char* BRIDGE_PROTECTED_TASK_NAMES[] = {
    "EspNowSenderTas",  // FreeRTOS truncates to 16 chars
    "LoraBridgeTask",
    nullptr // sentinel
};

static void resumeSuspendedProtectedTasks() {
    UBaseType_t numTasks = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskArray = (TaskStatus_t*)pvPortMalloc(numTasks * sizeof(TaskStatus_t));
    if (!taskArray) return;
    uint32_t totalRunTime = 0;
    UBaseType_t filled = uxTaskGetSystemState(taskArray, numTasks, &totalRunTime);
    for (UBaseType_t i = 0; i < filled; i++) {
        if (taskArray[i].eCurrentState == eSuspended) {
            for (int j = 0; BRIDGE_PROTECTED_TASK_NAMES[j] != nullptr; j++) {
                if (strncmp(taskArray[i].pcTaskName, BRIDGE_PROTECTED_TASK_NAMES[j], configMAX_TASK_NAME_LEN - 1) == 0) {
                    ESP_LOGW("WDT_RESUMER", "Task '%s' Suspended by taskfreezer! Resuming...", taskArray[i].pcTaskName);
                    vTaskResume(taskArray[i].xHandle);
                    break;
                }
            }
        }
    }
    vPortFree(taskArray);
}

static void watchdogTask(void *arg) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // kiểm tra mỗi 1 giây
        resumeSuspendedProtectedTasks();
    }
}

extern "C" void startWatchdogTask() {
    xTaskCreate(watchdogTask, "WdtResumer", 2048, nullptr, 22, nullptr);
}


const System_ConfigSystemData defaultConfig = {.userSystemID = "sys000001",
                                               .ssidWifiSta = "test",
                                               .passWifiSta = "12345678",
                                               .uriMqtt = "",
                                               .portMqtt = 0,
                                               .mqttUser = "",
                                               .mqttPass = ""};

// Init param
OSBase *mOSBase;
// wifi task
// Shared Data Store
SharedDataStore gSharedData;

// E-Stop EventGroup
EventGroupHandle_t gEmergencyEventGroup;
EventGroupHandle_t gEventGroupNetworkState;

// IPC Queues
OSBase::QueueHandle gQueuePowerDataToMqtt;
OSBase::QueueHandle gQueueRelayControlCmd;

// relay gpio for webserver control
HalGpioAbstract *gGpioRelay[MessageCommon::MAX_NUM_RELAY]{};
// power manager access for webserver monitor
PowerManagerTask *gPowerManagerTask{nullptr};

// device id
uint64_t gDeviceID;
// object config system
ConfigSystemMessage gConfigSystem;
FlashManagerAbstract *gFlashManager = new FlashManager();
bool gIsSntpSynced = false;
// Init function

void processTimer100Hz() {
  static int counter = 0;
  // send semmaphore event 100Hz (every 10 calls, since timer is 1kHz)
  if (++counter >= 10) {
    TaskManager::getInstance()->onTimer100Hz();
    counter = 0;
  }
}

// get ID
uint64_t device_id48_u64(void) {
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac); // Factory MAC duy nhất
  uint64_t id = 0;
  for (int i = 0; i < 6; ++i) {
    id = (id << 8) | mac[i];
  }
  return id; // 48-bit nằm ở phần thấp của u64
}

void initSystem() {
  // get OSBase
  mOSBase = TaskManager::getInstance()->getOSBase();
  
  // init E-Stop EventGroup using FreeRTOS API
  gEmergencyEventGroup = xEventGroupCreate();
  gEventGroupNetworkState = xEventGroupCreate();

  // init IPC Queues
  gQueuePowerDataToMqtt = mOSBase->queueCreate(10, sizeof(ControlStatusDataMessage));
  gQueueRelayControlCmd = mOSBase->queueCreate(10, sizeof(ControlRelayMessage));

  gConfigSystem = ConfigSystemMessage(defaultConfig);
  // Thu tai cau hinh tu Flash NVS
  if (StoreFlashManager::getInstance()->readConfigFromFlash(&gConfigSystem)) {
    LOG_INFO("MyMain", "Loaded custom config from Flash NVS successfully!");
  } else {
    LOG_INFO("MyMain", "No custom config found, using default config");
  }
  
  const auto &activeConfig = gConfigSystem.getConfigSystem();
  LOG_INFO("MyMain", "UserSystemID: %s  ", activeConfig.userSystemID);
  LOG_INFO("MyMain", "Ssid: %s  ", activeConfig.ssidWifiSta);
  LOG_INFO("MyMain", "Pass: %s  ", activeConfig.passWifiSta);

  // init device id
  gDeviceID = device_id48_u64();
  LOG_INFO("MyMain", "Device ID: %" PRIu64, gDeviceID);
}
