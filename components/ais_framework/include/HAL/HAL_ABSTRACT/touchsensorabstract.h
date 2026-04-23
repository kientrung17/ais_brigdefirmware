/*
 * touchsensorabstract.h
 */

#ifndef HAL_ABSTRACT_INC_TOUCHSENSORABSTRACT_H_
#define HAL_ABSTRACT_INC_TOUCHSENSORABSTRACT_H_

#include <stdint.h>

class TouchSensorAbstract
{
public:
    virtual ~TouchSensorAbstract() = default;

    // Initialize sensor on a given channel/pad index.
    virtual bool init(int filterPeriodMs = 10) = 0;

    // Calibrate baseline by averaging some samples; returns true on success.
    virtual bool calibrate(int samples = 32, int sampleDelayMs = 5) = 0;

    // Set absolute threshold (raw/filtered value boundary for ON detection).
    virtual void setThreshold(uint32_t threshold) = 0;

    // Optional convenience: percentage below baseline (0..100). Implementation may clamp.
    virtual void setRelativeThresholdPercent(uint8_t percentBelowBaseline) = 0;

    // Read raw and filtered values.
    virtual bool readRaw(uint32_t &value) = 0;
    virtual bool readFiltered(uint32_t &value) = 0;

    // Configure debounce counts (consecutive samples to toggle state).
    virtual void setDebounce(int onCount, int offCount) = 0;

    // Take one sample, update internal ON/OFF state, return true if state changed.
    virtual bool update() = 0;

    // Current ON/OFF state after last update().
    virtual bool isOn() const = 0;
};

#endif /* HAL_ABSTRACT_INC_TOUCHSENSORABSTRACT_H_ */

