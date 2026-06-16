/*
 * myMain.cpp
 * Bridge Firmware
 */

#include "myMain.h"
#include "logger/loggermanager.h"
#include "taskmanager.h"
#include "task/espnowsendertask.h"
#include "task/lora_bridge_task.h"
#include "common/common.h"
#include "common/storeflashmanager.h"

// esp-now sender task
#define ID_ESPNOW_SENDER_TASK 7
const std::string ESPNOW_SENDER_TASKNAME = "EspNowSenderTask";
const uint8_t MaxElementQueueSetTaskEspNowSender = 2;
EspNowSenderTask *mEspNowSenderTask{nullptr};

// lora bridge task
const std::string LORA_BRIDGE_TASKNAME = "LoraBridgeTask";
const uint8_t MaxElementQueueSetTaskLoraBridge = 2;
LoraBridgeTask *mLoraBridgeTask{nullptr};

void StartEspNowSenderTask(void const *argument) {
  if (mEspNowSenderTask) {
    LOG_INFO("MyMain", "Start Task ESP-NOW Sender");
    mEspNowSenderTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task ESP-NOW Sender Error");
  }
}

void StartLoraBridgeTask(void const *argument) {
  if (mLoraBridgeTask) {
    LOG_INFO("MyMain", "Start Task LoRa Bridge");
    mLoraBridgeTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task LoRa Bridge Error");
  }
}

void startAllTask() {
  LOG_INFO("MyMain",
           "************Start init system and run task (BRIDGE)****************");
  initSystem();

  // init task objects
  mEspNowSenderTask =
      new EspNowSenderTask(ESPNOW_SENDER_TASKNAME, MaxElementQueueSetTaskEspNowSender);
  
  mLoraBridgeTask =
      new LoraBridgeTask(LORA_BRIDGE_TASKNAME, MaxElementQueueSetTaskLoraBridge);

  // init tasks
  bool success = true;
  success &= mOSBase->taskCreate((char *)ESPNOW_SENDER_TASKNAME.c_str(),
                                 (TaskProc)StartEspNowSenderTask, OSBase::PRIORITY_NORMAL,
                                 4096, ID_ESPNOW_SENDER_TASK);

  success &= mOSBase->taskCreate((char *)LORA_BRIDGE_TASKNAME.c_str(),
                                 (TaskProc)StartLoraBridgeTask, OSBase::PRIORITY_NORMAL,
                                 4096, ID_LORA_BRIDGE_TASK);

  if (success) {
    LOG_INFO("MyMain", "Start All Task Success");
  } else {
    LOG_ERROR("MyMain", "Start All Task Error");
  }
}

