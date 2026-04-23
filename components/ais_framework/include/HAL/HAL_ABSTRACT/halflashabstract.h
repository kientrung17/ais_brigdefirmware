/*
 * halflashabstract.h
 *
 *  Created on: May 24, 2024
 *      Author: canhpn2
 */

#ifndef SUBMODULE_VTJC2_FRAMEWORK_HAL_HAL_ABSTRACT_INC_HALFLASHABSTRACT_H_
#define SUBMODULE_VTJC2_FRAMEWORK_HAL_HAL_ABSTRACT_INC_HALFLASHABSTRACT_H_
#include <stdint.h>

class HalFlashAbstract
{
public:
	enum class FlashTypeStore: uint32_t
	{
		Type8 = 0,
		Type16,
		Type32,
		Type64,
	};
	virtual bool Hal_FLASH_Progarm(FlashTypeStore typeProgram, uint32_t address, uint64_t data) = 0;
	virtual uint32_t Hal_FLASH_GetError() = 0;
	virtual bool Hal_FLASH_Unlock() = 0;
	virtual bool Hal_FLASH_Lock() = 0;

	virtual void Flash_Earse_Sector(uint32_t sector) = 0;
	virtual bool Flash_Write(uint32_t adress, uint8_t* data, uint16_t lenght) = 0;
	virtual void Flash_Read(uint32_t adress, uint8_t* data, uint16_t lenght) = 0;
};

#endif /* SUBMODULE_VTJC2_FRAMEWORK_HAL_HAL_ABSTRACT_INC_HALFLASHABSTRACT_H_ */
