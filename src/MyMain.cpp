/*
 * myMain.cpp
 * Bridge Firmware
 */

#include "myMain.h"
#include "logger/loggermanager.h"
#include "taskmanager.h"
#include "task/espnowsendertask.h"
#include "common/common.h"
#include "common/storeflashmanager.h"

// esp-now sender task
#define ID_ESPNOW_SENDER_TASK 7
const std::string ESPNOW_SENDER_TASKNAME = "EspNowSenderTask";
const uint8_t MaxElementQueueSetTaskEspNowSender = 2;
EspNowSenderTask *mEspNowSenderTask{nullptr};

void StartEspNowSenderTask(void const *argument) {
  if (mEspNowSenderTask) {
    LOG_INFO("MyMain", "Start Task ESP-NOW Sender");
    mEspNowSenderTask->onStartProcess();
  } else {
    LOG_ERROR("MyMain", "Start Task ESP-NOW Sender Error");
  }
}

void startAllTask() {
  LOG_INFO("MyMain",
           "************Start init system and run task (BRIDGE)****************");
  initSystem();

  // init task object
  mEspNowSenderTask =
      new EspNowSenderTask(ESPNOW_SENDER_TASKNAME, MaxElementQueueSetTaskEspNowSender);

  // init task
  if (mOSBase->taskCreate((char *)ESPNOW_SENDER_TASKNAME.c_str(),
                          (TaskProc)StartEspNowSenderTask, OSBase::PRIORITY_NORMAL,
                          4096, ID_ESPNOW_SENDER_TASK)) {
    LOG_INFO("MyMain", "Start All Task Success");
  } else {
    LOG_ERROR("MyMain", "Start All Task Error");
  }
}
