#include "architectureabs/taskmanager.h"
#include "OSAL/ESP32/OSFreeRTOS.h"
#include "esp_log.h"

static const char* TAG = "TaskManager_OVERRIDE";

TaskManager TaskManager::self;

TaskManager* TaskManager::getInstance() {
    return &self;
}

TaskManager::TaskManager() {
    mOSBase = new OSFreeRTOS();
    ESP_LOGI(TAG, "TaskManager initialized (NEW OVERRIDE VERSION)");
}

TaskManager::~TaskManager() {
    // OSBase is a singleton-like object with protected destructor.
    // It should not be deleted here.
}

bool TaskManager::registerProcessor(TaskAbstract* processor) {
    if (processor) {
        mMapProcessor[processor->getTaskName()] = processor;
        ESP_LOGI(TAG, "Registered processor: %s", processor->getTaskName().c_str());
        return true;
    }
    return false;
}

bool TaskManager::deregisterProcessor(std::string processorName) {
    auto it = mMapProcessor.find(processorName);
    if (it != mMapProcessor.end()) {
        mMapProcessor.erase(it);
        ESP_LOGI(TAG, "Deregistered processor: %s", processorName.c_str());
        return true;
    }
    return false;
}

OSBase* TaskManager::getOSBase() {
    return mOSBase;
}

void TaskManager::onTimer100Hz() {
    if (!mOSBase) return;
    
    for (auto& pair : mMapProcessor) {
        TaskAbstract* task = pair.second;
        if (task) {
            OSBase::SemHandle sem = task->getSemaphore100Hz();
            if (sem > 0) {
                // If queue is full, it just fails silently which is expected 
                // for a binary semaphore that's already given
                mOSBase->semGive(sem);
            }
        }
    }
}
