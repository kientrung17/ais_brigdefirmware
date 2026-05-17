#ifndef ESPNOWRECEIVERTASK_H
#define ESPNOWRECEIVERTASK_H

#include "taskabstract.h"
#include "esp_now.h"

// =========================================================
// Struct dữ liệu ESP-NOW gửi từ mạch Monitor sang mạch Control
// Mạch Monitor phải đóng gói đúng theo struct này trước khi gửi
// =========================================================
struct EspNowMonitorPayload {
    uint32_t deviceId;         // ID định danh mạch Monitor (ví dụ: 101)
    float    motorTemp;        // Nhiệt độ vỏ động cơ (°C)
    float    remoteVoltage;    // Điện áp đo được tại mạch Monitor (V)
};

class EspNowReceiverTask : public TaskAbstract
{
public:
    static constexpr uint32_t MONITOR_DEVICE_ID = 101; // Hardcode ID mạch Con

    EspNowReceiverTask(std::string nameTask, int numElementQueueSet);
    ~EspNowReceiverTask();

    virtual void onInitProcess() override;
    virtual void onTimer100HzProcess() override;
    virtual void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;

private:
    // Hàm callback tĩnh nhận gói tin ESP-NOW (bắt buộc phải là static)
    static void onDataReceive(const esp_now_recv_info_t *recvInfo,
                              const uint8_t *data,
                              int len);
};

#endif // ESPNOWRECEIVERTASK_H
