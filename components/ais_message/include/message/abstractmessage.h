/*
 * innermessage.h
 *
 *  Created on: May 17, 2024
 *      Author: canhpn2
 */

#ifndef MESSAGEABS_MESSAGE_INC_INNERMESSAGE_H_
#define MESSAGEABS_MESSAGE_INC_INNERMESSAGE_H_
#include "stdint.h"
#include <cstring>
#include "codecmessage.h"

class AbstractMessage
{
public:
	enum class MessageId : int32_t
	{
		NONE = 0,
		CONFIG_SYSTEM_MESSAGE = 2,
		CONTROL_RELAY_MESAGE = 6,
		MONITOR_DATA_MESSAGE = 9,
		CONTROL_STATUS_DATA_MESSAGE = 10
	};
	AbstractMessage(MessageId id);
	virtual ~AbstractMessage();

	MessageId getMessageId() const;
	void setMessageId(MessageId mMessageId);
	virtual bool packData(CodecMessage *msg) = 0;
	virtual bool unpackData(CodecMessage *msg) = 0;

private:
	AbstractMessage() = delete;
protected:
	MessageId mMessageId { MessageId::NONE };
};

#endif /* SUBMODULE_VTJC2_FRAMEWORK_MESSAGEABS_MESSAGE_INC_INNERMESSAGE_H_ */
