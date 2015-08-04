#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "my_printf.h"
#include "lsm9ds0_regs.h"

static const uint8_t xmAddress = 0x3D;
static const uint8_t gAddress = 0xD5;

/**
	@bried	Write gyro register.
*/
void gWriteByte(uint8_t address, uint8_t data)
{
	writeRegister(gAddress, address, data);
}

/**
	@bried	Write accelerometer register.
*/
void xmWriteByte(uint8_t address, uint8_t data)
{
	return writeRegister(xmAddress, address, data);
}

/**
	@bried	Read gyro register.
*/
uint8_t gReadByte(uint8_t address)
{
	return readRegister(gAddress, address);
}


/**
	@bried	Read accelerometer register.
*/
uint8_t xmReadByte(uint8_t address)
{
	return readRegister(xmAddress, address);
}

/**
	@brief	Init gyro.
*/
void initGyro()
{
	gWriteByte(CTRL_REG1_G, 0x0F); // Normal mode, enable all axes
	
	gWriteByte(CTRL_REG2_G, 0x00); // Normal mode, high cutoff frequency
	
	gWriteByte(CTRL_REG3_G, 0x00); // Disable interrupts
	
	gWriteByte(CTRL_REG4_G, 0x00); // Set scale to 245 dps
	
	gWriteByte(CTRL_REG5_G, 0x00); // Disable fifo mode
}

/**
	@brief	Init accelerometer.
*/
void initAccel()
{
	xmWriteByte(CTRL_REG0_XM, 0x00); // Disable fifo mode
	
	xmWriteByte(CTRL_REG1_XM, 0x57); // 100Hz data rate, x/y/z all enabled
	
	xmWriteByte(CTRL_REG2_XM, 0x00); // Set scale to 2g
	
	xmWriteByte(CTRL_REG3_XM, 0x00); 
}


/**
	@brief	Init magnetometer.
*/
void initMag()

{	
	xmWriteByte(CTRL_REG5_XM, 0x14); // Mag data rate - 100 Hz
	
	xmWriteByte(CTRL_REG6_XM, 0x00); // Mag scale to +/- 2GS
	
	xmWriteByte(CTRL_REG7_XM, 0x00); // Continuous conversion mode
	
	xmWriteByte(CTRL_REG4_XM, 0x00); // 
	
	xmWriteByte(INT_CTRL_REG_M, 0x00); // Disable interrupts for mag
}

/**
	@brief	Read gyro.
			The return values are in dps units multiplied by factor of 100.
*/
void readGyro(int32_t* gx, int32_t* gy, int32_t* gz)
{
	uint8_t temp[6]; // We'll read six bytes from the gyro into temp
	float gScale = 245.0 / 32768.0; // 245dps/ADC ticks based on 2bit value
	int16_t x, y, z;
	float fx, fy, fz;
	const float factor = 100.0;

	temp[0] = gReadByte(OUT_X_L_G);
	temp[1] = gReadByte(OUT_X_H_G);
	temp[2] = gReadByte(OUT_Y_L_G);
	temp[3] = gReadByte(OUT_Y_H_G);
	temp[4] = gReadByte(OUT_Z_L_G);
	temp[5] = gReadByte(OUT_Z_H_G);

	x = (temp[1] << 8) | temp[0];
	y = (temp[3] << 8) | temp[2];
	z = (temp[5] << 8) | temp[4];

	// Convert the values
	fx = x * gScale * factor;
	fy = y * gScale * factor;
	fz = z * gScale * factor;

	*gx = (int)fx;
	*gy = (int)fy;
	*gz = (int)fz;
}


/**
	@brief	Read accelerometer.
			The return values are in g units multiplied by factor of 100.
*/
void readAccel(int32_t* ax, int32_t* ay, int32_t* az)
{
	
	uint8_t temp[6]; // We'll read six bytes from the gyro into temp
	int16_t x, y, z;
	float aScale = 2.0 / 32768.0; // Scale factor for 2g. Convert from ADC raw value to g
	float fx, fy, fz;
	const float factor = 100.0;

	temp[0] = xmReadByte(OUT_X_L_A);
	temp[1] = xmReadByte(OUT_X_H_A);
	temp[2] = xmReadByte(OUT_Y_L_A);
	temp[3] = xmReadByte(OUT_Y_H_A);
	temp[4] = xmReadByte(OUT_Z_L_A);
	temp[5] = xmReadByte(OUT_Z_H_A);

	x = (temp[1] << 8) | temp[0];
	y = (temp[3] << 8) | temp[2];
	z = (temp[5] << 8) | temp[4];

	// Convert the values
	fx = x * aScale * factor;
	fy = y * aScale * factor;
	fz = z * aScale * factor;

	*ax = (int)fx;
	*ay = (int)fy;
	*az = (int)fz;
}

/**
	@brief	Read mag values
			The return values are in Gs units multiplied by factor of 100.
*/
void readMag(int32_t* mx, int32_t* my, int32_t* mz)
{
	uint8_t temp[6]; // We'll read six bytes from the gyro into temp
	float mScale = 2.0 / 32768.0; // 2Gs/(ADC tick) based on 2-bit value	
	float fx, fy, fz;
	const float factor = 100.0;
	int16_t x, y, z;

	temp[0] = xmReadByte(OUT_X_L_M);
	temp[1] = xmReadByte(OUT_X_H_M);
	temp[2] = xmReadByte(OUT_Y_L_M);
	temp[3] = xmReadByte(OUT_Y_H_M);
	temp[4] = xmReadByte(OUT_Z_L_M);
	temp[5] = xmReadByte(OUT_Z_H_M);

	x = (temp[1] << 8) | temp[0];
	y = (temp[3] << 8) | temp[2];
	z = (temp[5] << 8) | temp[4];

	// Convert the values
	fx = x * mScale * factor;
	fy = y * mScale * factor;
	fz = z * mScale * factor;

	*mx = (int)fx;
	*my = (int)fy;
	*mz = (int)fz;
}

