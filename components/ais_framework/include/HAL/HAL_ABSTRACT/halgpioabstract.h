/*
 * halgpioabstract.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ABSTRACT_INC_HALGPIOABSTRACT_H_
#define HAL_ABSTRACT_INC_HALGPIOABSTRACT_H_
#include <stdint.h>

class HalGpioAbstract
{
public:
	enum class GpioPinState : uint8_t
	{
		GPIO_PIN_RESET = 0,
		GPIO_PIN_SET
	};

	enum class GpioMode : uint8_t
	{
		GPIO_MODE_DISABLE,		   /*!< GPIO mode : disable input and output             */
		GPIO_MODE_INPUT,		   /*!< GPIO mode : input only                           */
		GPIO_MODE_OUTPUT,		   /*!< GPIO mode : output only mode                     */
		GPIO_MODE_OUTPUT_OD,	   /*!< GPIO mode : output only with open-drain mode     */
		GPIO_MODE_INPUT_OUTPUT_OD, /*!< GPIO mode : output and input with open-drain mode*/
		GPIO_MODE_INPUT_OUTPUT,	   /*!< GPIO mode : output and input mode                */
	};
	
	virtual ~HalGpioAbstract() = default;
	virtual bool Hal_Gpio_Init(GpioMode mode, bool pullup, bool pulldown, int initial_level) = 0;
	virtual bool Hal_Gpio_ReadPin() = 0;
	virtual void Hal_Gpio_WritePin(GpioPinState state) = 0;
	virtual void Hal_Gpio_TogglePin() = 0;
};

#endif /* HAL_ABSTRACT_INC_HALGPIOABSTRACT_H_ */
