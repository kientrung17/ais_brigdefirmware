/*
 * esp32spi.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ESP32_INC_ESP32SPI_H_
#define HAL_ESP32_INC_ESP32SPI_H_

#include "halspiabstract.h"

class EspidfSpiHal : public HalSpiAbstract
{
public:
    // Hide ESP-IDF host type behind our own enum
    enum class Host
    {
        SPI2, // HSPI on classic ESP32
        SPI3  // VSPI on classic ESP32
    };
    enum class TransferMode
    {
        Polling,
        QueuedDMA
    };

    EspidfSpiHal();
    ~EspidfSpiHal();

    // Initialize bus + device
    // mode: SPI mode 0..3; clock_hz: e.g., 10*1000*1000; queue_size used when QueuedDMA
    bool Init(Host host,
              int mosi, int miso, int sclk, int cs,
              int clock_hz,
              int mode = 0,
              TransferMode xferMode = TransferMode::Polling,
              int queue_size = 3,
              int dma_channel = -1 /* -1:auto, 0:disabled, 1/2:channel */);

    // HalSpiAbstract overrides
    bool Hal_Spi_Transmit(uint8_t *data, uint16_t size, uint32_t timeout) override;
    bool Hal_Spi_Receive(uint8_t *data, uint16_t size, uint32_t timeout) override;
    bool Hal_Spi_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint16_t Timeout) override;

private:
    bool doTransfer(uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout_ms);

private:
    Host mHost{Host::SPI2};
    void *mDev{nullptr}; // opaque to avoid exposing ESP-IDF types in header
    int mClockHz{10 * 1000 * 1000};
    int mMode{0};
    int mQueueSize{3};
    TransferMode mTransferMode{TransferMode::Polling};
    int mPinMosi{-1}, mPinMiso{-1}, mPinSclk{-1}, mPinCs{-1};
    bool mInited{false};
};

#endif /* HAL_ESP32_INC_ESP32SPI_H_ */
