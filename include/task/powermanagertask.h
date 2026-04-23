#pragma once
#include "taskabstract.h"
#include "common/common.h"
#include <array>
#include <string>
#include "HAL_ABSTRACT/halgpioabstract.h"

class PowerManagerTask : public TaskAbstract
{
public:
    static constexpr size_t NUM_CHANNEL = 3; // 2 chnanel stc + 1 channel check pin
    static constexpr size_t INDEX_CHANNEL_CHECK_PIN = 2;
    static constexpr size_t AVG_WINDOW = 10;
    static constexpr float MV_TO_VOLT = 1.0f / 1000.0f; // Input tu HAL la mV
    static constexpr uint8_t MAX_AMPLE_SENSOR = 20;     // Max ampe of sensor
    static constexpr float VOL_AT_0A = 1.65;            // vol tại 0A 1.65V
    static constexpr float VOL_AT_MAX_A = 2.65;         // Vol tại Max A //2.65v
    static constexpr uint16_t INTERVAL_SEND_MONITOR_1Hz = 100;
    static constexpr uint16_t INTERVAL_CHECK_WARNING_10S = 1000;
    static constexpr uint16_t INTERVAL_MANAGE_CHARGE_PIN = 1000; // check every 10s
    // notify phase, electric
    static constexpr bool STATUS_GPIO_LOST_PHASE = false;
    static constexpr bool STATUS_GPIO_LOST_ELECTRIC = false;
    // pin
    static constexpr float PARAM_CONVERT_TO_VOL_PIN = 2.0f;
    static constexpr float THRESHOULD_VOL_PIN_LOW = 3.7f;  // sac khi < 3v
    static constexpr float THRESHOULD_VOL_PIN_FULL = 4.18f; 
    static constexpr uint8_t COUNTER_VERIFY_PIN = 6; //check muc dien ap 6 lan truoc khi dieu khien sac
    static constexpr uint16_t MAX_COUNTER_TIME_CHARGE = 540; // 540 * 10s = 1.5 gio

    PowerManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioLostPhase, HalGpioAbstract *gpioLostElectric,
                     HalGpioAbstract *gpioChargePin);
    ~PowerManagerTask();

    struct SampleResult
    {
        float voltage[NUM_CHANNEL]{};
        uint16_t avgRaw[NUM_CHANNEL]{};
        float ampe[NUM_CHANNEL]{};
    };

    void onTimer100HzProcess() override;
    void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    void onInitProcess() override;

    const SampleResult &latest() const;
    bool isLostPhase() const;
    bool isLostElectric() const;

private:
    struct MovingAverage
    {
        std::array<uint16_t, AVG_WINDOW> buf{};
        size_t idx{0};
        size_t count{0};
        uint32_t sum{0};
        uint16_t push(uint16_t v);
    };

    void processSamples(const uint16_t samples[NUM_CHANNEL]);
    float toVoltage(uint16_t mv) const;
    void processManageChargePin();

private:
    MovingAverage mFilters[NUM_CHANNEL];
    SampleResult mResult{};
    uint32_t mCounter100Hz{0};
    HalGpioAbstract *mGPIOLostPhase{nullptr};
    HalGpioAbstract *mGPIOLostElectric{nullptr};
    HalGpioAbstract *mGPIOChargePin{nullptr};
    // check phase
    bool mIsSystemLostPhase{false};
    bool mIsSystemLostElectric{false};
    // vol pin
    float mVolPin{0};
    uint8_t mCounterVerifyCharge {0};
    uint32_t mCounterCharge {0};
    bool mIsPinLow {false};
};
