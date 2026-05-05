#include "message/controlstatusdatamessage.h"
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"

ControlStatusDataMessage::ControlStatusDataMessage()
    : AbstractMessage(MessageId::CONTROL_STATUS_DATA_MESSAGE)
{
}

ControlStatusDataMessage::ControlStatusDataMessage(AquaCtrl_ControlStatusData controlStatusData)
    : AbstractMessage(MessageId::CONTROL_STATUS_DATA_MESSAGE), mControlStatusData(controlStatusData)
{
}

ControlStatusDataMessage::~ControlStatusDataMessage()
{
}

const AquaCtrl_ControlStatusData &ControlStatusDataMessage::getControlStatusData() const
{
    return mControlStatusData;
}

void ControlStatusDataMessage::setControlStatusData(const AquaCtrl_ControlStatusData &controlStatusData)
{
    mControlStatusData = controlStatusData;
}

bool ControlStatusDataMessage::packData(CodecMessage *msg)
{
    if (msg)
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(msg->mDataRaw, CodecMessage::MAX_DATA_LENGTH);
        if (!pb_encode(&ostream, AquaCtrl_ControlStatusData_fields, &mControlStatusData))
        {
            return false;
        }
        msg->mMsgDataLength = ostream.bytes_written;
        msg->mMsgID = (uint8_t)this->mMessageId;
        return true;
    }
    return false;
}

bool ControlStatusDataMessage::unpackData(CodecMessage *msg)
{
    if (msg)
    {
        pb_istream_t istream = pb_istream_from_buffer(msg->mDataRaw, msg->mMsgDataLength);
        if (!pb_decode(&istream, AquaCtrl_ControlStatusData_fields, &mControlStatusData))
        {
            return false;
        }
        return true;
    }
    return false;
}
