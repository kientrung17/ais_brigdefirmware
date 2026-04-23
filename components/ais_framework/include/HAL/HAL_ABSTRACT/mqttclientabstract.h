#pragma once

#include <string>
#include <functional>

class MqttClientAbstract
{
public:
    using MessageCallback = std::function<void(const std::string &topic, const char *data, const uint16_t &len)>;
    using SubscribeTopicCallback = std::function<void()>;

    static constexpr int QOS_0 = 0;
    static constexpr int QOS_1 = 1;
    static constexpr int QOS_2 = 2;

    // setcall back before using mqtt client
    virtual void setReceivedMessageCallback(MessageCallback cb) = 0;
    virtual void setSubcribeToppicCallback(SubscribeTopicCallback cb) = 0;

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool reconnect() = 0;
    virtual bool publish(const std::string &topic, const std::string &message, int qos = MqttClientAbstract::QOS_0) = 0;
    virtual bool publish(const std::string &topic, const char *message, uint16_t length, int qos = MqttClientAbstract::QOS_0) = 0;
    virtual bool subscribe(const std::string &topic, int qos = 1) = 0;
    virtual bool unsubscribe(const std::string &topic) = 0;

    virtual bool isMqttConnected() = 0;

protected:
    MessageCallback mMessageCallback;
    SubscribeTopicCallback mSubcribeTopicFunction;
    bool mIsConnected{false};
};
