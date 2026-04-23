#ifndef CONTROL_RELAY_H
#define CONTROL_RELAY_H
#include "abstractmessage.h"
#include "messagecommon.h"
#include "protobuf/aquacontrol.pb.h"

class ControlRelayMessage : public AbstractMessage
{
public:
    ControlRelayMessage();
    ControlRelayMessage(AquaCtrl_ControlRelayData ping);
    ControlRelayMessage(int channel, bool status);
    ~ControlRelayMessage();

    const AquaCtrl_ControlRelayData &getControlRelayMessage() const;
    void setControlRelayMessage(const AquaCtrl_ControlRelayData &ControlRelayMessage);

    virtual bool packData(CodecMessage *msg) override;
    virtual bool unpackData(CodecMessage *msg) override;

private:
    AquaCtrl_ControlRelayData mControlRelayMessage {};
};

#endif