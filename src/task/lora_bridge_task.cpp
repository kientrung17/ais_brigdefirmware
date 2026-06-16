#include "task/lora_bridge_task.h"
#include "common/common.h"
#include "common/lora_packet.h"
#include "lora_config.h"
#include "logger/loggermanager.h"
#include "task/espnowsendertask.h"
#include <string.h>

static const char *TAG = "LoraBridgeTask";

LoraBridgeTask::LoraBridgeTask(const std::string& taskName, uint8_t maxElementQueueSet)
    : TaskAbstract(taskName, maxElementQueueSet)
{
}

LoraBridgeTask::~LoraBridgeTask()
{
}

void LoraBridgeTask::onInitProcess()
{
    LOG_INFO(TAG, "Initializing LoRa Bridge on SPI3 (VSPI)...");

    esp_err_t ret = lora.init(
        PIN_LORA_MOSI,
        PIN_LORA_MISO,
        PIN_LORA_SCK,
        PIN_LORA_NSS,
        PIN_LORA_RST,
        PIN_LORA_IRQ
    );

    if (ret == ESP_OK) {
        lora.setFrequency(LORA_FREQUENCY);
        lora.setSyncWord(LORA_SYNC_WORD);
        lora.startReceive();
        LOG_INFO(TAG, "LoRa Bridge initialized successfully. Frequency: %.1f MHz, Sync Word: 0x%02X", LORA_FREQUENCY, LORA_SYNC_WORD);
    } else {
        LOG_ERROR(TAG, "LoRa Bridge initialization failed!");
    }
}

void LoraBridgeTask::onTimer100HzProcess()
{
    // Poll nhận tin qua LoRa liên tục (gọi tại 100Hz = 10ms/lần)
    processLoraRx();
}

void LoraBridgeTask::processLoraRx()
{
    uint8_t buf[255];
    uint8_t len = lora.receivePacket(buf, sizeof(buf));

    if (len == sizeof(LoRaPacket)) {
        LoRaPacket *packet = (LoRaPacket*)buf;

        // 1. Kiểm tra CRC16
        uint16_t calc_crc = calculate_crc16(buf, sizeof(LoRaPacket) - sizeof(uint16_t));
        if (calc_crc != packet->crc) {
            LOG_ERROR(TAG, "Received LoRa packet with CRC mismatch!");
            return;
        }

        // 2. Lọc Zone ID (Chống nhận nhầm từ khu vực lân cận)
        if (packet->zone_id != LORA_ZONE_ID) {
            return;
        }

        // 3. Xử lý các loại gói tin nhận được từ Monitor
        if (packet->cmd_type == CMD_TYPE_TELEMETRY) {
            LOG_INFO(TAG, "Received Telemetry from Monitor ID: %d", packet->sender_id);

            // Chuyển đổi dữ liệu sang struct ESP-NOW
            EspNowMonitorPayload payload;
            payload.deviceId = packet->sender_id;
            payload.ampeChannel1 = packet->data.telemetry.ampe_x100 / 100.0f;
            payload.oxy = packet->data.telemetry.oxy_x100 / 100.0f;
            payload.pH = packet->data.telemetry.ph_x100 / 100.0f;
            payload.temperature = packet->data.telemetry.temp_x100 / 100.0f;
            payload.voltage = packet->data.telemetry.volt_x100 / 100.0f;
            payload.isPowerLostPhare = packet->data.telemetry.phase_lost;

            // Phát broadcast qua ESP-NOW về cho ControlHub
            esp_err_t result = esp_now_send(mBroadcastAddress, (uint8_t *)&payload, sizeof(payload));
            if (result == ESP_OK) {
                LOG_INFO(TAG, "Forwarded telemetry to ControlHub via ESP-NOW: Monitor ID %d", payload.deviceId);
            } else {
                LOG_ERROR(TAG, "Failed to send ESP-NOW packet: 0x%x", result);
            }
        } 
        else if (packet->cmd_type == CMD_TYPE_ACK) {
            LOG_INFO(TAG, "Received ACK from Monitor ID %d: Channel %d status %d", 
                     packet->sender_id, packet->data.ack.relay_channel, packet->data.ack.status);
            
            // Xử lý hoặc chuyển tiếp ACK này về cho ControlHub nếu cần thiết
        }
    }
}
