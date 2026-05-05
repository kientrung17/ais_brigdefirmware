#ifndef ADCREADERTASK_H
#define ADCREADERTASK_H

#include "taskabstract.h"
#include "common/common.h"
#include "task/powermanagertask.h"
#include "HAL_ESP32/esp32adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class AdcReaderTask : public TaskAbstract
{
public:
    AdcReaderTask(std::string nameTask, int numElementQueueSet);
    ~AdcReaderTask();

    void onInitProcess() override;
    void onTimer100HzProcess() override;
    void onQueueSetMessageProcess(OSBase::QueueHandle queue_sem) override;

private:
    HalEsp32Adc *mAdcSCT01{nullptr};
    HalEsp32Adc *mAdcSCT02{nullptr};
    HalEsp32Adc *mAdcCHECKPin{nullptr};
    adc_cali_handle_t mCaliHandle{nullptr};

    // Lọc DC động (Exponential Moving Average) - khởi tạo 1650mV
    float mDcOffset1{1650.0f};
    float mDcOffset2{1650.0f};
    // Alpha nhỏ = bám DC chậm, không bị ảnh hưởng bởi AC
    static constexpr float EMA_ALPHA = 0.005f;

    // Tích lũy bình phương cho RMS
    float mSumSq1{0.0f};
    float mSumSq2{0.0f};
    float mSumCheckPin{0.0f};
    int mSampleCount{0};
    // Tính RMS mỗi 200 mẫu (200ms @ 1kHz)
    static constexpr int RMS_WINDOW_SIZE = 200;
    // 20A / 1000mV -> 0.02 A/mV (kế thừa từ code cũ: MAX_AMPLE_SENSOR=20, delta_vol=1V)
    static constexpr float AMPS_PER_MV = 20.0f / 1000.0f;
    // Noise floor: loại bỏ nhiễu khi không có dòng
    static constexpr float NOISE_FLOOR_AMPS = 0.1f;

    void initCalibration();
    int calibrateToMv(uint32_t raw);
    void sampleOnce();

    static void adcSamplingTask(void *arg);
};

#endif // ADCREADERTASK_H
