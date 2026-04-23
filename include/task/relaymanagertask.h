#ifndef RELAYMANAGERTASK_H
#define RELAYMANAGERTASK_H
#include "taskabstract.h"
#include "message/controlrelaymessage.h"
#include "HAL/HAL_ABSTRACT/halgpioabstract.h"
#include "message/messagecommon.h"
#include "HAL/HAL_ABSTRACT/touchsensorabstract.h"

class RelayManagerTask : public TaskAbstract
{
public:
    // touch sensor
    static constexpr uint16_t MAX_NUM_TOUCH_SENSOR = MessageCommon::MAX_NUM_RELAY + 1; // 6 relay + 1 manual
    static constexpr uint16_t INDEX_TOUCH_SENSOR_MAUNUAL = 6;
    static constexpr uint16_t DIV_COUNTER_10Hz = 10;
    static constexpr uint8_t TIME_FILTER_TOUCH_SENSOR_HARDWARE = 20; // 20ms
    static constexpr uint16_t NUM_SAMPLE_CALIB_BASELINE_TOUCH_SENSOR = 200;
    static constexpr uint8_t NUM_DETECT_ON_OFF_STABLE_MIN = 10; // 1s
    static constexpr uint8_t NUM_DETECT_ON_OFF_STABLE_MAX = 30; // 3s

    static constexpr uint8_t RELATIVE_THRESHOULD_PERCENT = 20; // ON when descease 20%
    static constexpr uint16_t NUM_SAMPLE_DETECT_ON_OFF_TOUCH_SENSOR = 3;    // 3 smaple , frequence read 10hz => 300ms detect on off
    RelayManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioRelay[],
                     TouchSensorAbstract *touchSensors[]);
    ~RelayManagerTask();

    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    virtual void onInitProcess() override;

    enum class STEP_CHECK_TOUCH_SENSOR
    {
        IDLE = 0,
        ON_DETECTED,
        ON_SATBLE,
        OFF_DETECTED,
        OFF_SATBLE,
        PROCESS_CONTROL_RELAY
    };

private:
    void processControlRelayMessage(const AquaCtrl_ControlRelayData &control);

    void updateStateMachineTouchSensor10Hz(uint8_t channel);

private:
    uint32_t mCounter100Hz{0};
    HalGpioAbstract *mRelay[MessageCommon::MAX_NUM_RELAY];
    TouchSensorAbstract *mTouchSensor[MAX_NUM_TOUCH_SENSOR];
    bool touchSensorPrevState[MAX_NUM_TOUCH_SENSOR]{false};
    uint32_t mCounterStateOffTouchSensor[MAX_NUM_TOUCH_SENSOR]{0};
    STEP_CHECK_TOUCH_SENSOR mCurrentStepCheckTouchSensor{STEP_CHECK_TOUCH_SENSOR::IDLE};
    uint32_t mCountOnStable[MAX_NUM_TOUCH_SENSOR]{0};
    uint32_t mCountOffStable[MAX_NUM_TOUCH_SENSOR]{0};
};
#endif
