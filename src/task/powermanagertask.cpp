#include "task/powermanagertask.h"
#include "loggermanager.h"
#include "message/controlstatusdatamessage.h"
#include "common/common.h"

PowerManagerTask::PowerManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioLostPhase,
                                   HalGpioAbstract *gpioLostElectric, HalGpioAbstract *gpioChargePin)
    : TaskAbstract(nameTask, numElementQueueSet)
{
    mGPIOLostElectric = gpioLostElectric;
    mGPIOLostPhase = gpioLostPhase;
    mGPIOChargePin = gpioChargePin;
    if (mGPIOLostElectric == nullptr || mGPIOLostPhase == nullptr)
    {
        LOG_ERROR("PowerManagerTask", "Gpio phase or gpio eletric is null??");
    }
}

PowerManagerTask::~PowerManagerTask() = default;



bool PowerManagerTask::isLostPhase() const
{
    return mIsSystemLostPhase;
}

bool PowerManagerTask::isLostElectric() const
{
    return mIsSystemLostElectric;
}

float PowerManagerTask::toVoltage(uint16_t mv) const
{
    // Đầu vào đã là mV (HAL đã hiệu chuẩn); đổi sang Volt.
    return static_cast<float>(mv) * MV_TO_VOLT;
}

void PowerManagerTask::onInitProcess()
{
    LOG_INFO("PowerManagerTask", "Init task");
}

void PowerManagerTask::onTimer100HzProcess()
{
    // Co the dung cho monitor/diagnostic sau nay
    mCounter100Hz++;
    if (mCounter100Hz % INTERVAL_SEND_MONITOR_1Hz == 0)
    {
        // reserved for future monitor/diagnostic hooks.
    }

    // CHECK LOST PHASE CONTINUOUSLY FOR SAFETY (100Hz)
    if (mGPIOLostPhase)
    {
        mIsSystemLostPhase = (mGPIOLostPhase->Hal_Gpio_ReadPin() == STATUS_GPIO_LOST_PHASE);
        
        // --- E-STOP TRIGGER: LOST PHASE ---
        if (mIsSystemLostPhase) {
            if (mOSBase->isStarted() && gEmergencyEventGroup != nullptr) {
                xEventGroupSetBits(gEmergencyEventGroup, BIT_ESTOP_LOST_PHASE);
            }
        }
    }

    if (mCounter100Hz % INTERVAL_CHECK_WARNING_10S == 0) // 10s
    {
        if (mGPIOLostElectric)
        {
            mIsSystemLostElectric = (mGPIOLostElectric->Hal_Gpio_ReadPin() == STATUS_GPIO_LOST_ELECTRIC);
        }
    }

    // Update Shared Data Store (Lock-free)
    gSharedData.is_lost_phase.store(mIsSystemLostPhase, std::memory_order_relaxed);
    gSharedData.is_lost_electric.store(mIsSystemLostElectric, std::memory_order_relaxed);

    if (mCounter100Hz % INTERVAL_MANAGE_CHARGE_PIN == 0)
    {
        processManageChargePin();
    }
}

void PowerManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    // No more queues to process here since we use SharedDataStore
}

void PowerManagerTask::processManageChargePin()
{
    if (mGPIOChargePin == nullptr)
    {
        LOG_ERROR("PowerManagerTask", "mGPIOChargePin is null????");
        return;
    }

    float rawVoltage = gSharedData.voltage_pin.load(std::memory_order_relaxed);
    mVolPin = rawVoltage * PARAM_CONVERT_TO_VOL_PIN;
    if (mVolPin < THRESHOULD_VOL_PIN_LOW && mIsPinLow == false)
    {
        // verify pin low
        if (mCounterVerifyCharge < COUNTER_VERIFY_PIN)
        {
            mCounterVerifyCharge++;
        }
        else
        {
            mIsPinLow = true;
            LOG_DEBUG("PowerManagerTask", "Veryfy pin low => start charge pin");
        }
    }
    else
    {
        mCounterVerifyCharge = 0;
    }

    // check charge
    if (mIsPinLow)
    {
        mCounterCharge++;
        // stop charge if pin full or timeout
        if (mVolPin >= THRESHOULD_VOL_PIN_FULL || mCounterCharge >= MAX_COUNTER_TIME_CHARGE)
        {
            mIsPinLow = false;
            // off change pin
            mGPIOChargePin->Hal_Gpio_WritePin(HalGpioAbstract::GpioPinState::GPIO_PIN_RESET);
            LOG_DEBUG("PowerManagerTask", "Pin full or timeout => stop charge");
        }
        else
        {
            // charge pin
            mGPIOChargePin->Hal_Gpio_WritePin(HalGpioAbstract::GpioPinState::GPIO_PIN_SET);
            LOG_DEBUG("PowerManagerTask", "Pin is charging");
        }
    }
    else
    {
        mCounterCharge = 0;
    }
}
