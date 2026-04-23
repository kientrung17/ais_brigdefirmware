
#ifndef BUSINESS_MESSAGE_MESSAGE_MESSAGE_SRC_CODECMESSAGE_H_
#define BUSINESS_MESSAGE_MESSAGE_MESSAGE_SRC_CODECMESSAGE_H_

#include <cstring>
#include <stdint.h>

class CodecMessage
{
public:
	CodecMessage();
	virtual ~CodecMessage();

	static const uint16_t MAX_DATA_LENGTH = 300;
	uint8_t mMsgID {0};
	uint16_t mMsgDataLength {0};
	uint8_t mDataRaw[MAX_DATA_LENGTH] {0};
};

#endif /* BUSINESS_MESSAGE_VTJC2_MESSAGE_MESSAGE_SRC_CODECMESSAGE_H_ */
