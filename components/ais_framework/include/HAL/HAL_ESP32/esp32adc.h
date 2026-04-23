/*
 * esp32adc.h
 *
 *  Created on: Dec 7, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ESP32_INC_ESP32ADC_H_
#define HAL_ESP32_INC_ESP32ADC_H_

#include "haladcabstract.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include <vector>

class HalEsp32Adc : public HalAdcAbstract
{
public:
    // Trinh dieu khien ADC cho ESP32: cho phep chon doc theo kieu polling (oneshot) hoac DMA (continuous).
    HalEsp32Adc() = default;
    ~HalEsp32Adc();
    static constexpr uint8_t MAX_ADC_VALUE_ARRAY = ADC_CHANNEL_9 + 1; //10 phan tu

    // B1:
    bool init(uint16_t pin, Mode mode = Mode::POLLING, uint16_t sampleFrequence = 20000) override;
    // B2:
    bool readLoopADC() override;
    // B3
    bool getADCValue(uint32_t &value) override;

private:
    bool configureAdcChannel(uint16_t pin);
    bool setupOneshot();    // Ones-shot (polling) setup
    bool setupContinuous(); // DMA continuous setup
    void cleanup();

    uint16_t mPin{0};
    Mode mMode{Mode::POLLING};
    bool mConfigured{false};                  // Da init thanh cong hay chua
    adc_unit_t mAdcUnit{ADC_UNIT_1};          // Don vi ADC ung voi GPIO da chon (set moi instance qua init)
    adc_channel_t mAdcChannel{ADC_CHANNEL_0}; // Kenh ADC ung voi GPIO da chon (set moi instance qua init)
    adc_oneshot_unit_handle_t mOneshotHandle{nullptr};
    adc_continuous_handle_t mContinuousHandle{nullptr};
    uint32_t mSampleFreqHz{20000}; // Tan so mau mac dinh cho DMA
    uint32_t mFrameSize{256};      // Kich thuoc frame DMA (byte)
    static int32_t mADCValueMap[MAX_ADC_VALUE_ARRAY];
};

#endif /* HAL_ESP32_INC_ESP32ADC_H_ */
