#include "task/adcreadertask.h"
#include "loggermanager.h"
#include <cmath>

AdcReaderTask::AdcReaderTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
}

AdcReaderTask::~AdcReaderTask()
{
    if (mAdcSCT01) delete mAdcSCT01;
    if (mAdcSCT02) delete mAdcSCT02;
    if (mAdcCHECKPin) delete mAdcCHECKPin;
    if (mCaliHandle) {
        // Dùng line_fitting - chip ESP32 classic không hỗ trợ curve_fitting
        adc_cali_delete_scheme_line_fitting(mCaliHandle);
    }
}

// ============================================================
// onInitProcess: Gọi bởi framework ngay khi task khởi động
// ============================================================
void AdcReaderTask::onInitProcess()
{
    LOG_INFO("AdcReaderTask", "Init ADC Pins (POLLING mode)");
    mAdcSCT01 = new HalEsp32Adc();
    mAdcSCT02 = new HalEsp32Adc();
    mAdcCHECKPin = new HalEsp32Adc();

    if (!mAdcSCT01->init(PIN_ADC_SCT013_01, HalAdcAbstract::Mode::POLLING)) {
        LOG_ERROR("AdcReaderTask", "Init mAdcSCT01 (GPIO %d) failed", PIN_ADC_SCT013_01);
    }
    if (!mAdcSCT02->init(PIN_ADC_SCT013_02, HalAdcAbstract::Mode::POLLING)) {
        LOG_ERROR("AdcReaderTask", "Init mAdcSCT02 (GPIO %d) failed", PIN_ADC_SCT013_02);
    }
    if (!mAdcCHECKPin->init(PIN_ADC_CHECK_PIN, HalAdcAbstract::Mode::POLLING)) {
        LOG_ERROR("AdcReaderTask", "Init mAdcCHECKPin (GPIO %d) failed", PIN_ADC_CHECK_PIN);
    }

    initCalibration();

    // Tạo FreeRTOS task riêng để lấy mẫu 1kHz
    // (không thể làm trong onTimer100HzProcess vì chỉ 100Hz)
    xTaskCreate(adcSamplingTask, "AdcSampling", 4096, this, 5, nullptr);

    LOG_INFO("AdcReaderTask", "Init complete. Sampling task spawned at 1kHz.");
}

void AdcReaderTask::onTimer100HzProcess()
{
    // AdcReaderTask không cần xử lý timer, vòng lặp 1kHz chạy riêng
}

void AdcReaderTask::onQueueSetMessageProcess(OSBase::QueueHandle queue_sem)
{
    // AdcReaderTask không đăng ký queue nào
    (void)queue_sem;
}

// ============================================================
// initCalibration: Dùng Line Fitting (ESP32 classic eFuse)
// ============================================================
void AdcReaderTask::initCalibration()
{
    LOG_INFO("AdcReaderTask", "Init ADC Calibration Line Fitting (ESP-IDF V5)");

    adc_cali_line_fitting_config_t cali_config = {};
    cali_config.unit_id    = ADC_UNIT_1;         // GPIO 35, 36, 39 đều thuộc ADC1
    cali_config.atten      = ADC_ATTEN_DB_12;    // 0-3.3V range (DB_11 đã deprecated)
    cali_config.bitwidth   = ADC_BITWIDTH_DEFAULT;

    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &mCaliHandle);
    if (ret != ESP_OK) {
        LOG_ERROR("AdcReaderTask", "Calibration init failed (err=0x%x), sẽ dùng raw value", ret);
        mCaliHandle = nullptr;
    } else {
        LOG_INFO("AdcReaderTask", "Calibration OK - đọc eFuse thành công");
    }
}

// ============================================================
// calibrateToMv: Raw ADC -> mV (có hoặc không có calibration)
// ============================================================
int AdcReaderTask::calibrateToMv(uint32_t raw)
{
    if (!mCaliHandle) {
        // Fallback: tuyến tính đơn giản 0..4095 -> 0..3300 mV
        return (int)(raw * 3300 / 4095);
    }
    int voltage_mv = 0;
    if (adc_cali_raw_to_voltage(mCaliHandle, (int)raw, &voltage_mv) == ESP_OK) {
        return voltage_mv;
    }
    return (int)(raw * 3300 / 4095);
}

// ============================================================
// sampleOnce: Lấy 1 mẫu từ cả 3 channel, cộng dồn vào RMS
// Gọi từ vòng lặp 1kHz của adcSamplingTask
// ============================================================
void AdcReaderTask::sampleOnce()
{
    uint32_t raw1 = 0, raw2 = 0, rawCheck = 0;

    mAdcSCT01->readLoopADC();
    mAdcSCT01->getADCValue(raw1);

    mAdcSCT02->readLoopADC();
    mAdcSCT02->getADCValue(raw2);

    mAdcCHECKPin->readLoopADC();
    mAdcCHECKPin->getADCValue(rawCheck);

    // Calibrate -> mV
    float mv1     = (float)calibrateToMv(raw1);
    float mv2     = (float)calibrateToMv(raw2);
    float mvCheck = (float)calibrateToMv(rawCheck);

    // ---- Dynamic DC Offset (EMA) ----
    mDcOffset1 = (EMA_ALPHA * mv1)     + ((1.0f - EMA_ALPHA) * mDcOffset1);
    mDcOffset2 = (EMA_ALPHA * mv2)     + ((1.0f - EMA_ALPHA) * mDcOffset2);

    // ---- AC thuần túy (đã khử DC) ----
    float ac1 = mv1 - mDcOffset1;
    float ac2 = mv2 - mDcOffset2;

    // ---- Cộng dồn bình phương ----
    mSumSq1      += ac1 * ac1;
    mSumSq2      += ac2 * ac2;
    mSumCheckPin += mvCheck;
    mSampleCount++;

    // ---- Tính RMS sau mỗi RMS_WINDOW_SIZE mẫu ----
    if (mSampleCount >= RMS_WINDOW_SIZE)
    {
        float rms_mv1 = std::sqrt(mSumSq1 / (float)RMS_WINDOW_SIZE);
        float rms_mv2 = std::sqrt(mSumSq2 / (float)RMS_WINDOW_SIZE);
        float avg_check_mv = mSumCheckPin / (float)RMS_WINDOW_SIZE;

        float ampe1 = rms_mv1 * AMPS_PER_MV;
        float ampe2 = rms_mv2 * AMPS_PER_MV;

        // Xoá noise floor
        if (ampe1 < NOISE_FLOOR_AMPS) ampe1 = 0.0f;
        if (ampe2 < NOISE_FLOOR_AMPS) ampe2 = 0.0f;

        LOG_DEBUG("AdcReaderTask",
                  "DC1=%.1fmV DC2=%.1fmV | RMS1=%.1fmV RMS2=%.1fmV | A1=%.2fA A2=%.2fA",
                  mDcOffset1, mDcOffset2, rms_mv1, rms_mv2, ampe1, ampe2);

        // ---- Gửi kết quả sang PowerManagerTask ----
        if (gQueueADCValueToPowerManageTask != 0)
        {
            PowerManagerTask::SampleResult result{};
            result.ampe[0]    = ampe1;
            result.ampe[1]    = ampe2;
            result.voltage[2] = avg_check_mv; // Pin voltage (mV, PowerTask sẽ scale)

            if (mOSBase->queueSend(gQueueADCValueToPowerManageTask, &result) != OSBase::QUEUE_OK)
            {
                LOG_DEBUG("AdcReaderTask", "Queue gQueueADCValueToPowerManageTask full");
            }
        }

        // ---- Reset bộ tích lũy ----
        mSumSq1      = 0.0f;
        mSumSq2      = 0.0f;
        mSumCheckPin = 0.0f;
        mSampleCount = 0;
    }
}

// ============================================================
// adcSamplingTask: FreeRTOS task chạy vòng lặp 1kHz
// ============================================================
void AdcReaderTask::adcSamplingTask(void *arg)
{
    AdcReaderTask *self = static_cast<AdcReaderTask *>(arg);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // 1ms = 1kHz

    while (true)
    {
        self->sampleOnce();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
