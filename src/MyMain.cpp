/*
 * myMain.cpp
 *
 *  Created on: May 13, 2024
 *      Author: tungtd10
 */

#include "myMain.h"
#include "logger/loggermanager.h"
#include "message/controlstatusdatamessage.h"
#include "task/adcreadertask.h"
#include "task/powermanagertask.h"
#include "task/relaymanagertask.h"
#include "task/wifimanagertask.h"
#include "taskmanager.h"


#include "HAL/HAL_ESP32/esp32gpio.h"
#include "HAL/HAL_ESP32/wifimanager.h"
// #include "HAL/HAL_ESP32/esp32touch.h"

#include "common/common.h"
#include "common/storeflashmanager.h"

// wifi
#define ID_WIFI_MANAGER_TASK 1
const std::string WIFI_MANAGER_TASKNAME = "WifiManagerTask";
// - 1 (sem 100Hz) + 1 (sem config btn) = 2
// Dùng 4 để có biên an toàn.
const uint8_t MaxElementQueueSetTaskWifiManager = 4;
WifiManagerTask *mWifiManagerTask{nullptr};
WiFiManagerAbstract *mWifiManagerAbs{nullptr};

// relay manager task
//  config system
#define ID_RELAY_MANAGER_TASK 3
const std::string RELAY_MANAGER_TASKNAME = "RelayManagerTask";
const uint8_t MaxElementQueueSetTaskRelayManager = 10;
RelayManagerTask *mRelayManagerTask{nullptr};
// TouchSensorAbstract *mTouchSensor[RelayManagerTask::MAX_NUM_TOUCH_SENSOR];

// power manager task
#define ID_POWER_MANAGER_TASK 4
const std::string POWER_MANAGER_TASKNAME = "PowerManagerTask";
// FIX: QueueSet size PHẢI >= tổng kích thước TẤT CẢ queue/sem đăng ký vào:
// - 1 (sem 100Hz)
// Dùng 2 để có biên an toàn.
const uint8_t MaxElementQueueSetTaskPowerManager = 2;
HalGpioAbstract *mGpioCheckPhase{nullptr};
HalGpioAbstract *mGpioCheckElectric{nullptr};
HalGpioAbstract *mGpioChargePin{nullptr};

// adc reader task
#define ID_ADC_READER_TASK 5
const std::string ADC_READER_TASKNAME = "AdcReaderTask";
const uint8_t MaxElementQueueSetTaskAdcReader = 2;
AdcReaderTask *mAdcReaderTask{nullptr};

void StartWifiMamnagerTask(void const *argument) {
  if (mWifiManagerTask) {
    LOG_INFO("MyMain", "Start Task wifi");
    mWifiManagerTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task Wifi Error");
  }
}

void StartRelayManagerTask(void const *argument) {
  if (mRelayManagerTask) {
    LOG_INFO("MyMain", "Start Task relay");
    mRelayManagerTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task Relay Error");
  }
}

void StartPowerManagerTask(void const *argument) {
  if (gPowerManagerTask) {
    LOG_INFO("MyMain", "Start Task Power Manager");
    gPowerManagerTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task Power Manager Error");
  }
}

void StartAdcReaderTask(void const *argument) {
  if (mAdcReaderTask) {
    LOG_INFO("MyMain", "Start Task ADC Reader");
    mAdcReaderTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task ADC Reader Error");
  }
}

void startAllTask() {
  LOG_INFO("MyMain",
           "************Start init system and run task****************");
  initSystem();

  // init task object
  ////////////////////// wifi
  mWifiManagerAbs = new WiFiManagerESP32();
  mWifiManagerTask = new WifiManagerTask(mWifiManagerAbs, WIFI_MANAGER_TASKNAME,
                                         MaxElementQueueSetTaskWifiManager);
  // register sem
  mWifiManagerTask->initRegisterSemaphoreToQueueset(
      &gSemInputBtnConfigFromRelayTaskToWifiTask);

  // queue transmit data from power task to wifi task (REMOVED - Using SharedDataStore)

  ////////////// relay manager task
  auto relayMode = HalGpioAbstract::GpioMode::GPIO_MODE_INPUT_OUTPUT;
  gGpioRelay[0] = new HalEsp32Gpio(PIN_GPIO_RELAY_1, relayMode);
  gGpioRelay[1] = new HalEsp32Gpio(PIN_GPIO_RELAY_2, relayMode);
  gGpioRelay[2] = new HalEsp32Gpio(PIN_GPIO_RELAY_3, relayMode);
  gGpioRelay[3] = new HalEsp32Gpio(PIN_GPIO_RELAY_4, relayMode);
  gGpioRelay[4] = new HalEsp32Gpio(PIN_GPIO_RELAY_5, relayMode);
  gGpioRelay[5] = new HalEsp32Gpio(PIN_GPIO_RELAY_6, relayMode);
  // mTouchSensor[0] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_1);
  // mTouchSensor[1] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_2);
  // mTouchSensor[2] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_3);
  // mTouchSensor[3] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_4);
  // mTouchSensor[4] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_5);
  // mTouchSensor[5] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_6);
  // mTouchSensor[6] = new EspidfTouchSensor(PIN_TOUCH_SENSOR_CONFIG);
  mRelayManagerTask = new RelayManagerTask(RELAY_MANAGER_TASKNAME,
                                           MaxElementQueueSetTaskRelayManager,
                                           gGpioRelay, nullptr);

  ////////////// power manager task
  mGpioCheckPhase = new HalEsp32Gpio(
      PIN_GPIO_CHECK_PHASE, HalGpioAbstract::GpioMode::GPIO_MODE_INPUT);
  mGpioCheckElectric = new HalEsp32Gpio(
      PIN_GPIO_CHECK_ELECTRIC, HalGpioAbstract::GpioMode::GPIO_MODE_INPUT);
  mGpioChargePin = new HalEsp32Gpio(
      PIN_GPIO_CHARGE_PIN, HalGpioAbstract::GpioMode::GPIO_MODE_INPUT_OUTPUT);
  gPowerManagerTask = new PowerManagerTask(
      POWER_MANAGER_TASKNAME, MaxElementQueueSetTaskPowerManager,
      mGpioCheckPhase, mGpioCheckElectric, mGpioChargePin);
  // queue receive ADC raw (2 channels) - REMOVED, using SharedDataStore

  ////////////// adc reader task
  mAdcReaderTask =
      new AdcReaderTask(ADC_READER_TASKNAME, MaxElementQueueSetTaskAdcReader);

  // init task
  if (mOSBase->taskCreate((char *)WIFI_MANAGER_TASKNAME.c_str(),
                          (TaskProc)StartWifiMamnagerTask,
                          OSBase::PRIORITY_HIGH, 8012, ID_WIFI_MANAGER_TASK) &&
      mOSBase->taskCreate((char *)RELAY_MANAGER_TASKNAME.c_str(),
                          (TaskProc)StartRelayManagerTask,
                          OSBase::PRIORITY_NORMAL, 8012,
                          ID_RELAY_MANAGER_TASK) &&
      mOSBase->taskCreate((char *)POWER_MANAGER_TASKNAME.c_str(),
                          (TaskProc)StartPowerManagerTask,
                          OSBase::PRIORITY_NORMAL, 4096,
                          ID_POWER_MANAGER_TASK) &&
      mOSBase->taskCreate((char *)ADC_READER_TASKNAME.c_str(),
                          (TaskProc)StartAdcReaderTask, OSBase::PRIORITY_HIGH,
                          4096, ID_ADC_READER_TASK)) {
    LOG_INFO("MyMain", "Start All Task Success");
  } else {
    LOG_ERROR("MyMain", "Start All Task Error");
  }
}
