/*
 * hali2cabstract.h
 *
 *  Created on: May 13, 2024
 *      Author: canhpn2
 */

#ifndef HAL_ABSTRACT_INC_HALI2CABSTRACT_H_
#define HAL_ABSTRACT_INC_HALI2CABSTRACT_H_
#include <stdint.h>

class HalI2cAbstract
{
public:
	enum class PORT_I2C
	{
		I2C_NUM_0 = 0,
		I2C_NUM_1
	};
	virtual ~HalI2cAbstract() = default;
	virtual bool Hal_I2C_Master_Transmit(uint8_t *data, uint16_t adrress, uint16_t size, uint32_t timeout) = 0;
	virtual bool Hal_I2C_Master_Receive(uint8_t *data, uint16_t adrress, uint16_t size, uint32_t timeout) = 0;
};

#endif /* HAL_ABSTRACT_INC_HALI2CABSTRACT_H_ */
