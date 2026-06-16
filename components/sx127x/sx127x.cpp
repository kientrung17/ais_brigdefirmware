#include "sx127x.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static const char *TAG = "SX127x";

// Register Definitions
#define REG_FIFO                    0x00
#define REG_OP_MODE                 0x01
#define REG_FRF_MSB                 0x06
#define REG_FRF_MID                 0x07
#define REG_FRF_LSB                 0x08
#define REG_PA_CONFIG               0x09
#define REG_FIFO_ADDR_PTR           0x0d
#define REG_FIFO_TX_BASE_ADDR       0x0e
#define REG_FIFO_RX_BASE_ADDR       0x0f
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_MODEM_CONFIG_1          0x1d
#define REG_MODEM_CONFIG_2          0x1e
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_DETECTION_OPTIMIZE      0x31
#define REG_DETECTION_THRESHOLD     0x37
#define REG_SYNC_WORD               0x39

// Modes
#define MODE_LORA                   0x80
#define MODE_SLEEP                  0x00
#define MODE_STANDBY                0x01
#define MODE_TX                     0x03
#define MODE_RX_CONTINUOUS          0x05

// IRQ flags
#define IRQ_TX_DONE_MASK            0x08
#define IRQ_RX_DONE_MASK            0x40
#define IRQ_PAYLOAD_CRC_ERROR_MASK  0x20

SX127x::SX127x() : spi(NULL), pinCs(-1), pinRst(-1), pinIrq(-1) {}

SX127x::~SX127x() {
    if (spi) {
        spi_bus_remove_device(spi);
    }
}

uint8_t SX127x::readRegister(uint8_t address) {
    uint8_t tx_data[2] = { (uint8_t)(address & 0x7F), 0x00 };
    uint8_t rx_data[2] = { 0 };

    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;

    if (spi_device_transmit(spi, &t) != ESP_OK) {
        ESP_LOGE(TAG, "Read Register SPI transmit failed");
    }
    return rx_data[1];
}

void SX127x::writeRegister(uint8_t address, uint8_t value) {
    uint8_t tx_data[2] = { (uint8_t)(address | 0x80), value };

    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = tx_data;

    if (spi_device_transmit(spi, &t) != ESP_OK) {
        ESP_LOGE(TAG, "Write Register SPI transmit failed");
    }
}

void SX127x::writeFifo(uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        writeRegister(REG_FIFO, data[i]);
    }
}

void SX127x::readFifo(uint8_t *buf, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        buf[i] = readRegister(REG_FIFO);
    }
}

void SX127x::reset() {
    gpio_set_level((gpio_num_t)pinRst, 0);
    ets_delay_us(1000); // 1ms
    gpio_set_level((gpio_num_t)pinRst, 1);
    vTaskDelay(pdMS_TO_TICKS(10)); // 10ms
}

void SX127x::setOpMode(uint8_t mode) {
    writeRegister(REG_OP_MODE, MODE_LORA | mode);
}

esp_err_t SX127x::init(int mosi, int miso, int sck, int cs, int rst, int irq) {
    pinCs = cs;
    pinRst = rst;
    pinIrq = irq;

    // Config GPIOs
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << rst);
    gpio_config(&io_conf);

    if (irq >= 0) {
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << irq);
        gpio_config(&io_conf);
    }

    // Config SPI Bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = mosi;
    buscfg.miso_io_num = miso;
    buscfg.sclk_io_num = sck;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;

    // Khởi tạo SPI bus (cho phép bỏ qua lỗi nếu bus đã được init trước đó)
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI Bus init failed!");
        return ret;
    }

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 8 * 1000 * 1000; // 8 MHz
    devcfg.mode = 0;
    devcfg.spics_io_num = cs;
    devcfg.queue_size = 7;

    ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device!");
        return ret;
    }

    // Reset module cứng
    reset();

    // Đọc test register check sự hiện diện của chip (REG_OP_MODE mặc định sau reset là 0x01 hoặc 0x09)
    uint8_t version = readRegister(0x42); // RegVersion
    ESP_LOGI(TAG, "SX127x RegVersion (0x42) = 0x%02X (Expected: 0x12)", version);

    // Chuyển sang LoRa sleep để ghi các thanh ghi cấu hình LoRa
    setOpMode(MODE_SLEEP);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Cấu hình cơ bản LoRa (Spreading Factor = 9, Bandwidth = 125kHz, Coding Rate = 4/5)
    writeRegister(REG_MODEM_CONFIG_1, 0x72); // BW = 125kHz, CR = 4/5, Explicit Header
    writeRegister(REG_MODEM_CONFIG_2, 0x94); // SF = 9, CRC Enable
    
    // Đặt dòng cấp PA tối đa (Power)
    writeRegister(REG_PA_CONFIG, 0x8F); // PA_BOOST, Max power = 17dBm
    
    // Cấu hình Preamble mặc định (8 kí tự)
    writeRegister(REG_PREAMBLE_MSB, 0x00);
    writeRegister(REG_PREAMBLE_LSB, 0x08);

    // Chuyển sang Standby
    setOpMode(MODE_STANDBY);

    return ESP_OK;
}

void SX127x::setFrequency(float mhz) {
    uint32_t frf = (uint32_t)((mhz * 1000000.0) / (32000000.0 / 524288.0));
    writeRegister(REG_FRF_MSB, (frf >> 16) & 0xFF);
    writeRegister(REG_FRF_MID, (frf >> 8) & 0xFF);
    writeRegister(REG_FRF_LSB, frf & 0xFF);
    ESP_LOGI(TAG, "Frequency set to %.1f MHz (FRF: 0x%06X)", mhz, (unsigned int)frf);
}

void SX127x::setSyncWord(uint8_t syncWord) {
    writeRegister(REG_SYNC_WORD, syncWord);
    ESP_LOGI(TAG, "Sync Word set to 0x%02X", syncWord);
}

bool SX127x::sendPacket(uint8_t *data, uint8_t length) {
    // Chuyển sang Standby để load FIFO
    setOpMode(MODE_STANDBY);

    // Reset con trỏ FIFO về vùng TX base
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_TX_BASE_ADDR));
    
    // Ghi độ dài payload
    writeRegister(REG_PAYLOAD_LENGTH, length);
    
    // Ghi payload vào FIFO
    writeFifo(data, length);

    // Chuyển sang TX mode
    setOpMode(MODE_TX);

    // Chờ phát xong (tx done) tối đa 1000ms
    int timeout = 100; // 100 * 10ms = 1s
    while (timeout > 0) {
        uint8_t irq = readRegister(REG_IRQ_FLAGS);
        if (irq & IRQ_TX_DONE_MASK) {
            // Clear TX_DONE
            writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        timeout--;
    }
    ESP_LOGE(TAG, "Send packet timeout!");
    return false;
}

void SX127x::startReceive() {
    setOpMode(MODE_STANDBY);
    // Cài RX base addr
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_BASE_ADDR));
    // Sang RX continuous
    setOpMode(MODE_RX_CONTINUOUS);
}

uint8_t SX127x::receivePacket(uint8_t *buf, uint8_t maxLen) {
    uint8_t irq = readRegister(REG_IRQ_FLAGS);
    
    // Nếu có cờ RX DONE
    if (irq & IRQ_RX_DONE_MASK) {
        // Kiểm tra lỗi CRC
        if (irq & IRQ_PAYLOAD_CRC_ERROR_MASK) {
            ESP_LOGW(TAG, "Payload CRC error!");
            writeRegister(REG_IRQ_FLAGS, irq); // clear all
            return 0;
        }

        // Đọc số byte nhận được
        uint8_t bytes = readRegister(REG_RX_NB_BYTES);
        uint8_t len = (bytes > maxLen) ? maxLen : bytes;

        // Trỏ FIFO về địa chỉ bắt đầu của gói tin vừa nhận
        writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));
        
        // Đọc FIFO
        readFifo(buf, len);

        // Clear cờ IRQ
        writeRegister(REG_IRQ_FLAGS, irq);
        
        return len;
    }
    return 0;
}
