#ifndef CONFIG_MANAGER___
#define CONFIG_MANAGER___
#include "message/controlrelaymessage.h"
#include "message/configsystemmessage.h"
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
    static constexpr const char *KEY_SYSTEM_CONFIG = "sys_config";
    static constexpr int MAX_SYSTEM_CONFIG_SIZE = 300;

    StoreFlashManager();
    ~StoreFlashManager();
    static StoreFlashManager *getInstance();

    // read relay infor from flash
    bool readRelayInforFromFlash(ControlRelayMessage *msg, uint8_t relayNum);
    bool saveRelayInforToFlash(ControlRelayMessage msg, uint8_t relayNum);

    // read/save system config from flash
    bool saveConfigToFlash(ConfigSystemMessage msg);
    bool readConfigFromFlash(ConfigSystemMessage *msg);

private:

};

#endif
