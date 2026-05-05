#include "task/adcreadertask.h"
#include "loggermanager.h"
#include "esp_err.h"
#include <cmath>

AdcReaderTask::AdcReaderTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet)
{
}

AdcReaderTask::~AdcReaderTask()
{
    if (mAdcHandle) {
        adc_oneshot_del_unit(mAdcHandle);
        mAdcHandle = nullptr;
    }
    if (mCaliHandle) {
        adc_cali_delete_scheme_line_fitting(mCaliHandle);
        mCaliHandle = nullptr;
    }
}

// ============================================================
// onInitProcess: Khởi tạo ADC và spawn burst task
// ============================================================
void AdcReaderTask::onInitProcess()
{
    LOG_INFO("AdcReaderTask", "Init ADC hardware...");
    initAdc();
    initCalibration();

    // KHÔNG dùng vTaskDelayUntil(1ms) - pdMS_TO_TICKS(1)=0 với tickrate 100Hz sẽ crash!
    // Dùng burst task: ngủ 200ms -> burst 200 mẫu liên tục -> tính RMS -> lặp lại
    BaseType_t ret = xTaskCreate(adcBurstTask, "AdcBurst", 4096, this, 5, nullptr);
    if (ret != pdPASS) {
        LOG_ERROR("AdcReaderTask", "Failed to create adcBurstTask (ret=%d)", (int)ret);
    } else {
        LOG_INFO("AdcReaderTask", "adcBurstTask spawned OK");
    }
}

// TaskAbstract yêu cầu implement 2 hàm này dù không dùng
void AdcReaderTask::onTimer100HzProcess()  { /* Không dùng - ADC xử lý ở adcBurstTask riêng */ }
void AdcReaderTask::onQueueSetMessageProcess(OSBase::QueueHandle) { /* Không đăng ký queue nào */ }

// ============================================================
// initAdc: Tạo 1 unit ADC1 duy nhất, cấu hình 3 channel trên đó
// FIX: Gọi adc_oneshot_new_unit đúng 1 lần - tránh lỗi "ADC1 in use"
// ============================================================
void AdcReaderTask::initAdc()
{
    // B1: Tạo unit ADC1 (Singleton - chỉ gọi 1 lần)
    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    unit_cfg.unit_id  = ADC_UNIT_1;
    unit_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
    esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &mAdcHandle);
    if (err != ESP_OK) {
        LOG_ERROR("AdcReaderTask", "adc_oneshot_new_unit failed: 0x%x", err);
        return;
    }

    // B2: Cấu hình kênh (chỉ cần gọi adc_oneshot_config_channel - không tạo unit mới)
    adc_oneshot_chan_cfg_t chan_cfg = {};
    chan_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
    chan_cfg.atten    = ADC_ATTEN_DB_12; // 0-3.3V range (DB_11 deprecated)

    ESP_ERROR_CHECK(adc_oneshot_config_channel(mAdcHandle, CH_SCT01,    &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(mAdcHandle, CH_SCT02,    &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(mAdcHandle, CH_CHECKPIN, &chan_cfg));

    LOG_INFO("AdcReaderTask", "ADC1 init OK: GPIO36(CH0), GPIO39(CH3), GPIO35(CH7)");
}

// ============================================================
// initCalibration: Line Fitting từ eFuse (ESP32 classic)
// ============================================================
void AdcReaderTask::initCalibration()
{
    adc_cali_line_fitting_config_t cali_cfg = {};
    cali_cfg.unit_id  = ADC_UNIT_1;
    cali_cfg.atten    = ADC_ATTEN_DB_12;
    cali_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;

    esp_err_t err = adc_cali_create_scheme_line_fitting(&cali_cfg, &mCaliHandle);
    if (err != ESP_OK) {
        LOG_ERROR("AdcReaderTask", "Calibration failed (0x%x), dùng fallback linear", err);
        mCaliHandle = nullptr;
    } else {
        LOG_INFO("AdcReaderTask", "ADC Calibration OK (eFuse line fitting)");
    }
}

// ============================================================
// calibrateToMv: Raw -> mV (có calibration hoặc linear fallback)
// ============================================================
int AdcReaderTask::calibrateToMv(adc_channel_t ch, int raw)
{
    (void)ch;
    if (mCaliHandle) {
        int mv = 0;
        if (adc_cali_raw_to_voltage(mCaliHandle, raw, &mv) == ESP_OK) {
            return mv;
        }
    }
    // Fallback: tuyến tính 0..4095 -> 0..3300 mV
    return raw * 3300 / 4095;
}

// ============================================================
// computeAndSendRms:
//   - Burst 200 mẫu LIÊN TỤC (không delay giữa các mẫu)
//   - Tính True RMS với EMA DC offset
//   - Gửi kết quả sang PowerManagerTask
// ============================================================
void AdcReaderTask::computeAndSendRms()
{
    float sumSq1   = 0.0f;
    float sumSq2   = 0.0f;
    float sumCheck = 0.0f;

    // Burst 200 mẫu liên tục - tốc độ ADC ESP32 ~40kHz
    // -> 200 mẫu = ~5ms, bắt trọn nhiều chu kỳ 50Hz
    for (int i = 0; i < RMS_SAMPLES; i++)
    {
        int raw1 = 0, raw2 = 0, rawCheck = 0;
        adc_oneshot_read(mAdcHandle, CH_SCT01,    &raw1);
        adc_oneshot_read(mAdcHandle, CH_SCT02,    &raw2);
        adc_oneshot_read(mAdcHandle, CH_CHECKPIN, &rawCheck);

        float mv1     = (float)calibrateToMv(CH_SCT01,    raw1);
        float mv2     = (float)calibrateToMv(CH_SCT02,    raw2);
        float mvCheck = (float)calibrateToMv(CH_CHECKPIN, rawCheck);

        // EMA: bám điểm DC thực tế (thay thế hardcode 1.65V)
        mDcOffset1 = EMA_ALPHA * mv1 + (1.0f - EMA_ALPHA) * mDcOffset1;
        mDcOffset2 = EMA_ALPHA * mv2 + (1.0f - EMA_ALPHA) * mDcOffset2;

        float ac1 = mv1 - mDcOffset1;
        float ac2 = mv2 - mDcOffset2;

        sumSq1   += ac1 * ac1;
        sumSq2   += ac2 * ac2;
        sumCheck += mvCheck;
    }

    // Tính RMS
    float rms_mv1    = std::sqrt(sumSq1 / (float)RMS_SAMPLES);
    float rms_mv2    = std::sqrt(sumSq2 / (float)RMS_SAMPLES);
    float avgCheckMv = sumCheck / (float)RMS_SAMPLES;

    float ampe1 = rms_mv1 * AMPS_PER_MV;
    float ampe2 = rms_mv2 * AMPS_PER_MV;

    // Xoá noise floor (không có dòng -> hiện 0)
    if (ampe1 < NOISE_FLOOR) ampe1 = 0.0f;
    if (ampe2 < NOISE_FLOOR) ampe2 = 0.0f;

    LOG_DEBUG("AdcReaderTask",
              "DC1=%.0fmV DC2=%.0fmV | RMS=%.1fmV/%.1fmV | A1=%.2fA A2=%.2fA",
              mDcOffset1, mDcOffset2, rms_mv1, rms_mv2, ampe1, ampe2);

    // Gửi sang PowerManagerTask
    if (gQueueADCValueToPowerManageTask != 0)
    {
        PowerManagerTask::SampleResult result{};
        result.ampe[0]    = ampe1;
        result.ampe[1]    = ampe2;
        result.voltage[2] = avgCheckMv;

        if (mOSBase->queueSend(gQueueADCValueToPowerManageTask, &result) != OSBase::QUEUE_OK) {
            LOG_DEBUG("AdcReaderTask", "Queue gQueueADCValueToPowerManageTask full, bỏ qua");
        }
    }
}

// ============================================================
// adcBurstTask: FreeRTOS task
//   Ngủ 200ms -> burst 200 mẫu -> tính RMS -> lặp lại
//   KHÔNG dùng vTaskDelayUntil(1ms) - crash với tickrate 100Hz!
// ============================================================
void AdcReaderTask::adcBurstTask(void *arg)
{
    AdcReaderTask *self = static_cast<AdcReaderTask *>(arg);

    // Chờ hệ thống khởi động ổn định trước khi bắt đầu đọc ADC
    vTaskDelay(pdMS_TO_TICKS(2000));
    LOG_INFO("AdcReaderTask", "ADC burst sampling started");

    while (true)
    {
        self->computeAndSendRms();
        // Nghỉ 1 giây giữa các lần đọc RMS (1Hz update rate, đủ cho App)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
