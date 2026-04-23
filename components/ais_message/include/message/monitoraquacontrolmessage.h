#ifndef MONITOR_AQUA_CONTROL_MESSAGE_H
#define MONITOR_AQUA_CONTROL_MESSAGE_H

#include "abstractmessage.h"
#include "protobuf/aquacontrol.pb.h"
#include "protobuf/pb_encode.h"
#include "protobuf/pb_decode.h"

class MonitorAquaControlMessage : public AbstractMessage
{
public:
    MonitorAquaControlMessage();
    MonitorAquaControlMessage(AquaCtrl_MonitorAquaControlData monitor);
    ~MonitorAquaControlMessage();

    const AquaCtrl_MonitorAquaControlData &getMonitor() const;
    void setMonitor(const AquaCtrl_MonitorAquaControlData &monitor);

    bool packData(CodecMessage *msg) override;
    bool unpackData(CodecMessage *msg) override;

private:
    AquaCtrl_MonitorAquaControlData mMonitor{};
};

#endif // MONITOR_AQUA_CONTROL_MESSAGE_H
