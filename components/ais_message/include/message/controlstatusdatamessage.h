#ifndef CONTROL_STATUS_DATA_MESSAGE_H
#define CONTROL_STATUS_DATA_MESSAGE_H

#include "abstractmessage.h"
#include "protobuf/aquacontrol.pb.h"

class ControlStatusDataMessage : public AbstractMessage
{
public:
    ControlStatusDataMessage();
    ControlStatusDataMessage(AquaCtrl_ControlStatusData controlStatusData);
    ~ControlStatusDataMessage();

    const AquaCtrl_ControlStatusData &getControlStatusData() const;
    void setControlStatusData(const AquaCtrl_ControlStatusData &controlStatusData);

    virtual bool packData(CodecMessage *msg) override;
    virtual bool unpackData(CodecMessage *msg) override;

private:
    AquaCtrl_ControlStatusData mControlStatusData {};
};

#endif
