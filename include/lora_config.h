#ifndef LORA_CONFIG_H_
#define LORA_CONFIG_H_

// === Cấu hình phân khu cứng (Hardcoded) ===
#define LORA_FREQUENCY      433.0f   // MHz (Khu 1: 433.0f, Khu 2: 433.5f, Khu 3: 434.0f)
#define LORA_SYNC_WORD      0x54     // Sync Word (Khu 1: 0x54, Khu 2: 0x67, Khu 3: 0x80)
#define LORA_ZONE_ID        0x01     // ID Khu vực (Khu 1: 1, Khu 2: 2, Khu 3: 3)

// === Cấu hình ID thiết bị ===
#define LORA_MONITOR_ID     2        // ID của Monitor này (ví dụ: Monitor số 2)
#define LORA_BRIDGE_ID      0        // ID của Bridge (mặc định = 0)

// === Cấu hình GPIO cho LoRa SX1278 (SPI) ===
#define PIN_LORA_SCK        18
#define PIN_LORA_MISO       19
#define PIN_LORA_MOSI       23
#define PIN_LORA_NSS        13
#define PIN_LORA_RST        14
#define PIN_LORA_IRQ        27

#endif /* LORA_CONFIG_H_ */
