#pragma once
#include "taskabstract.h"
#include "HAL/HAL_ABSTRACT/udpabstract.h"
#include "codec/codecmessage.h"
#include <string>

class ConfigSystemTask : public TaskAbstract
{
public:
    static constexpr uint16_t DIV_COUNTER_100Hz = 1; // Run every tick (10ms)
    
    ConfigSystemTask(std::string nameTask, int numElementQueueSet);
    ~ConfigSystemTask();

    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    virtual void onInitProcess() override;

private:
    void processReadDataFromUDP();
    void processorPingMessage();
    void processorCommandSystemMessage(CodecMessage *msg);
    void processorConfigSystemMessage(CodecMessage *msg);
    void sendPingResponseToAppCenter();

private:
    UdpAbstract *mUdp{nullptr};
    uint32_t mCounter100Hz{0};
};
