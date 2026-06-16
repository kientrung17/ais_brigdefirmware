#ifndef SX127X_H_
#define SX127X_H_

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "stdint.h"

class SX127x {
public:
    SX127x();
    ~SX127x();

    // Khởi tạo SPI và cấu hình LoRa
    esp_err_t init(int mosi, int miso, int sck, int cs, int rst, int irq);

    // Cấu hình tần số (MHz)
    void setFrequency(float mhz);

    // Cấu hình Sync Word
    void setSyncWord(uint8_t syncWord);

    // Phát gói tin (blocking/non-blocking tùy ý, ở đây chờ TxDone hoặc timeout)
    bool sendPacket(uint8_t *data, uint8_t length);

    // Bắt đầu chế độ nhận liên tục (RX Continuous)
    void startReceive();

    // Nhận gói tin nếu có sẵn (non-blocking poll)
    // Trả về độ dài dữ liệu nhận được (0 nếu không có)
    uint8_t receivePacket(uint8_t *buf, uint8_t maxLen);

private:
    spi_device_handle_t spi;
    int pinCs;
    int pinRst;
    int pinIrq;

    // Đọc/Ghi register qua SPI
    uint8_t readRegister(uint8_t address);
    void writeRegister(uint8_t address, uint8_t value);
    void writeFifo(uint8_t *data, uint8_t length);
    void readFifo(uint8_t *buf, uint8_t length);

    // Quản lý Reset cứng
    void reset();

    // Các hàm phụ trợ chuyển đổi chế độ
    void setOpMode(uint8_t mode);
};

#endif /* SX127X_H_ */
