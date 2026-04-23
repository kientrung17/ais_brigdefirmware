#ifndef ESPIDF_I2C_HAL_H_
#define ESPIDF_I2C_HAL_H_

#include "hali2cabstract.h"
#include "driver/i2c.h"

class EspidfI2cHal : public HalI2cAbstract {
public:
    EspidfI2cHal();
    ~EspidfI2cHal();

    bool Init(PORT_I2C i2c_port, int sda, int scl, int freq);
    bool Hal_I2C_Master_Transmit(uint8_t *data, uint16_t address, uint16_t size, uint32_t timeout) override;
    bool Hal_I2C_Master_Receive(uint8_t *data, uint16_t address, uint16_t size, uint32_t timeout) override;

private:
    i2c_port_t mPortI2C {i2c_port_t::I2C_NUM_0};
};

#endif
