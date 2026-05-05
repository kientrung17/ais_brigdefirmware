#include "message/controlrelaymessage.h"
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"

ControlRelayMessage::ControlRelayMessage()
    : AbstractMessage(MessageId::CONTROL_RELAY_MESSAGE)
{
}

ControlRelayMessage::ControlRelayMessage(AquaCtrl_ControlRelayData control)
    : AbstractMessage(MessageId::CONTROL_RELAY_MESSAGE), mControlRelayMessage(control)
{
}

ControlRelayMessage::ControlRelayMessage(int channel, bool status)
    : AbstractMessage(MessageId::CONTROL_RELAY_MESSAGE)
{
    if (channel < MessageCommon::MAX_NUM_RELAY)
    {
        mControlRelayMessage.channel = channel;
        mControlRelayMessage.status_control = status;
    }
}

ControlRelayMessage::~ControlRelayMessage()
{
}

const AquaCtrl_ControlRelayData &ControlRelayMessage::getControlRelayMessage() const
{
    return mControlRelayMessage;
}

void ControlRelayMessage::setControlRelayMessage(const AquaCtrl_ControlRelayData &controlRelayMessage)
{
    mControlRelayMessage = controlRelayMessage;
}

bool ControlRelayMessage::packData(CodecMessage *msg)
{
    if (msg)
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(msg->mDataRaw, CodecMessage::MAX_DATA_LENGTH);
        if (!pb_encode(&ostream, AquaCtrl_ControlRelayData_fields, &mControlRelayMessage))
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

bool ControlRelayMessage::unpackData(CodecMessage *msg)
{
    if (msg)
    {
        pb_istream_t istream = pb_istream_from_buffer(msg->mDataRaw, msg->mMsgDataLength);
        if (!pb_decode(&istream, AquaCtrl_ControlRelayData_fields, &mControlRelayMessage))
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
