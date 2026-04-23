// /*
//  * esp32touch.h
//  *
//  * Hướng dẫn sử dụng nhanh (Usage):
//  *
//  *  // 1) Khai báo và khởi tạo cảm biến (chọn pad phù hợp phần cứng)
//  *  EspidfTouchSensor touch;
//  *  const int kPad = 5; // ví dụ: TOUCH_PAD5 (GPIO33 trên ESP32 classic)
//  *  touch.init(kPad, /*filterPeriodMs=*/10);
//  *
//  *  // 2) Hiệu chuẩn để lấy baseline và tự động tính ngưỡng theo %
//  *  touch.calibrate(/*samples=*/32, /*sampleDelayMs=*/0);
//  *  touch.setRelativeThresholdPercent(12); // ON khi giá trị < baseline - 12%
//  *
//  *  // 3) Chống dội (debounce) – yêu cầu N mẫu liên tiếp để đổi trạng thái
//  *  touch.setDebounce(/*onCount=*/3, /*offCount=*/3);
//  *
//  *  // 4) Trong vòng lặp định kỳ (ví dụ gọi mỗi 10–20 ms):
//  *  if (touch.update()) {
//  *      // chỉ vào đây khi trạng thái vừa thay đổi
//  *      if (touch.isOn()) {
//  *          // xử lý khi nhấn (ON)
//  *      } else {
//  *          // xử lý khi nhả (OFF)
//  *      }
//  *  }
//  *
//  * Ghi chú:
//  * - Trên ESP32, giá trị đo giảm khi có chạm => ngưỡng được áp dụng theo điều kiện value < threshold.
//  * - Có thể đặt ngưỡng tuyệt đối bằng setThreshold() nếu bạn đã biết biên cụ thể.
//  * - Có thể gọi lại calibrate() định kỳ để bù trôi theo môi trường.
//  */

#ifndef HAL_ESP32_INC_ESP32TOUCH_H_
#define HAL_ESP32_INC_ESP32TOUCH_H_

#include <stdint.h>
#include "driver/touch_pad.h"
#include "touchsensorabstract.h"

class EspidfTouchSensor : public TouchSensorAbstract
{
public:
    EspidfTouchSensor(int pad);
    ~EspidfTouchSensor();

    bool init(int filterPeriodMs = 10) override;
    bool calibrate(int samples = 32, int sampleDelayMs = 5) override;

    void setThreshold(uint32_t threshold) override;
    void setRelativeThresholdPercent(uint8_t percentBelowBaseline) override;

    bool readRaw(uint32_t &value) override;
    bool readFiltered(uint32_t &value) override;

    void setDebounce(int onCount, int offCount) override;
    bool update() override;
    bool isOn() const override { return mIsOn; }

private:
    int mPad{-1};                 // user-supplied GPIO or touch channel
    touch_pad_t mTouchPad{TOUCH_PAD_MAX};
    int mFilterPeriodMs{10};
    uint32_t mBaseline{0};
    uint32_t mThreshold{0};
    uint8_t mRelPercent{10}; // default 10% below baseline
    int mDebounceOn{3};
    int mDebounceOff{3};
    int mCntOn{0};
    int mCntOff{0};
    bool mIsOn{false};
    bool mInitialized{false};
};

#endif /* HAL_ESP32_INC_ESP32TOUCH_H_ */
