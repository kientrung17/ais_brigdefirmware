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

    if (mCounter100Hz % INTERVAL_MANAGE_CHARGE_PIN == 0)
    {
        processManageChargePin();
    }
}

void PowerManagerTask::processSampleResult(const SampleResult &result)
{
    mResult = result;

    // ---- convert to vol pin (channel 2) ----
    mVolPin = mResult.voltage[INDEX_CHANNEL_CHECK_PIN] * PARAM_CONVERT_TO_VOL_PIN;

    // ---- Build ControlStatusDataMessage (Protobuf) ----
    // AmpeChannel1x100: Ampe channel 1 nhân 100 (tránh float trên App)
    // Ví dụ: 1.23A -> 123
    AquaCtrl_ControlStatusData statusData = AquaCtrl_ControlStatusData_init_zero;
    statusData.gatewayId        = (uint32_t)(gDeviceID & 0xFFFFFFFF);
    statusData.AmpeChannel1x100 = (uint32_t)(mResult.ampe[0] * 100.0f);
    statusData.AmpeChannel2x100 = (uint32_t)(mResult.ampe[1] * 100.0f);
    statusData.IsPowerLostPhare = mIsSystemLostPhase  ? 1u : 0u;
    statusData.IsLostElectric   = mIsSystemLostElectric ? 1u : 0u;

    // Ghi log moi 10 lan nhan (khong spam uart)
    static uint32_t logCounter = 0;
    if (++logCounter % 10 == 0) {
        LOG_DEBUG("PowerManagerTask",
                  "A1=%.2fA A2=%.2fA LostPhase=%d LostElec=%d",
                  mResult.ampe[0], mResult.ampe[1],
                  (int)mIsSystemLostPhase, (int)mIsSystemLostElectric);
    }

    // ---- Gửi sang WifiTask qua Queue ----
    if (gQueuePowerDataToWifiTask != 0)
    {
        ControlStatusDataMessage msg(statusData);
        if (mOSBase->queueSend(gQueuePowerDataToWifiTask, &msg) != OSBase::QUEUE_OK)
        {
            LOG_ERROR("PowerManagerTask", "queueSend gQueuePowerDataToWifiTask failed");
        }
    }
}

void PowerManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    if (queue_sem == gQueueADCValueToPowerManageTask)
    {
        SampleResult incomingResult{};
        if (mOSBase->queueReceive(gQueueADCValueToPowerManageTask, &incomingResult, 0) == OSBase::QUEUE_OK)
        {
            processSampleResult(incomingResult);
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
