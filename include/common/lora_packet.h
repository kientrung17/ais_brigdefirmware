#ifndef LORA_PACKET_H_
#define LORA_PACKET_H_

#include <stdint.h>

#define CMD_TYPE_TELEMETRY    0x00
#define CMD_TYPE_RELAY_CTRL   0x01
#define CMD_TYPE_ACK          0x02

#pragma pack(push, 1)
typedef struct {
    uint8_t zone_id;     // ID Khu vực
    uint8_t sender_id;   // ID Người gửi (0: Bridge, 1..N: Monitor)
    uint8_t receiver_id; // ID Người nhận (0: Bridge, 1..N: Monitor, 0xFF: Broadcast)
    uint8_t cmd_type;    // Loại lệnh (CMD_TYPE_...)
    
    union {
        // cmd_type == CMD_TYPE_TELEMETRY
        struct {
            int16_t ampe_x100;
            uint16_t oxy_x100;
            uint16_t ph_x100;
            int16_t temp_x100;
            uint16_t volt_x100;
            uint8_t phase_lost;
        } telemetry;

        // cmd_type == CMD_TYPE_RELAY_CTRL
        struct {
            uint8_t relay_channel; // 0..5
            uint8_t relay_state;   // 0: Tắt, 1: Bật
        } relay_ctrl;

        // cmd_type == CMD_TYPE_ACK
        struct {
            uint8_t status;        // 1: OK, 0: Fail
            uint8_t relay_channel;
            uint8_t relay_state;
        } ack;
    } data;
    
    uint16_t crc; // Checksum CRC16
} LoRaPacket;
#pragma pack(pop)

// Hàm tính CRC16 đơn giản
static inline uint16_t calculate_crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

#endif /* LORA_PACKET_H_ */
