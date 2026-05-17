#ifndef COMMON_H_
#define COMMON_H_

#include "stdbool.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// === C-compatible section ===
#define MAX_LENGTH_TASK_NAME 16
#define MAX_TASK 10
#define FREQUENCE_TIMER_READ_ADC 1000 // 1khz
void processTimer100Hz();

#ifdef __cplusplus
}
#endif

// === C++-only section ===
#ifdef __cplusplus

#include "HAL_ESP32/esp32adc.h"
#include "OSFreeRTOS.h"
#include "configsystemmessage.h"
#include "flashmanager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "loggermanager.h"
#include "message/controlrelaymessage.h"
#include "message/controlstatusdatamessage.h"
#include "message/messagecommon.h"
#include "stdint.h"
#include <atomic>
#include <string>

#define UDP_LOCAL_PORT_SYSTEM_CONFIG 8080
// Gpio relay
#define PIN_GPIO_RELAY_1 32
#define PIN_GPIO_RELAY_2 33
#define PIN_GPIO_RELAY_3 25
#define PIN_GPIO_RELAY_4 26
#define PIN_GPIO_RELAY_5 27
#define PIN_GPIO_RELAY_6 14
// Gpio input check power
#define PIN_GPIO_CHECK_PHASE 34
#define PIN_GPIO_CHECK_ELECTRIC 18
// Adc power monitor
#define PIN_ADC_SCT013_01 36
#define PIN_ADC_SCT013_02 39
#define PIN_ADC_CHECK_PIN 35
// Pin
#define PIN_GPIO_CHARGE_PIN 15
//
#define PIN_TOUCH_SENSOR_1 4
#define PIN_TOUCH_SENSOR_2 0
#define PIN_TOUCH_SENSOR_3 2
#define PIN_TOUCH_SENSOR_4 15
#define PIN_TOUCH_SENSOR_5 13
#define PIN_TOUCH_SENSOR_6 12
#define PIN_TOUCH_SENSOR_CONFIG 27 // pin config mode

// Default configuration
extern const System_ConfigSystemData defaultConfig;
// Osbase
extern OSBase *mOSBase;

// relay manager task
// wifi task
extern OSBase::SemHandle gSemInputBtnConfigFromRelayTaskToWifiTask;
// Shared Data Store (Lock-free)
struct SharedDataStore {
  std::atomic<float> ampe_ch1{0.0f};
  std::atomic<float> ampe_ch2{0.0f};
  std::atomic<float> voltage_pin{0.0f};
  std::atomic<float> motor_temp{0.0f};
  std::atomic<float> remote_voltage{0.0f};
  std::atomic<bool> is_lost_phase{false};
  std::atomic<bool> is_lost_electric{false};
};
extern SharedDataStore gSharedData;

// E-Stop EventGroup for ultra-low latency emergency relay cutoff
extern EventGroupHandle_t gEmergencyEventGroup;
#define BIT_ESTOP_OVERLOAD (1 << 0)
#define BIT_ESTOP_LOST_PHASE (1 << 1)

extern EventGroupHandle_t gEventGroupNetworkState;
#define BIT_WIFI_CONNECTED (1 << 0)
#define BIT_LAN_CONNECTED (1 << 1)

// IPC Queues
extern OSBase::QueueHandle gQueuePowerDataToMqtt;
extern OSBase::QueueHandle gQueueRelayControlCmd;

// sys infor
class HalGpioAbstract;
class PowerManagerTask;
extern HalGpioAbstract *gGpioRelay[MessageCommon::MAX_NUM_RELAY];
extern PowerManagerTask *gPowerManagerTask;

extern uint64_t gDeviceID;
extern FlashManagerAbstract *gFlashManager;
extern ConfigSystemMessage gConfigSystem;
extern bool gIsSntpSynced;

void initSystem();
#endif // __cplusplus

#endif /* COMMON_H_ */
