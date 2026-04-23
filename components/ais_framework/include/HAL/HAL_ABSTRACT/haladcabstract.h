/*
 * haladcabstract.h
 *
 *  Created on: Dec 7, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ABSTRACT_INC_HALADCABSTRACT_H_
#define HAL_ABSTRACT_INC_HALADCABSTRACT_H_

#include <stdint.h>

class HalAdcAbstract
{
public:
    enum class Mode : uint8_t
    {
        POLLING = 0, // Use oneshot/polling conversion
        DMA          // Use continuous DMA sampling
    };

    virtual ~HalAdcAbstract() = default;
    // B1:
    //  Initialize ADC on the given pin with selected mode.
    virtual bool init(uint16_t pin, Mode mode = Mode::POLLING, uint16_t sampleFrequence = 20000) = 0;
    // B1: đặt  hàm readLoopADC trong vòng lặp với chế độ DMA
    virtual bool readLoopADC() = 0;
    // B3: Lấy giá trị
    //  Read latest raw ADC value; returns true on success.
    virtual bool getADCValue(uint32_t &value) = 0;
};

#endif /* HAL_ABSTRACT_INC_HALADCABSTRACT_H_ */
