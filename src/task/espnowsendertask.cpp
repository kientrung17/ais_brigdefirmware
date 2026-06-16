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
    
    // Quét tìm kênh WiFi của SoftAP "Aqua_Control" của mạch ControlHub để tự động đồng bộ kênh
    uint8_t target_channel = 1; // Mặc định là kênh 1
    wifi_scan_config_t scan_config = {};
    scan_config.ssid = (uint8_t*)"Aqua_Control";
    scan_config.show_hidden = true;
    
    LOG_INFO("EspNowSenderTask", "Scanning for ControlHub SoftAP 'Aqua_Control' to sync channel...");
    // Gọi quét đồng bộ (blocking scan)
    if (esp_wifi_scan_start(&scan_config, true) == ESP_OK) {
        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);
        if (ap_count > 0) {
            wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
            if (ap_records) {
                if (esp_wifi_scan_get_ap_records(&ap_count, ap_records) == ESP_OK) {
                    for (int i = 0; i < ap_count; i++) {
                        if (strcmp((char*)ap_records[i].ssid, "Aqua_Control") == 0) {
                            target_channel = ap_records[i].primary;
                            LOG_INFO("EspNowSenderTask", "Found 'Aqua_Control' SoftAP on Channel %d. Syncing...", target_channel);
                            break;
                        }
                    }
                }
                free(ap_records);
            }
        } else {
            LOG_INFO("EspNowSenderTask", "'Aqua_Control' SoftAP not found. Defaulting to Channel 1.");
        }
    } else {
        LOG_ERROR("EspNowSenderTask", "WiFi Scan failed. Defaulting to Channel 1.");
    }

    ESP_ERROR_CHECK(esp_wifi_set_channel(target_channel, WIFI_SECOND_CHAN_NONE));

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
    // Không gửi dữ liệu giả lập nữa, chỉ chuyển tiếp dữ liệu thật từ LoRaBridgeTask
}
