#pragma once
#include "TaskAbstract.h"
#include <esp_now.h>
#include <esp_wifi.h>

#define ID_ESPNOW_SENDER_TASK 7

#pragma pack(push, 1)
struct EspNowMonitorPayload {
    uint32_t deviceId;       // ID thiết bị phát, ex: 101
    float motorTemp;         // Nhiệt độ vỏ động cơ
    float remoteVoltage;     // Điện áp pin/ắc quy
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
