#pragma once
#include "taskabstract.h"
#include "HAL/HAL_ABSTRACT/mqttclientabstract.h"
#include <string>

class MqttManagerTask : public TaskAbstract
{
public:
    static constexpr uint16_t DIV_COUNTER_100Hz = 1;

    MqttManagerTask(std::string nameTask, int numElementQueueSet);
    ~MqttManagerTask();

    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;
    virtual void onInitProcess() override;

private:
    void initMqttClient();
    void onMessageReceived(const std::string &topic, const char *data, const uint16_t &len);
    void onSubscribeTopic();

private:
    MqttClientAbstract *mMqttClient{nullptr};
    uint32_t mCounter100Hz{0};
    bool mIsWifiConnected{false};
    
    std::string mTopicTelemetry{""};
    std::string mTopicCommand{""};
};
