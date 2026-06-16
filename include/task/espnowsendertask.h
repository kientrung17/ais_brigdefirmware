#pragma once
#include "TaskAbstract.h"
#include <esp_now.h>
#include <esp_wifi.h>

#define ID_ESPNOW_SENDER_TASK 7

#pragma pack(push, 1)
struct EspNowMonitorPayload {
    uint32_t deviceId;       // ID thiết bị phát, ex: 1, 2, 3...
    float ampeChannel1;      // Dòng điện Kênh 1
    float oxy;               // Nồng độ Oxy hòa tan
    float pH;                // Độ pH nước
    float voltage;           // Điện áp pin/ắc quy
    float temperature;       // Nhiệt độ vỏ động cơ
    uint32_t isPowerLostPhare; // Mất pha mạch Monitor (Bitmask từ 0 đến 7)
};
#pragma pack(pop)

class EspNowSenderTask : public TaskAbstract
{
private:
    uint8_t mBroadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

public:
    EspNowSenderTask(const std::string& taskName, uint8_t maxElementQueueSet);
    ~EspNowSenderTask();

protected:
    void onInitProcess() override;
    void onTimer100HzProcess() override;
    void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override {}
};
