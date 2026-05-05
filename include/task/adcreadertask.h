#ifndef ADCREADERTASK_H
#define ADCREADERTASK_H

#include "taskabstract.h"
#include "common/common.h"
#include "task/powermanagertask.h"
#include "esp_adc/adc_oneshot.h"
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
    // Một handle ADC1 duy nhất (Singleton) - chia sẻ cho cả 3 channel
    adc_oneshot_unit_handle_t mAdcHandle{nullptr};
    adc_cali_handle_t mCaliHandle{nullptr};

    // Mapping GPIO -> ADC1 Channel (ESP32)
    // GPIO36 -> ADC1_CH0, GPIO39 -> ADC1_CH3, GPIO35 -> ADC1_CH7
    static constexpr adc_channel_t CH_SCT01    = ADC_CHANNEL_0; // GPIO36
    static constexpr adc_channel_t CH_SCT02    = ADC_CHANNEL_3; // GPIO39
    static constexpr adc_channel_t CH_CHECKPIN = ADC_CHANNEL_7; // GPIO35

    // EMA DC Offset (mV) - Khởi tạo 1650mV, tự bám điểm 0V thực tế
    float mDcOffset1{1650.0f};
    float mDcOffset2{1650.0f};
    // Alpha nhỏ = bám DC chậm, không ảnh hưởng thành phần AC
    static constexpr float EMA_ALPHA = 0.005f;

    // RMS: burst 200 mẫu LIÊN TỤC (không delay giữa các mẫu)
    // Ở tốc độ ADC ESP32 ~40kHz, 200 mẫu = 5ms -> bắt trọn 50Hz sin
    static constexpr int   RMS_SAMPLES  = 200;
    // 20A / 1000mV (kế thừa: MAX_AMPLE_SENSOR=20A tại delta 1V)
    static constexpr float AMPS_PER_MV  = 20.0f / 1000.0f;
    // Noise floor: loại bỏ nhiễu dưới 0.1A
    static constexpr float NOISE_FLOOR  = 0.1f;

    void initAdc();
    void initCalibration();
    int  calibrateToMv(adc_channel_t ch, int raw);
    void computeAndSendRms();

    // FreeRTOS task tĩnh: ngủ 200ms -> burst 200 mẫu -> ngủ tiếp
    static void adcBurstTask(void *arg);
};

#endif // ADCREADERTASK_H
