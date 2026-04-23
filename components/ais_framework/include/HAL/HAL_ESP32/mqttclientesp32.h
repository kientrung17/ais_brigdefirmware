#pragma once

#include "mqtt_client.h"
#include <string>
#include "mqttclientabstract.h"

class MqttClientEsp32 : public MqttClientAbstract
{
public:
    MqttClientEsp32(const std::string &uri, int port, std::string username, std::string pass);
    ~MqttClientEsp32();

    // setcall back before using mqtt client
    virtual void setReceivedMessageCallback(MessageCallback cb) override;
    virtual void setSubcribeToppicCallback(SubscribeTopicCallback cb) override;

    bool start() override;
    void stop() override;
    bool reconnect() override;
    bool publish(const std::string &topic, const std::string &message, int qos = MqttClientAbstract::QOS_0) override;
    bool publish(const std::string &topic, const char* message, uint16_t length, int qos = MqttClientAbstract::QOS_0) override;
    bool subscribe(const std::string &topic, int qos = 1) override;
    bool unsubscribe(const std::string &topic) override;

    bool isMqttConnected() override;

private:
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    void handleEvent(esp_mqtt_event_handle_t event);

private:
    esp_mqtt_client_handle_t mClient{nullptr};
    std::string mBrokerUri{""};
    int mPort{0};
    std::string mUsername{""};
    std::string mPassword{""};
    char mClientId[32] {0};
};
