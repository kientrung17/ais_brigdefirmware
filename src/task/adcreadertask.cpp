#include "task/adcreadertask.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "loggermanager.h"
#include <cmath>

AdcReaderTask::AdcReaderTask(std::string nameTask, int numElementQueueSet)
    : TaskAbstract(nameTask, numElementQueueSet) {}

AdcReaderTask::~AdcReaderTask() {
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
void AdcReaderTask::onInitProcess() {
  LOG_INFO("AdcReaderTask", "Init ADC hardware...");
  initAdc();
  initCalibration();

  // KHÔNG dùng vTaskDelayUntil(1ms) - pdMS_TO_TICKS(1)=0 với tickrate 100Hz sẽ
  // crash! Dùng burst task: ngủ 200ms -> burst 200 mẫu liên tục -> tính RMS ->
  // lặp lại
  BaseType_t ret =
      xTaskCreate(adcBurstTask, "AdcBurst", 4096, this, 5, nullptr);
  if (ret != pdPASS) {
    LOG_ERROR("AdcReaderTask", "Failed to create adcBurstTask (ret=%d)",
              (int)ret);
  } else {
    LOG_INFO("AdcReaderTask", "adcBurstTask spawned OK");
  }
}

// TaskAbstract yêu cầu implement 2 hàm này dù không dùng
void AdcReaderTask::onTimer100HzProcess() { /* Không dùng - ADC xử lý ở
                                               adcBurstTask riêng */
}
void AdcReaderTask::onQueueSetMessageProcess(
    OSBase::QueueHandle) { /* Không đăng ký queue nào */ }

// ============================================================
// initAdc: Tạo 1 unit ADC1 duy nhất, cấu hình 3 channel trên đó
// FIX: Gọi adc_oneshot_new_unit đúng 1 lần - tránh lỗi "ADC1 in use"
// ============================================================
void AdcReaderTask::initAdc() {
  // B1: Tạo unit ADC1 (Singleton - chỉ gọi 1 lần)
  adc_oneshot_unit_init_cfg_t unit_cfg = {};
  unit_cfg.unit_id = ADC_UNIT_1;
  unit_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
  esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &mAdcHandle);
  if (err != ESP_OK) {
    LOG_ERROR("AdcReaderTask", "adc_oneshot_new_unit failed: 0x%x", err);
    return;
  }

  // B2: Cấu hình kênh (chỉ cần gọi adc_oneshot_config_channel - không tạo unit
  // mới)
  adc_oneshot_chan_cfg_t chan_cfg = {};
  chan_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
  chan_cfg.atten = ADC_ATTEN_DB_12; // 0-3.3V range (DB_11 deprecated)

  ESP_ERROR_CHECK(adc_oneshot_config_channel(mAdcHandle, CH_SCT01, &chan_cfg));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(mAdcHandle, CH_SCT02, &chan_cfg));
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(mAdcHandle, CH_CHECKPIN, &chan_cfg));

  LOG_INFO("AdcReaderTask",
           "ADC1 init OK: GPIO36(CH0), GPIO39(CH3), GPIO35(CH7)");
}

// ============================================================
// initCalibration: Line Fitting từ eFuse (ESP32 classic)
// ============================================================
void AdcReaderTask::initCalibration() {
  adc_cali_line_fitting_config_t cali_cfg = {};
  cali_cfg.unit_id = ADC_UNIT_1;
  cali_cfg.atten = ADC_ATTEN_DB_12;
  cali_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;

  esp_err_t err = adc_cali_create_scheme_line_fitting(&cali_cfg, &mCaliHandle);
  if (err != ESP_OK) {
    LOG_ERROR("AdcReaderTask",
              "Calibration failed (0x%x), dùng fallback linear", err);
    mCaliHandle = nullptr;
  } else {
    LOG_INFO("AdcReaderTask", "ADC Calibration OK (eFuse line fitting)");
  }
}

// ============================================================
// calibrateToMv: Raw -> mV (có calibration hoặc linear fallback)
// ============================================================
int AdcReaderTask::calibrateToMv(adc_channel_t ch, int raw) {
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
//   - Tính True RMS với Welford's algorithm
// ============================================================
void AdcReaderTask::computeAndSendRms() {
  // Welford's Online Algorithm: tính mean và variance trong 1 pass
  // DC = mean của burst => AC = sample - mean => không dùng EMA để tránh "chạy
  // theo AC"
  double mean1 = 0.0, mean2 = 0.0;
  double M2_1 = 0.0, M2_2 = 0.0;
  double sumCheck = 0.0;
  int count = 0;

  // Sample đúng 100ms = 5 chu kỳ 50Hz
  // 5 chu kỳ: sai số pha giảm sqrt(5)x, variance RMS ổn định hơn nhiều
  const uint64_t WINDOW_US = 100000ULL;
  uint64_t startUs = esp_timer_get_time();

  // ----- Diagnostic: Log raw đầu burst -----
  {
    int r1 = 0, r2 = 0, rC = 0;
    adc_oneshot_read(mAdcHandle, CH_SCT01, &r1);
    adc_oneshot_read(mAdcHandle, CH_SCT02, &r2);
    adc_oneshot_read(mAdcHandle, CH_CHECKPIN, &rC);
    LOG_DEBUG("AdcReaderTask", "[RAW] GPIO36=%d GPIO39=%d GPIO35=%d", r1, r2,
              rC);
  }
  // ------------------------------------------

  while ((esp_timer_get_time() - startUs) < WINDOW_US) {
    int raw1 = 0, raw2 = 0, rawCheck = 0;
    adc_oneshot_read(mAdcHandle, CH_SCT01, &raw1);
    adc_oneshot_read(mAdcHandle, CH_SCT02, &raw2);
    adc_oneshot_read(mAdcHandle, CH_CHECKPIN, &rawCheck);

    double mv1 = (double)calibrateToMv(CH_SCT01, raw1);
    double mv2 = (double)calibrateToMv(CH_SCT02, raw2);
    double mvCheck = (double)calibrateToMv(CH_CHECKPIN, rawCheck);

    count++;

    // Welford update ch1: mean và M2 (= sum of squared deviations from mean)
    double d1a = mv1 - mean1;
    mean1 += d1a / count;
    M2_1 += d1a * (mv1 - mean1);

    // Welford update ch2
    double d2a = mv2 - mean2;
    mean2 += d2a / count;
    M2_2 += d2a * (mv2 - mean2);

    sumCheck += mvCheck;
  }

  if (count < 2)
    return;

  // RMS_AC = sqrt(variance) = sqrt(M2 / count)
  float rms_mv1 = (float)std::sqrt(M2_1 / count);
  float rms_mv2 = (float)std::sqrt(M2_2 / count);
  float avgCheckMv = (float)(sumCheck / count);

  // Cập nhật EMA DC offset cho lần sau (long-term tracking)
  mDcOffset1 = EMA_ALPHA * (float)mean1 + (1.0f - EMA_ALPHA) * mDcOffset1;
  mDcOffset2 = EMA_ALPHA * (float)mean2 + (1.0f - EMA_ALPHA) * mDcOffset2;

  float ampe1 = rms_mv1 * AMPS_PER_MV;
  float ampe2 = rms_mv2 * AMPS_PER_MV;

  // --- E-STOP TRIGGER: OVERLOAD ---
  if (ampe1 > 15.0f || ampe2 > 15.0f) {
      if (mOSBase->isStarted() && gEmergencyEventGroup != nullptr) {
          xEventGroupSetBits(gEmergencyEventGroup, BIT_ESTOP_OVERLOAD);
      }
  }

  if (ampe1 < NOISE_FLOOR)
    ampe1 = 0.0f;
  if (ampe2 < NOISE_FLOOR)
    ampe2 = 0.0f;

  // EMA output smoothing: giảm jitter hiển thị (không ảnh hưởng đến RMS tính
  // toán)
  mSmoothedAmpe1 =
      SMOOTH_ALPHA * ampe1 + (1.0f - SMOOTH_ALPHA) * mSmoothedAmpe1;
  mSmoothedAmpe2 =
      SMOOTH_ALPHA * ampe2 + (1.0f - SMOOTH_ALPHA) * mSmoothedAmpe2;

  LOG_DEBUG(
      "AdcReaderTask",
      "DC1=%.0fmV DC2=%.0fmV | RMS=%.1fmV/%.1fmV | A1=%.2fA A2=%.2fA [N=%d]",
      (float)mean1, (float)mean2, rms_mv1, rms_mv2, mSmoothedAmpe1,
      mSmoothedAmpe2, count);

  if (gQueueADCValueToPowerManageTask != 0) {
    PowerManagerTask::SampleResult result{};
    result.ampe[0] = mSmoothedAmpe1;
    result.ampe[1] = mSmoothedAmpe2;
    result.voltage[2] = avgCheckMv;
    mOSBase->queueSend(gQueueADCValueToPowerManageTask, &result);
  }
}

// ============================================================
// adcBurstTask: FreeRTOS task
//   Ngủ 200ms -> burst 200 mẫu -> tính RMS -> lặp lại
//   KHÔNG dùng vTaskDelayUntil(1ms) - crash với tickrate 100Hz!
// ============================================================
void AdcReaderTask::adcBurstTask(void *arg) {
  AdcReaderTask *self = static_cast<AdcReaderTask *>(arg);

  // Chờ hệ thống khởi động ổn định trước khi bắt đầu đọc ADC
  vTaskDelay(pdMS_TO_TICKS(2000));
  LOG_INFO("AdcReaderTask", "ADC burst sampling started");

  while (true) {
    self->computeAndSendRms();
    // Nghỉ 200ms giữa các lần đọc RMS (5Hz update rate)
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
