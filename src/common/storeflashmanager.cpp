#include "common/storeflashmanager.h"
#include "loggermanager.h"
#include "common/common.h"

StoreFlashManager *StoreFlashManager::mInstance = nullptr;

StoreFlashManager::StoreFlashManager() = default;

StoreFlashManager::~StoreFlashManager() = default;

StoreFlashManager *StoreFlashManager::getInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new StoreFlashManager();
    }
    return mInstance;
}
bool StoreFlashManager::readRelayInforFromFlash(ControlRelayMessage *msg, uint8_t relayNum)
{
    if (relayNum >= MessageCommon::MAX_NUM_RELAY || msg == nullptr)
    {
        LOG_ERROR("[StoreFlashManager]", "readRelayInforFromFlash but index(%d) > max: %d or ControlRelayMessage null", relayNum, (int)MessageCommon::MAX_NUM_RELAY);
        return false;
    }
    size_t lengthDataReaded = 0;
    if (gFlashManager == nullptr)
    {
        LOG_ERROR("StoreFlashManager", "why gFlashManager null?");
        return false;
    }
    CodecMessage msgCodec;
    bool ret = gFlashManager->readBlob(KEY_CONTROL_RELAY[relayNum], msgCodec.mDataRaw, MAX_CONTROL_RELAY_INFOR, lengthDataReaded);
    if (ret == false)
    {
        LOG_ERROR("StoreFlashManager", "Ready control relay from nvs faild");
        return false;
    }
    else
    {
        msgCodec.mMsgDataLength = lengthDataReaded;
    }
    if (msg->unpackData(&msgCodec))
    {
        LOG_INFO("StoreFlashManager", "Read config relay control infor: %d success", relayNum);
        return true;
    }
    else
    {
        LOG_ERROR("StoreFlashManager", "Read config relay control infor: %d faild", relayNum);
        return false;
    }
    return false;
}

bool StoreFlashManager::saveRelayInforToFlash(ControlRelayMessage msg, uint8_t relayNum)
{
    if (relayNum >= MessageCommon::MAX_NUM_RELAY)
    {
        LOG_ERROR("[StoreFlashManager]", "saveRelayInforToFlash but index(%d) > max: %d or ControlRelayMessage null", relayNum, (int)MessageCommon::MAX_NUM_RELAY);
        return false;
    }
    CodecMessage codecMsg;
    auto retPack = msg.packData(&codecMsg);
    if (!retPack)
    {
        LOG_ERROR("StoreFlashManager", "ConfigSystemMessage pack data error");
        return false;
    }
    uint16_t msgLength = codecMsg.mMsgDataLength;
    if (msgLength > 0)
    {
        bool ret = gFlashManager->writeBlob(KEY_CONTROL_RELAY[relayNum], codecMsg.mDataRaw, msgLength);
        if (ret == false)
        {
            LOG_ERROR("StoreFlashManager", "saveRelayInforToFlash write data system from nvs faild");
            return false;
        }
        else
        {
            LOG_INFO("StoreFlashManager", "saveRelayInforToFlash write data system from nvs success");
        }
    }
    else
    {
        LOG_ERROR("StoreFlashManager", "saveRelayInforToFlash encode error");
        return false;
    }
    return true;
}

