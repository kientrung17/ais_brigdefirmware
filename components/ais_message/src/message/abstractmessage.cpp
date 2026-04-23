/*
 * innermessage.cpp
 *
 *  Created on: May 17, 2024
 *      Author: canhpn2
 */

#include "abstractmessage.h"

AbstractMessage::AbstractMessage(MessageId id) : mMessageId(id)
{

}

AbstractMessage::~AbstractMessage()
{

}

AbstractMessage::MessageId AbstractMessage::getMessageId() const
{
	return mMessageId;
}

void AbstractMessage::setMessageId(MessageId messageId)
{
	mMessageId = messageId;
}
