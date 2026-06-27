#include "architectureabs/taskabstract.h"
#include "architectureabs/taskmanager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "TaskAbstract_OVERRIDE";

TaskAbstract::TaskAbstract(const std::string &processorName, int numElementQueueset)
    : mTaskName(processorName)
{
    mMonitorSemaphore100Hz = 0;
    
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (osBase) {
        mQueueSet = osBase->queueSetCreate(numElementQueueset);
        initRegisterSemaphoreToQueueset(&mMonitorSemaphore100Hz);
        ESP_LOGI(TAG, "[%s] Create semaphore 100Hz: %d", mTaskName.c_str(), mMonitorSemaphore100Hz);
    } else {
        mQueueSet = 0;
    }

    TaskManager::getInstance()->registerProcessor(this);
}

TaskAbstract::~TaskAbstract()
{
    TaskManager::getInstance()->deregisterProcessor(mTaskName);
}

std::string TaskAbstract::getTaskName() const
{
    return mTaskName;
}

OSBase::SemHandle TaskAbstract::getSemaphore100Hz()
{
    return mMonitorSemaphore100Hz;
}

void TaskAbstract::onStartProcess()
{
    ESP_LOGI(TAG, "[%s] onStartProcess called (NEW OVERRIDE VERSION - NO 120s TIMEOUT!)", mTaskName.c_str());
    
    onInitProcess();
    
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (!osBase) {
        ESP_LOGE(TAG, "[%s] OSBase is NULL! Task suspended.", mTaskName.c_str());
        vTaskSuspend(NULL);
    }

    ESP_LOGI(TAG, "[%s] Entering infinite event loop...", mTaskName.c_str());
    while (1)
    {
        // OSBase::queueSetWaitAny dÆ°á»›i quáº£n lĂ½ cá»§a tĂ¡c giáº£ cÅ©ng cĂ³ thá»ƒ cĂ³ lá»—i tráº£ vá»  0 khi timeout
        OSBase::QueueSetMember handle = osBase->queueSetWaitAny(mQueueSet);
        
        if (handle == 0) {
            // NĂ U Ä Ă‚Y LĂ€ LĂ—I GĂ‚Y TREO (TIMEOUT) THĂŒ TA CHá»ˆ TIáº¾P Tá»¤C, KHĂ”NG BREAK!
            ESP_LOGW(TAG, "[%s] queueSetWaitAny returned 0! Ignoring and continuing loop...", mTaskName.c_str());
            continue;
        }
        
        if (handle == mMonitorSemaphore100Hz) {
            osBase->semTake(mMonitorSemaphore100Hz, 0); // Clear semaphore
            onTimer100HzProcess();
        } else {
            onQueueSetMessageProcess(handle);
        }
    }
}

void TaskAbstract::onStopProcess()
{
    ESP_LOGW(TAG, "[%s] onStopProcess called!", mTaskName.c_str());
}

bool TaskAbstract::initRegisterSemaphoreToQueueset(OSBase::SemHandle *sem)
{
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (!osBase) return false;
    
    *sem = osBase->semCreate();
    return registerSemaphoreToQueueset(*sem);
}

bool TaskAbstract::initregisterQueueToQueueset(OSBase::QueueHandle *queue, int sizeElement, int numElement)
{
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (!osBase) return false;
    
    *queue = osBase->queueCreate(numElement, sizeElement);
    return registerQueueToQueueset(*queue);
}

bool TaskAbstract::registerSemaphoreToQueueset(OSBase::SemHandle sem)
{
    if (sem == 0) return false;
    
    if (mMonitorSemaphore100Hz == 0) {
        mMonitorSemaphore100Hz = sem;
    }
    
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (!osBase) return false;
    
    return osBase->queueSetAddSemaphore(sem, mQueueSet) == OSBase::QUEUESET_OK;
}

bool TaskAbstract::registerQueueToQueueset(OSBase::QueueHandle queue)
{
    if (queue == 0) return false;
    
    OSBase* osBase = TaskManager::getInstance()->getOSBase();
    if (!osBase) return false;
    
    return osBase->queueSetAddQueue(queue, mQueueSet) == OSBase::QUEUESET_OK;
}
