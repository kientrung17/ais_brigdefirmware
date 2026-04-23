/*
 * haluartabstract.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ABSTRACT_INC_HALUARTABSTRACT_H_
#define HAL_ABSTRACT_INC_HALUARTABSTRACT_H_
#include <stdint.h>
#include "OSBase.h"

class HalUartAbstract
{
public:
    enum UART_NUM_TYPE
    {
        UART_1 = 0,
        UART_2,
        UART_3,
        UART_4,
        UART_MAX_NUM
    };

    HalUartAbstract() {};
    virtual ~HalUartAbstract() {};
    //initUart: initialize UART not using received queue handle
    //param: baudrate, pinTX, pinRx, bufferSizeTx, bufferSizeRx
    // bufferSizeTx: size of buffer for transmit data, default is 256 bytes
    // bufferSizeRx: size of buffer for receive data, default is 1024 bytes
    virtual bool initUart(int baudrate, int pinTX, int pinRx, int bufferSizeTx = 256, int bufferSizeRx = 1024) = 0;
   
    //initReadUartFromQueueHandle: initialize UART using received queue handle
    //param: baudrate, pinTX, pinRx, bufferSizeTx, bufferSizeRx, queueSize
    // queueSize: size of queue for receiving data, default is 20
    virtual OSBase::QueueHandle initReadUartFromQueueHandle(int baudrate, int pinTX, int pinRx, int bufferSizeTx = 256, int bufferSizeRx = 1024, int queueSize = 20) = 0;
    
    // writeData: write data to UART
    // param: data: pointer to data to write, size: size of data to write, timeout: timeout for writing data
    // return: true if data is written successfully, false otherwise
    virtual bool writeData(uint8_t *data, uint16_t size, uint32_t timeout = 0) = 0;
    
    // readData: read data from UART
    // param: data: pointer to buffer for receiving data, size: size of data to read, timeout: timeout for reading data
    // return: true if data is read successfully, false otherwise
    virtual bool readData(uint8_t *data, uint16_t size, uint32_t timeout = 0) = 0;

    //processReceiveQueueHandleUart: process data received from UART using queue handle
    // param: queueHandle: handle of queue for receiving data, data: pointer to buffer for receiving data, maxLenth: maximum length of data to receive
    // return: number of bytes received from UART
    //using: 
    //B1:  initReadUartFromQueueHandle(int baudrate, int pinTX, int pinRx, int bufferSizeTx = 256, int bufferSizeRx = 1024, int queueSize = 20) = 0;
    //B2: When recwived queue, call this function to process data
    virtual int processReceiveQueueHandleUart(OSBase::QueueHandle queueHandle, uint8_t *data, int maxLenth) = 0;
};

#endif /* HAL_ABSTRACT_INC_HALUARTABSTRACT_H_ */
