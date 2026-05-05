#include "message/monitordatamessage.h"
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"

MonitorDataMessage::MonitorDataMessage()
    : AbstractMessage(MessageId::MONITOR_DATA_MESSAGE)
{
}

MonitorDataMessage::MonitorDataMessage(AquaCtrl_MonitorData monitorData)
    : AbstractMessage(MessageId::MONITOR_DATA_MESSAGE), mMonitorData(monitorData)
{
}

MonitorDataMessage::~MonitorDataMessage()
{
}

const AquaCtrl_MonitorData &MonitorDataMessage::getMonitorData() const
{
    return mMonitorData;
}

void MonitorDataMessage::setMonitorData(const AquaCtrl_MonitorData &monitorData)
{
    mMonitorData = monitorData;
}

bool MonitorDataMessage::packData(CodecMessage *msg)
{
    if (msg)
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(msg->mDataRaw, CodecMessage::MAX_DATA_LENGTH);
        if (!pb_encode(&ostream, AquaCtrl_MonitorData_fields, &mMonitorData))
        {
            return false;
        }
        msg->mMsgDataLength = ostream.bytes_written;
        msg->mMsgID = (uint8_t)this->mMessageId;
        return true;
    }
    return false;
}

bool MonitorDataMessage::unpackData(CodecMessage *msg)
{
    if (msg)
    {
        pb_istream_t istream = pb_istream_from_buffer(msg->mDataRaw, msg->mMsgDataLength);
        if (!pb_decode(&istream, AquaCtrl_MonitorData_fields, &mMonitorData))
        {
            return false;
        }
        return true;
    }
    return false;
}
