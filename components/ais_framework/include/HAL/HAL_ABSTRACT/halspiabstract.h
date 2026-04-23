/*
 * halspiabstract.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ABSTRACT_INC_HALSPIABSTRACT_H_
#define HAL_ABSTRACT_INC_HALSPIABSTRACT_H_
#include <stdint.h>

class HalSpiAbstract
{
public:
    virtual ~HalSpiAbstract() = default;
    virtual bool Hal_Spi_Transmit(uint8_t *data, uint16_t size, uint32_t timeout) = 0;
    virtual bool Hal_Spi_Receive(uint8_t *data, uint16_t size, uint32_t timeout) = 0;
    virtual bool Hal_Spi_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint16_t Timeout) = 0;
};

#endif /* HAL_ABSTRACT_INC_HALSPIABSTRACT_H_ */
