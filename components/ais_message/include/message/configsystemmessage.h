#ifndef CONFIG_SYSTEM_MESSAGE_H
#define CONFIG_SYSTEM_MESSAGE_H

#include "abstractmessage.h"
#include "stdint.h"
#include "protobuf/aissystem.pb.h"

class ConfigSystemMessage : public AbstractMessage
{
public:
    ConfigSystemMessage();
    ConfigSystemMessage(System_ConfigSystemData config);
    ~ConfigSystemMessage();

    const System_ConfigSystemData &getConfigSystem() const;
    void setConfigSyste(const System_ConfigSystemData &config);

    virtual bool packData(CodecMessage *msg) override;
    virtual bool unpackData(CodecMessage *msg) override;

    // getter setter
    void setSsid(const char *value);
    void setPass(const char *value);
    void setUriMqtt(const char *value);
    void setPortMqtt(int value);
    void setMqttUser(const char *value);
    void setMqttPass(const char *value);

private:
    System_ConfigSystemData mConfig {};
};

#endif
