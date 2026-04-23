#ifndef CONFIG_MANAGER___
#define CONFIG_MANAGER___
#include "message/controlrelaymessage.h"
#include "message/messagecommon.h"

class StoreFlashManager
{
private:
    /* data */
public:
    static constexpr const char *KEY_CONTROL_RELAY[MessageCommon::MAX_NUM_RELAY] = {
        "rel_ctrl_1",
        "rel_ctrl_2",
        "rel_ctrl_3",
        "rel_ctrl_4",
        "rel_ctrl_5",
        "rel_ctrl_6"};
    static constexpr int MAX_CONTROL_RELAY_INFOR = 50;
    StoreFlashManager();
    ~StoreFlashManager();
    static StoreFlashManager *getInstance();

    // read relay infor from flash
    bool readRelayInforFromFlash(ControlRelayMessage *msg, uint8_t relayNum);
    bool saveRelayInforToFlash(ControlRelayMessage msg, uint8_t relayNum);

private:
    static StoreFlashManager *mInstance;

};

#endif
