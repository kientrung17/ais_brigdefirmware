#include "monitoraquacontrolmessage.h"

MonitorAquaControlMessage::MonitorAquaControlMessage()
    : AbstractMessage(MessageId::MONITOR_AQUA_CONTROL_MESSAGE)
{
    mMonitor = AquaCtrl_MonitorAquaControlData_init_zero;
}

MonitorAquaControlMessage::MonitorAquaControlMessage(AquaCtrl_MonitorAquaControlData monitor)
    : AbstractMessage(MessageId::MONITOR_AQUA_CONTROL_MESSAGE), mMonitor(monitor)
{
}

MonitorAquaControlMessage::~MonitorAquaControlMessage() = default;

const AquaCtrl_MonitorAquaControlData &MonitorAquaControlMessage::getMonitor() const
{
    return mMonitor;
}

void MonitorAquaControlMessage::setMonitor(const AquaCtrl_MonitorAquaControlData &monitor)
{
    mMonitor = monitor;
}

bool MonitorAquaControlMessage::packData(CodecMessage *msg)
{
    if (!msg)
    {
        return false;
    }

    pb_ostream_t ostream = pb_ostream_from_buffer(msg->mDataRaw, CodecMessage::MAX_DATA_LENGTH);
    if (!pb_encode(&ostream, AquaCtrl_MonitorAquaControlData_fields, &mMonitor))
    {
        return false;
    }
    msg->mMsgDataLength = ostream.bytes_written;
    msg->mMsgID = static_cast<uint8_t>(this->mMessageId);
    return true;
}

bool MonitorAquaControlMessage::unpackData(CodecMessage *msg)
{
    if (!msg)
    {
        return false;
    }

    pb_istream_t istream = pb_istream_from_buffer(msg->mDataRaw, msg->mMsgDataLength);
    if (!pb_decode(&istream, AquaCtrl_MonitorAquaControlData_fields, &mMonitor))
    {
        return false;
    }
    return true;
}
