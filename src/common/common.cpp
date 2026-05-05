#include "common/common.h"
#include "common.h"
#include "esp_mac.h"
#include "esp_task_wdt.h"
#include "flashmanager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "taskmanager.h"
#include <inttypes.h>


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
OSBase::SemHandle gSemInputBtnConfigFromRelayTaskToWifiTask;
// power manager task
OSBase::QueueHandle gQueueADCValueToPowerManageTask;
OSBase::QueueHandle gQueuePowerDataToWifiTask;

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
  gConfigSystem = ConfigSystemMessage(defaultConfig);
  LOG_INFO("MyMain", "Default Config");
  const auto &activeConfig = gConfigSystem.getConfigSystem();
  LOG_INFO("MyMain", "UserSystemID: %s  ", activeConfig.userSystemID);
  LOG_INFO("MyMain", "Ssid: %s  ", activeConfig.ssidWifiSta);
  LOG_INFO("MyMain", "Pass: %s  ", activeConfig.passWifiSta);

  // init device id
  gDeviceID = device_id48_u64();
  LOG_INFO("MyMain", "Device ID: %" PRIu64, gDeviceID);
}
