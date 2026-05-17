#include "task/espnowreceivertask.h"
#include "common/common.h"
#include "loggermanager.h"
#include "esp_wifi.h"
#include "esp_now.h"

EspNowReceiverTask::EspNowReceiverTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
}

EspNowReceiverTask::~EspNowReceiverTask()
{
    esp_now_deinit();
}

// =========================================================
// onInitProcess: Khởi tạo ESP-NOW và đăng ký callback
// Được gọi một lần duy nhất khi task bắt đầu chạy
// =========================================================
void EspNowReceiverTask::onInitProcess()
{
    LOG_INFO("EspNowReceiverTask", "Initializing ESP-NOW receiver...");

    // Bước 1: ESP-NOW bắt buộc phải chạy sau khi WiFi đã được khởi tạo
    // WifiManagerTask đã khởi tạo WiFi stack trước đó => gọi trực tiếp
    esp_err_t err = esp_now_init();
    if (err != ESP_OK) {
        LOG_ERROR("EspNowReceiverTask", "esp_now_init() failed: 0x%x", err);
        return;
    }

    // Bước 2: Đăng ký hàm callback nhận gói tin
    // Callback sẽ được gọi từ Wi-Fi task nội bộ của ESP-IDF (ISR-like context)
    // => Tuyệt đối không allocate heap, không delay, không dùng Queue blocking
    err = esp_now_register_recv_cb(EspNowReceiverTask::onDataReceive);
    if (err != ESP_OK) {
        LOG_ERROR("EspNowReceiverTask", "esp_now_register_recv_cb() failed: 0x%x", err);
        return;
    }

    LOG_INFO("EspNowReceiverTask", "ESP-NOW receiver ready. Listening for deviceId=%d",
             MONITOR_DEVICE_ID);
}

// =========================================================
// onTimer100HzProcess: Task này không cần timer loop
// Toàn bộ logic xử lý nằm trong callback onDataReceive
// =========================================================
void EspNowReceiverTask::onTimer100HzProcess()
{
    // ESP-NOW dùng callback, không cần polling trong vòng lặp timer
}

// =========================================================
// onQueueSetMessageProcess: Task này không đăng ký Queue nào
// =========================================================
void EspNowReceiverTask::onQueueSetMessageProcess(OSBase::QueueHandle /*queue_sem*/)
{
    // Không có Queue nào được đăng ký
}

// =========================================================
// onDataReceive [STATIC CALLBACK]
//
// Ràng buộc kỹ thuật (QUAN TRỌNG):
//   - Hàm này chạy trong Wi-Fi interrupt context của ESP-IDF
//   - TUYỆT ĐỐI KHÔNG dùng: Queue, vTaskDelay, malloc, new
//   - CHỈ được dùng: std::atomic::store() (Lock-free, ISR-safe)
// =========================================================
void EspNowReceiverTask::onDataReceive(const esp_now_recv_info_t *recvInfo,
                                       const uint8_t *data,
                                       int len)
{
    // Kiểm tra kích thước gói tin hợp lệ
    if (len != sizeof(EspNowMonitorPayload)) {
        LOG_ERROR("EspNowReceiverTask", "Invalid payload size: got %d, expected %d",
                 len, (int)sizeof(EspNowMonitorPayload));
        return;
    }

    // Ép kiểu an toàn từ raw bytes sang struct
    EspNowMonitorPayload payload;
    memcpy(&payload, data, sizeof(EspNowMonitorPayload));

    // Lọc định danh: chỉ nhận dữ liệu từ mạch Monitor đã đăng ký
    if (payload.deviceId != MONITOR_DEVICE_ID) {
        LOG_DEBUG("EspNowReceiverTask", "Ignored packet from unknown deviceId=%d",
                  payload.deviceId);
        return;
    }

    // Ghi thẳng vào SharedDataStore (Lock-free, ISR-safe, zero latency)
    gSharedData.motor_temp.store(payload.motorTemp, std::memory_order_relaxed);
    gSharedData.remote_voltage.store(payload.remoteVoltage, std::memory_order_relaxed);

    LOG_INFO("EspNowReceiverTask", "OK | deviceId=%d | Temp=%.2f°C | Volt=%.2fV",
              payload.deviceId, payload.motorTemp, payload.remoteVoltage);
}
