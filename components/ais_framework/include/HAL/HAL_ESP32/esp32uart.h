/*
 * esp32uart.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ESP32_INC_ESP32UART_H_
#define HAL_ESP32_INC_ESP32UART_H_

#include "haluartabstract.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/uart.h"

class HalEsp32Uart : public HalUartAbstract
{
public:
    HalEsp32Uart(OSBase *osbase, UART_NUM_TYPE uartNum);
    virtual ~HalEsp32Uart();

    virtual bool initUart(int baudrate, int pinTX, int pinRx, int bufferSizeTx = 256, int bufferSizeRx = 1024) override;
    virtual OSBase::QueueHandle initReadUartFromQueueHandle(int baudrate, int pinTX, int pinRx, int bufferSizeTx = 256, int bufferSizeRx = 1024, int queueSize = 20) override;

    virtual bool writeData(uint8_t *data, uint16_t size, uint32_t timeout = 0) override;
    virtual bool readData(uint8_t *data, uint16_t size, uint32_t timeout = 0) override;

    virtual int processReceiveQueueHandleUart(OSBase::QueueHandle queueHandle, uint8_t *data, int maxLenth) override;

private:
    QueueSetHandle_t mUartQueueRx; ///< Queue for receiving data from UART
    OSBase *mOSbase;
    uart_port_t mUartNum;
    uart_event_t mUartEvent; //Receive event structure
};

#endif /* HAL_ESP32_INC_ESP32UART_H_ */
