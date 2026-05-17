#ifndef RELAYMANAGERTASK_H
#define RELAYMANAGERTASK_H
#include "taskabstract.h"
#include "message/controlrelaymessage.h"
#include "HAL/HAL_ABSTRACT/halgpioabstract.h"
#include "message/messagecommon.h"

class RelayManagerTask : public TaskAbstract
{
public:
    RelayManagerTask(std::string nameTask, int numElementQueueSet, HalGpioAbstract *gpioRelay[]);
    ~RelayManagerTask();

    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    virtual void onInitProcess() override;

private:
    void processControlRelayMessage(const AquaCtrl_ControlRelayData &control);

    static void emergencyTask(void *arg);

private:
    uint32_t mCounter100Hz{0};
    HalGpioAbstract *mRelay[MessageCommon::MAX_NUM_RELAY];
};
#endif
