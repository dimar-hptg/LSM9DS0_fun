#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H


/**
	@brief The function initializes I2C interface
*/
void initI2C(void);


/**
	@brief Read register
*/
uint32_t readRegister(uint8_t deviceAddress, uint8_t RegisterAddr);

/**
	@brief	Write register
*/
uint32_t writeRegister(uint8_t deviceAddress, uint8_t RegisterAddr, uint8_t RegisterValue);

#endif // I2C_DRIVER_H

