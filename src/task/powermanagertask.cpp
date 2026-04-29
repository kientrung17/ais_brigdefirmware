#include "task/powermanagertask.h"
#include "loggermanager.h"

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

const PowerManagerTask::SampleResult &PowerManagerTask::latest() const
{
    return mResult;
}

bool PowerManagerTask::isLostPhase() const
{
    return mIsSystemLostPhase;
}

bool PowerManagerTask::isLostElectric() const
{
    return mIsSystemLostElectric;
}

uint16_t PowerManagerTask::MovingAverage::push(uint16_t v)
{
    // Rolling average trên cửa sổ cố định.
    sum -= buf[idx];
    buf[idx] = v;
    sum += v;
    idx = (idx + 1) % AVG_WINDOW;
    if (count < AVG_WINDOW)
    {
        ++count;
    }
    return static_cast<uint16_t>(sum / (count ? count : 1));
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

    if (mCounter100Hz % INTERVAL_CHECK_WARNING_10S == 0) // 10s
    {
        if (mGPIOLostPhase)
        {
            mIsSystemLostPhase = (mGPIOLostPhase->Hal_Gpio_ReadPin() == STATUS_GPIO_LOST_PHASE);
        }
        if (mGPIOLostElectric)
        {
            mIsSystemLostElectric = (mGPIOLostElectric->Hal_Gpio_ReadPin() == STATUS_GPIO_LOST_ELECTRIC);
        }
    }

    if (mCounter100Hz % INTERVAL_MANAGE_CHARGE_PIN == 0)
    {
        processManageChargePin();
    }
}

void PowerManagerTask::processSamples(const uint16_t samples[NUM_CHANNEL])
{
    for (size_t i = 0; i < NUM_CHANNEL; ++i)
    {
        // get value and filter
        uint16_t avg = mFilters[i].push(samples[i]);
        mResult.avgRaw[i] = avg;
        float vol = toVoltage(avg);
        mResult.voltage[i] = vol;

        // convert to vol pin
        if (i == INDEX_CHANNEL_CHECK_PIN)
        {
            mVolPin = mResult.voltage[i] * PARAM_CONVERT_TO_VOL_PIN;
        }
        // convert to ampe
        else
        {
            mResult.ampe[i] = MAX_AMPLE_SENSOR * (std::abs(mResult.voltage[i] - VOL_AT_0A)) / (VOL_AT_MAX_A - VOL_AT_0A);
        }
    }

    // Gửi data sang WifiTask qua Queue
    if (gQueuePowerDataToWifiTask != 0)
    {
        if (mOSBase->queueSend(gQueuePowerDataToWifiTask, &mResult) != OSBase::QUEUE_OK)
        {
            LOG_ERROR("PowerManagerTask", "queueSend gQueuePowerDataToWifiTask failed");
        }
    }
}

void PowerManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    if (queue_sem == gQueueADCValueToPowerManageTask)
    {
        uint16_t adcSamples[NUM_CHANNEL] = {0};
        if (mOSBase->queueReceive(gQueueADCValueToPowerManageTask, adcSamples, 0) == OSBase::QUEUE_OK)
        {
            processSamples(adcSamples);
        }
        else
        {
            LOG_ERROR("PowerManagerTask", "queueReceive gQueueADCValueToPowerManageTask failed");
        }
    }
    else
    {
        LOG_ERROR("PowerManagerTask", "Unknown queue/sem: %d", queue_sem);
    }
}

void PowerManagerTask::processManageChargePin()
{
    if (mGPIOChargePin == nullptr)
    {
        LOG_ERROR("PowerManagerTask", "mGPIOChargePin is null????");
        return;
    }
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
