#include "task/espnowsendertask.h"
#include "logger/loggermanager.h"
#include <string.h>
#include <esp_random.h>

EspNowSenderTask::EspNowSenderTask(const std::string& taskName, uint8_t maxElementQueueSet) 
    : TaskAbstract(taskName, maxElementQueueSet)
{
}

EspNowSenderTask::~EspNowSenderTask()
{
}

void EspNowSenderTask::onInitProcess()
{
    // Cấu hình Wi-Fi ở chế độ STA để dùng được ESP-NOW
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Khởi tạo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        LOG_ERROR("EspNowSenderTask", "Error initializing ESP-NOW");
        return;
    }

    // Đăng ký mạch đích là Broadcast (nhận mù)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mBroadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        LOG_ERROR("EspNowSenderTask", "Failed to add broadcast peer");
        return;
    }

    LOG_INFO("EspNowSenderTask", "ESP-NOW Sender Init Success. Broadcasting...");
}

void EspNowSenderTask::onTimer100HzProcess()
{
    static int tick = 0;
    tick++;
    if (tick >= 100) { // Mỗi 1 giây phát 1 lần
        tick = 0;

        EspNowMonitorPayload payload;
        payload.deviceId = 101;
        
        // Giả lập nhiệt độ từ 30.00 đến 40.00
        payload.motorTemp = 30.0f + (esp_random() % 1000) / 100.0f;
        // Giả lập điện áp từ 11.50 đến 13.00
        payload.remoteVoltage = 11.5f + (esp_random() % 150) / 100.0f;

        esp_err_t result = esp_now_send(mBroadcastAddress, (uint8_t *) &payload, sizeof(payload));
        
        if (result == ESP_OK) {
            LOG_INFO("EspNowSenderTask", "Sent Mock Data - Temp: %.2f, Volt: %.2f", payload.motorTemp, payload.remoteVoltage);
        } else {
            LOG_ERROR("EspNowSenderTask", "Error sending data");
        }
    }
}
