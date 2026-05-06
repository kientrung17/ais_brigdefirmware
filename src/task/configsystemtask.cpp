#include "task/configsystemtask.h"
#include "HAL/HAL_ESP32/udpesp32.h"
#include "common/common.h"
#include "common/storeflashmanager.h"
#include "loggermanager.h"
#include "message/configsystemmessage.h"
#include "esp_system.h"

ConfigSystemTask::ConfigSystemTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
}

ConfigSystemTask::~ConfigSystemTask()
{
    if (mUdp) {
        delete mUdp;
    }
}

void ConfigSystemTask::onInitProcess()
{
    LOG_INFO("ConfigSystemTask", "Initializing ConfigSystemTask on UDP port %d", UDP_LOCAL_PORT_SYSTEM_CONFIG);
    mUdp = new UdpEsp32(UDP_LOCAL_PORT_SYSTEM_CONFIG);
    mUdp->init();
}

void ConfigSystemTask::onTimer100HzProcess()
{
    mCounter100Hz++;
    processReadDataFromUDP();
}

void ConfigSystemTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    // Không có hàng đợi nào đăng ký vào tác vụ này
}

void ConfigSystemTask::processReadDataFromUDP()
{
    if (mUdp && mUdp->checkAvailable()) {
        uint8_t rawBuffer[384];
        int length = mUdp->read(rawBuffer, sizeof(rawBuffer));
        if (length > 0) {
            CodecMessage codecMsg;
            if (length >= 3) {
                codecMsg.mMsgID = rawBuffer[0];
                codecMsg.mMsgDataLength = (rawBuffer[1] << 8) | rawBuffer[2];
                if (codecMsg.mMsgDataLength <= CodecMessage::MAX_DATA_LENGTH && 
                    codecMsg.mMsgDataLength <= (length - 3)) {
                    memcpy(codecMsg.mDataRaw, &rawBuffer[3], codecMsg.mMsgDataLength);
                    
                    mUdp->updateRemoteFromRemoteOfDataReceive();

                    switch (codecMsg.mMsgID) {
                        case 1: // Bản tin Ping (Discovery)
                            processorPingMessage();
                            break;
                        case 2: // Bản tin ghi cấu hình mới CONFIG_SYSTEM_MESSAGE
                            processorConfigSystemMessage(&codecMsg);
                            break;
                        case 3: // Bản tin yêu cầu đọc cấu hình (READ_CONFIG_SYSTEM)
                            processorCommandSystemMessage(&codecMsg);
                            break;
                        default:
                            LOG_ERROR("ConfigSystemTask", "Unknown Message ID received: %d", codecMsg.mMsgID);
                            break;
                    }
                } else {
                    LOG_ERROR("ConfigSystemTask", "Message data length mismatch: expected %d, got %d", codecMsg.mMsgDataLength, length - 3);
                }
            }
        }
    }
}

void ConfigSystemTask::processorPingMessage()
{
    LOG_INFO("ConfigSystemTask", "Received Ping message from App Center");
    sendPingResponseToAppCenter();
}

void ConfigSystemTask::sendPingResponseToAppCenter()
{
    CodecMessage respMsg;
    respMsg.mMsgID = 1; // Ping response Msg ID
    respMsg.mMsgDataLength = 9; // 8 bytes device_id + 1 byte device_type
    
    memcpy(respMsg.mDataRaw, &gDeviceID, sizeof(gDeviceID));
    respMsg.mDataRaw[8] = MessageCommon::AquaControlDevice;
    
    uint8_t sendBuffer[12];
    sendBuffer[0] = respMsg.mMsgID;
    sendBuffer[1] = (respMsg.mMsgDataLength >> 8) & 0xFF;
    sendBuffer[2] = respMsg.mMsgDataLength & 0xFF;
    memcpy(&sendBuffer[3], respMsg.mDataRaw, respMsg.mMsgDataLength);
    
    mUdp->send(sendBuffer, respMsg.mMsgDataLength + 3);
    LOG_INFO("ConfigSystemTask", "Sent Ping Response to App Center");
}

void ConfigSystemTask::processorCommandSystemMessage(CodecMessage *msg)
{
    LOG_INFO("ConfigSystemTask", "Received Command: READ_CONFIG_SYSTEM");
    
    ConfigSystemMessage configMsg(gConfigSystem.getConfigSystem());
    CodecMessage respMsg;
    if (configMsg.packData(&respMsg)) {
        uint8_t sendBuffer[CodecMessage::MAX_DATA_LENGTH + 3];
        sendBuffer[0] = respMsg.mMsgID; // 2 (CONFIG_SYSTEM_MESSAGE)
        sendBuffer[1] = (respMsg.mMsgDataLength >> 8) & 0xFF;
        sendBuffer[2] = respMsg.mMsgDataLength & 0xFF;
        memcpy(&sendBuffer[3], respMsg.mDataRaw, respMsg.mMsgDataLength);
        
        mUdp->send(sendBuffer, respMsg.mMsgDataLength + 3);
        LOG_INFO("ConfigSystemTask", "Sent ConfigSystemMessage response to App Center");
    } else {
        LOG_ERROR("ConfigSystemTask", "Failed to pack ConfigSystemMessage");
    }
}

void ConfigSystemTask::processorConfigSystemMessage(CodecMessage *msg)
{
    LOG_INFO("ConfigSystemTask", "Received CONFIG_SYSTEM_MESSAGE to write new config");
    ConfigSystemMessage newConfigMsg;
    if (newConfigMsg.unpackData(msg)) {
        // Cập nhật cấu hình toàn cục
        gConfigSystem.setConfigSyste(newConfigMsg.getConfigSystem());
        
        // Lưu cấu hình vào Flash NVS thông qua StoreFlashManager
        bool success = StoreFlashManager::getInstance()->saveConfigToFlash(gConfigSystem);
        if (success) {
            LOG_INFO("ConfigSystemTask", "Config saved to flash successfully!");
            
            // Phản hồi xác nhận thành công về cho App bằng bản tin ping response
            sendPingResponseToAppCenter();
            
            // Trì hoãn 500ms để đảm bảo gói tin UDP phản hồi đã được truyền đi
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // Restart hệ thống để áp dụng cấu hình Wifi/MQTT mới
            LOG_INFO("ConfigSystemTask", "Restarting device to apply new configuration...");
            esp_restart();
        } else {
            LOG_ERROR("ConfigSystemTask", "Failed to save configuration to flash!");
        }
    } else {
        LOG_ERROR("ConfigSystemTask", "Failed to unpack CONFIG_SYSTEM_MESSAGE");
    }
}
