/*
 * esp32gpio.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ESP32_INC_ESP32GPIO_H_
#define HAL_ESP32_INC_ESP32GPIO_H_

#include "halgpioabstract.h"

class HalEsp32Gpio : public HalGpioAbstract
{
public:
	HalEsp32Gpio(uint16_t pin, GpioMode mode, bool pullup = false, bool pulldown = false, int initial_level = false);
	virtual ~HalEsp32Gpio();

	bool Hal_Gpio_Init(GpioMode mode, bool pullup = false, bool pulldown = false, int initial_level = false) override;
	bool Hal_Gpio_ReadPin() override;
	void Hal_Gpio_WritePin(GpioPinState state) override;
	void Hal_Gpio_TogglePin() override;

private:
	bool isOutputMode(GpioMode mode) const;
	uint16_t mPin{0};
	GpioMode mMode{GpioMode::GPIO_MODE_DISABLE};
};

#endif /* HAL_ESP32_INC_ESP32GPIO_H_ */
