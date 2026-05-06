#include "task/relaymanagertask.h"
#include "loggermanager.h"
#include "common/common.h"
#include "common/storeflashmanager.h"

RelayManagerTask::RelayManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioRelay[],
                                   TouchSensorAbstract *touchSensors[])
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
    for (int i = 0; i < MAX_NUM_TOUCH_SENSOR; i++)
    {
        if (touchSensors != nullptr && touchSensors[i] != nullptr)
        {
            mTouchSensor[i] = touchSensors[i];
        }
        else
        {
            mTouchSensor[i] = nullptr;
            LOG_INFO("RelayManagerTask", "Touch sensor %d is disabled", i);
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
    (void)queue_sem;
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
    // init touch sensor
    // for (int i = 0; i < MAX_NUM_TOUCH_SENSOR; i++)
    // {
    //     mTouchSensor[i]->init(TIME_FILTER_TOUCH_SENSOR_HARDWARE);
    //     mTouchSensor[i]->calibrate(NUM_SAMPLE_CALIB_BASELINE_TOUCH_SENSOR, 5);
    //     mTouchSensor[i]->setRelativeThresholdPercent(RELATIVE_THRESHOULD_PERCENT);                           // ON khi giá trị < baseline - RELATIVE_THRESHOULD_PERCENT%
    //     mTouchSensor[i]->setDebounce(NUM_SAMPLE_DETECT_ON_OFF_TOUCH_SENSOR, NUM_SAMPLE_DETECT_ON_OFF_TOUCH_SENSOR); // Yêu cầu 5 mẫu liên tiếp để thay đổi trạng thái
    // }

    // Spawn emergency E-Stop listener task with highest priority
    BaseType_t ret = xTaskCreate(emergencyTask, "EStopTask", 2048, this, configMAX_PRIORITIES - 1, nullptr);
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

void RelayManagerTask::updateStateMachineTouchSensor10Hz(uint8_t channel)
{
    if (mTouchSensor[channel] == nullptr)
    {
        return; // Touch sensor disabled
    }
    switch (mCurrentStepCheckTouchSensor)
    {
    case STEP_CHECK_TOUCH_SENSOR::IDLE:
    {
        if (mTouchSensor[channel]->isOn())
        {
            LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: Step ON detected", channel);
            mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::ON_DETECTED;
            updateStateMachineTouchSensor10Hz(channel);
        }
        break;
    }
    case STEP_CHECK_TOUCH_SENSOR::ON_DETECTED:
    {
        if (mTouchSensor[channel]->isOn())
        {
            mCountOnStable[channel]++;
            if (mCountOnStable[channel] >= NUM_DETECT_ON_OFF_STABLE_MIN)
            {
                LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: Step ON stable", channel);
                mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::ON_SATBLE;
                updateStateMachineTouchSensor10Hz(channel);
            }
        }
        else
        {
            LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: Off in step ON_DETECTED-> change to Step idle", channel);
            mCountOnStable[channel] = 0;
            mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::IDLE;
            updateStateMachineTouchSensor10Hz(channel);
        }
        break;
    }

    case STEP_CHECK_TOUCH_SENSOR::ON_SATBLE:
    {
        if (mTouchSensor[channel]->isOn())
        {
            mCountOnStable[channel]++;
        }
        else // off
        {
            if (mCountOnStable[channel] >= NUM_DETECT_ON_OFF_STABLE_MIN && mCountOnStable[channel] <= NUM_DETECT_ON_OFF_STABLE_MAX)
            {
                LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: Step OFF_DETECTED stable", channel);
                mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::OFF_DETECTED;
                updateStateMachineTouchSensor10Hz(channel);
            }
            else
            {
                LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: on over timer in step ON_SATBLE-> change to Step idle", channel);
                mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::IDLE;
                updateStateMachineTouchSensor10Hz(channel);
            } 
            mCountOnStable[channel] = 0;
        }
        break;
    }
    case STEP_CHECK_TOUCH_SENSOR::OFF_DETECTED:
    {
        if (mTouchSensor[channel]->isOn() == false)
        {
            mCountOffStable[channel]++;
            if (mCountOffStable[channel] >= NUM_DETECT_ON_OFF_STABLE_MIN)
            {
                mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::OFF_SATBLE;
                mCountOffStable[channel] = 0;
                updateStateMachineTouchSensor10Hz(channel);
            }
        }
        else
        {
            LOG_DEBUG("RelayManagerTask", "Touch sensor: %d: on in step OFF_DETECTED-> change to Step idle", channel);
            mCountOffStable[channel] = 0;
            mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::IDLE;
            updateStateMachineTouchSensor10Hz(channel);
        }
        break;
    }

    case STEP_CHECK_TOUCH_SENSOR::OFF_SATBLE:
    {
        mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::PROCESS_CONTROL_RELAY;
        updateStateMachineTouchSensor10Hz(channel);
        break;
    }
    case STEP_CHECK_TOUCH_SENSOR::PROCESS_CONTROL_RELAY:
    {
        if (channel < MessageCommon::MAX_NUM_RELAY)
        {
            // Toggle relay state
            mRelay[channel]->Hal_Gpio_TogglePin();

            // Update control relay data
            AquaCtrl_ControlRelayData controlData;
            controlData.channel = channel;
            controlData.status_control = mRelay[channel]->Hal_Gpio_ReadPin();

            // Save status to flash
            ControlRelayMessage msgControlRelay = ControlRelayMessage(controlData);
            if (StoreFlashManager::getInstance()->saveRelayInforToFlash(msgControlRelay, channel))
            {
                LOG_INFO("RelayManagerTask", "Touch sensor: saved relay state to flash: channel=%d, value=%d", channel, (int)controlData.status_control);
            }
            else
            {
                LOG_ERROR("RelayManagerTask", "Touch sensor: failed to save relay state to flash: channel=%d, value=%d", channel, (int)controlData.status_control);
            }

        }
        else if (channel == INDEX_TOUCH_SENSOR_MAUNUAL)
        {
            mOSBase->semGive(gSemInputBtnConfigFromRelayTaskToWifiTask);
            LOG_INFO("RelayManagerTask", "Touch sensor: Manual button pressed, sent semaphore to Wifi Task for config mode");
        }
        else
        {
            LOG_ERROR("RelayManagerTask", "updateStateMachineTouchSensor10Hz: Invalid channel index %d", channel);
        }

        // change to idle
        LOG_DEBUG("RelayManagerTask", "Touch sensor: %d change to Step idle", channel);
        mCurrentStepCheckTouchSensor = STEP_CHECK_TOUCH_SENSOR::IDLE;
        updateStateMachineTouchSensor10Hz(channel);
        break;
    }
    default:
    {
        break;
    }
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
                    self->mRelay[i]->Hal_Gpio_WritePin(HalGpioAbstract::GPIO_PIN_RESET);
                }
            }
        }
    }
}
