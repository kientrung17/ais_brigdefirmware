#include "configsystemmessage.h"
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"


ConfigSystemMessage::ConfigSystemMessage()
    : AbstractMessage(MessageId::CONFIG_SYSTEM_MESSAGE)
{
}
ConfigSystemMessage::ConfigSystemMessage(System_ConfigSystemData config)
    : AbstractMessage(MessageId::CONFIG_SYSTEM_MESSAGE), mConfig(config)
{
}

ConfigSystemMessage::~ConfigSystemMessage()
{
}

const System_ConfigSystemData &ConfigSystemMessage::getConfigSystem() const
{
    return mConfig;
}

void ConfigSystemMessage::setConfigSyste(const System_ConfigSystemData &config)
{
    mConfig = config;
}

bool ConfigSystemMessage::packData(CodecMessage *msg)
{
    if (msg)
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(msg->mDataRaw, CodecMessage::MAX_DATA_LENGTH);
        if (!pb_encode(&ostream, System_ConfigSystemData_fields, &mConfig))
        {
            return false;
        }
        msg->mMsgDataLength = ostream.bytes_written;
        msg->mMsgID = (uint8_t)this->mMessageId;
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigSystemMessage::unpackData(CodecMessage *msg)
{
    if (msg)
    {
        pb_istream_t istream = pb_istream_from_buffer(msg->mDataRaw, msg->mMsgDataLength);
        if (!pb_decode(&istream, System_ConfigSystemData_fields, &mConfig))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

void ConfigSystemMessage::setSsid(const char *value)
{
    strncpy(mConfig.ssidWifiSta, value, sizeof(mConfig.ssidWifiSta) - 1);
    mConfig.ssidWifiSta[sizeof(mConfig.ssidWifiSta) - 1] = '\0'; // Đảm bảo null-terminated
}

void ConfigSystemMessage::setPass(const char *value)
{
    strncpy(mConfig.passWifiSta, value, sizeof(mConfig.passWifiSta) - 1);
    mConfig.passWifiSta[sizeof(mConfig.passWifiSta) - 1] = '\0'; // Đảm bảo null-terminated
}

void ConfigSystemMessage::setUriMqtt(const char *value)
{
    strncpy(mConfig.uriMqtt, value, sizeof(mConfig.uriMqtt) - 1);
    mConfig.uriMqtt[sizeof(mConfig.uriMqtt) - 1] = '\0'; // Đảm bảo null-terminated
}

void ConfigSystemMessage::setPortMqtt(int value)
{
    mConfig.portMqtt = value; // Chuyển chuỗi sang int
}

void ConfigSystemMessage::setMqttUser(const char *value)
{
    strncpy(mConfig.mqttUser, value, sizeof(mConfig.mqttUser) - 1);
    mConfig.mqttUser[sizeof(mConfig.mqttUser) - 1] = '\0'; // Đảm bảo null-terminated
}

void ConfigSystemMessage::setMqttPass(const char *value)
{
    strncpy(mConfig.mqttPass, value, sizeof(mConfig.mqttPass) - 1);
    mConfig.mqttPass[sizeof(mConfig.mqttPass) - 1] = '\0'; // Đảm bảo null-terminated
}
