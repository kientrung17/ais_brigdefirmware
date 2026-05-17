#include "task/relaymanagertask.h"
#include "loggermanager.h"
#include "common/common.h"
#include "common/storeflashmanager.h"

RelayManagerTask::RelayManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioRelay[])
    : TaskAbstract(nameTask, numElementQueueSet)
{
    for (int i = 0; i < MessageCommon::MAX_NUM_RELAY; i++)
    {
        if (gpioRelay[i] != nullptr)
        {
            mRelay[i] = gpioRelay[i];
        }
        else
        {
            LOG_ERROR("RelayManagerTask", "Why init gpio null");
        }
    }

    // Call onInitProcess once at startup
    onInitProcess();
}

RelayManagerTask::~RelayManagerTask()
{
}

void RelayManagerTask::onTimer100HzProcess()
{
    // TODO: read button or touch sensor in future.
    mCounter100Hz++;
        // Đồng bộ trạng thái relay mỗi 1s để update GPIO từ flash
    // if (mCounter100Hz % 100 == 0) 
    // {
    //     onInitProcess(); 
    // }
}

void RelayManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    if (queue_sem == gQueueRelayControlCmd)
    {
        ControlRelayMessage msg;
        if (mOSBase->queueReceive(gQueueRelayControlCmd, &msg, 0) == OSBase::QUEUE_OK)
        {
            LOG_INFO("RelayManagerTask", "Received command from MQTT, processing...");
            processControlRelayMessage(msg.getControlRelayMessage());
        }
    }
}

void RelayManagerTask::onInitProcess()
{
    LOG_INFO("RelayManagerTask", "Init task");
    // load status from flash
    for (int i = 0; i < MessageCommon::MAX_NUM_RELAY; i++)
    {
        // read relay status
        ControlRelayMessage controlRelay;
        if (StoreFlashManager::getInstance()->readRelayInforFromFlash(&controlRelay, i))
        {
            // control relay
            int channel = controlRelay.getControlRelayMessage().channel;
            if (channel < MessageCommon::MAX_NUM_RELAY)
            {
                if (mRelay[channel])
                {
                    HalGpioAbstract::GpioPinState state = static_cast<HalGpioAbstract::GpioPinState>(controlRelay.getControlRelayMessage().status_control);
                    mRelay[channel]->Hal_Gpio_WritePin(state);
                }
            }
            else
            {
                LOG_ERROR("RelayManagerTask", "onInitProcess: Channel > max => not control");
            }
        }
        else
        {
            LOG_ERROR("RelayManagerTask", "onInitProcess: Read relay:%d infor error", i);
        }
    }


    // Spawn emergency E-Stop listener task with highest priority (4096 bytes stack)
    BaseType_t ret = xTaskCreate(emergencyTask, "EStopTask", 4096, this, configMAX_PRIORITIES - 1, nullptr);
    if (ret != pdPASS) {
        LOG_ERROR("RelayManagerTask", "Failed to create EStopTask (ret=%d)", (int)ret);
    } else {
        LOG_INFO("RelayManagerTask", "EStopTask spawned OK");
    }
}

void RelayManagerTask::processControlRelayMessage(const AquaCtrl_ControlRelayData &control)
{
    LOG_INFO("RelayManagerTask", "Process control relay message: channel=%u, status=%u", control.channel, control.status_control);
    int channel = control.channel;
    if (channel < MessageCommon::MAX_NUM_RELAY)
    {
        // B1. contrpl relay
        HalGpioAbstract::GpioPinState state = static_cast<HalGpioAbstract::GpioPinState>(control.status_control);
        mRelay[channel]->Hal_Gpio_WritePin(state);
        // B2. save to nvs
        ControlRelayMessage msgControl = ControlRelayMessage(control);
        if (StoreFlashManager::getInstance()->saveRelayInforToFlash(msgControl, channel))
        {
            LOG_INFO("RelayManagerTask", "Save to flash control relay message: channel=%u, status=%u", control.channel, control.status_control);
        }
    }
    else
    {
        LOG_ERROR("RelayManagerTask", "processControlRelayMessage: Channel > max => not control");
    }
}



void RelayManagerTask::emergencyTask(void *arg)
{
    RelayManagerTask *self = static_cast<RelayManagerTask *>(arg);

    LOG_INFO("RelayManagerTask", "EStopTask listening for E-Stop events...");

    while (true)
    {
        // Wait forever (portMAX_DELAY) for any E-Stop bit. 
        // This unblocks immediately when an event is posted, giving 0ms latency.
        EventBits_t bits = xEventGroupWaitBits(
            gEmergencyEventGroup,
            BIT_ESTOP_OVERLOAD | BIT_ESTOP_LOST_PHASE,
            pdTRUE,  // Clear bits on exit
            pdFALSE, // Wait for ANY bit (not ALL bits)
            portMAX_DELAY
        );

        if (bits != 0)
        {
            LOG_ERROR("RelayManagerTask", "!!! EMERGENCY STOP TRIGGERED !!! (Bits: 0x%08X)", (unsigned int)bits);
            
            // Hard cut all relays
            for (int i = 0; i < MessageCommon::MAX_NUM_RELAY; ++i)
            {
                if (self->mRelay[i])
                {
                    self->mRelay[i]->Hal_Gpio_WritePin(HalGpioAbstract::GpioPinState::GPIO_PIN_RESET);
                }
            }
        }
    }
}
