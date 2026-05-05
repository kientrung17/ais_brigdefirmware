#ifndef MONITOR_DATA_MESSAGE_H
#define MONITOR_DATA_MESSAGE_H

#include "abstractmessage.h"
#include "protobuf/aquacontrol.pb.h"

class MonitorDataMessage : public AbstractMessage
{
public:
    MonitorDataMessage();
    MonitorDataMessage(AquaCtrl_MonitorData monitorData);
    ~MonitorDataMessage();

    const AquaCtrl_MonitorData &getMonitorData() const;
    void setMonitorData(const AquaCtrl_MonitorData &monitorData);

    virtual bool packData(CodecMessage *msg) override;
    virtual bool unpackData(CodecMessage *msg) override;

private:
    AquaCtrl_MonitorData mMonitorData {};
};

#endif
