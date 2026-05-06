#include "task/mqttmanagertask.h"
#include "HAL/HAL_ESP32/mqttclientesp32.h"
#include "common/common.h"
#include "loggermanager.h"
#include "message/controlstatusdatamessage.h"
#include "message/controlrelaymessage.h"
#include "codec/codecmessage.h"

MqttManagerTask::MqttManagerTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
}

MqttManagerTask::~MqttManagerTask()
{
    // Không delete mMqttClient vì MqttClientAbstract không có virtual destructor
    // và Task này chạy vô hạn, không bao giờ bị destroy trong suốt vòng đời Firmware.
}

void MqttManagerTask::onInitProcess()
{
    LOG_INFO("MqttManagerTask", "Initializing MqttManagerTask...");
    
    // Format topics using Device ID
    char devIdStr[32];
    snprintf(devIdStr, sizeof(devIdStr), "%llu", (unsigned long long)gDeviceID);
    
    mTopicTelemetry = std::string("devices/") + devIdStr + "/telemetry";
    mTopicCommand = std::string("devices/") + devIdStr + "/commands";

    initMqttClient();
}

void MqttManagerTask::initMqttClient()
{
    const auto &config = gConfigSystem.getConfigSystem();
    
    // Check if URI is configured
    if (strlen(config.uriMqtt) == 0) {
        LOG_INFO("MqttManagerTask", "MQTT Broker URI is empty, skipping MQTT initialization");
        return;
    }

    std::string brokerUri = config.uriMqtt;
    int port = config.portMqtt;
    if (port == 0) port = 1883; // default port
    
    std::string username = config.mqttUser;
    std::string password = config.mqttPass;

    mMqttClient = new MqttClientEsp32(brokerUri, port, username, password);

    mMqttClient->setReceivedMessageCallback(std::bind(&MqttManagerTask::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    mMqttClient->setSubcribeToppicCallback(std::bind(&MqttManagerTask::onSubscribeTopic, this));
}

void MqttManagerTask::onTimer100HzProcess()
{
    mCounter100Hz++;

    // Check Network state using EventGroup
    EventBits_t networkBits = xEventGroupGetBits(gEventGroupNetworkState);
    bool wifiConnected = (networkBits & BIT_WIFI_CONNECTED) != 0;

    if (wifiConnected && !mIsWifiConnected) {
        mIsWifiConnected = true;
        if (mMqttClient) {
            LOG_INFO("MqttManagerTask", "Wifi connected, starting MQTT client...");
            mMqttClient->start();
        }
    } else if (!wifiConnected && mIsWifiConnected) {
        mIsWifiConnected = false;
        if (mMqttClient) {
            LOG_INFO("MqttManagerTask", "Wifi disconnected, stopping MQTT client...");
            mMqttClient->stop();
        }
    }
}

void MqttManagerTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    // Check if data available in Telemetry queue
    if (queue_sem == gQueuePowerDataToMqtt) {
        ControlStatusDataMessage msg;
        if (mOSBase->queueReceive(gQueuePowerDataToMqtt, &msg, 0) == OSBase::QUEUE_OK) {
            if (mMqttClient && mMqttClient->isMqttConnected()) {
                CodecMessage codecMsg;
                if (msg.packData(&codecMsg)) {
                    // Publish raw bytes over MQTT
                    uint16_t totalLen = codecMsg.mMsgDataLength; 
                    mMqttClient->publish(mTopicTelemetry, (const char*)codecMsg.mDataRaw, totalLen, MqttClientAbstract::QOS_1);
                    // LOG_DEBUG("MqttManagerTask", "Published telemetry to topic: %s", mTopicTelemetry.c_str());
                } else {
                    LOG_ERROR("MqttManagerTask", "Failed to pack telemetry data");
                }
            }
        }
    }
}

void MqttManagerTask::onSubscribeTopic()
{
    LOG_INFO("MqttManagerTask", "MQTT Connected! Subscribing to topic: %s", mTopicCommand.c_str());
    if (mMqttClient) {
        mMqttClient->subscribe(mTopicCommand, MqttClientAbstract::QOS_1);
    }
}

void MqttManagerTask::onMessageReceived(const std::string &topic, const char *data, const uint16_t &len)
{
    LOG_INFO("MqttManagerTask", "Received MQTT message on topic: %s, length: %d", topic.c_str(), len);
    
    if (topic == mTopicCommand) {
        // Parse the incoming Protobuf data into a ControlRelayMessage
        CodecMessage codecMsg;
        if (len <= CodecMessage::MAX_DATA_LENGTH) {
            codecMsg.mMsgID = static_cast<int>(AbstractMessage::MessageId::CONTROL_RELAY_MESSAGE);
            codecMsg.mMsgDataLength = len;
            memcpy(codecMsg.mDataRaw, data, len);

            ControlRelayMessage relayMsg;
            if (relayMsg.unpackData(&codecMsg)) {
                // Successfully parsed, send it to gQueueRelayControlCmd
                if (mOSBase->queueSend(gQueueRelayControlCmd, &relayMsg) != OSBase::QUEUE_OK) {
                    LOG_ERROR("MqttManagerTask", "Failed to send to gQueueRelayControlCmd, queue might be full!");
                } else {
                    LOG_INFO("MqttManagerTask", "Command successfully sent to RelayManagerTask");
                }
            } else {
                LOG_ERROR("MqttManagerTask", "Failed to unpack ControlRelayMessage from MQTT payload");
            }
        } else {
            LOG_ERROR("MqttManagerTask", "MQTT message length exceeds MAX_DATA_LENGTH");
        }
    }
}
