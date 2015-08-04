#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "my_printf.h"


#define MY_I2C I2C1
#define MY_I2C_CLK RCC_APB1Periph_I2C1

static const int my_timeout = 1000000; 

/**
	@brief The function initializes I2C interface
*/
void initI2C(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStruct;

	// enable APB1 peripheral clock for I2C1
	RCC_APB1PeriphClockCmd(MY_I2C_CLK, ENABLE);

	// enable clock for SCL and SDA pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* setup SCL and SDA pins
	 * You can connect I2C1 to two different
	 * pairs of pins:
	 * 1. SCL on PB6 and SDA on PB7 
	 * 2. SCL on PB8 and SDA on PB9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // we are going to use PB6 and PB7
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			// set pins to alternate function
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// set GPIO speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// init GPIOB
	
	// Connect I2C1 pins to AF  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);	// SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA


	/* MY_I2C peripheral configuration */
	//I2C_DeInit(MY_I2C);

	// Init I2C struct
	I2C_StructInit(&I2C_InitStructure);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = 100000;

	/* Enable the I2C peripheral */
	I2C_Cmd(MY_I2C, ENABLE);  
	I2C_Init(MY_I2C, &I2C_InitStructure);
}


/**
	@brief Read register
*/
uint32_t readRegister(uint8_t deviceAddress, uint8_t RegisterAddr)
{
  uint32_t result = 0;

  int timeout = my_timeout;

  /*!< While the bus is busy */
  while(I2C_GetFlagStatus(MY_I2C, I2C_FLAG_BUSY))
  {
    if((timeout--) == 0)
	{
		my_printf("read reg timeout - 0\r\n");
		return 0;
    }
  }
  
  /* Start the config sequence */
  I2C_GenerateSTART(MY_I2C, ENABLE);

  /* Test on EV5 and clear it */
  timeout = my_timeout;
  while (!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 1\n");
		  return 0;
	  }
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(MY_I2C, deviceAddress, I2C_Direction_Transmitter);

  /* Test on EV6 and clear it */
  timeout = my_timeout;
  while (!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 2\n");
		  return 0;
	  }
  }

  /* Transmit the register address to be read */
  I2C_SendData(MY_I2C, RegisterAddr);

  /* Test on EV8 and clear it */
  timeout = my_timeout;
  while (I2C_GetFlagStatus(MY_I2C, I2C_FLAG_BTF) == RESET)
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 3\n");
		  return 0;
	  }
  }
  
  /*!< Send START condition a second time */  
  I2C_GenerateSTART(MY_I2C, ENABLE);
  
  /*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
  timeout = my_timeout;
  while(!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 4\n");
		  return 0;
	  }
  } 
  
  /*!< Send address for read */
  I2C_Send7bitAddress(MY_I2C, deviceAddress, I2C_Direction_Receiver);  
  
  /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
  timeout = my_timeout;
  while(I2C_GetFlagStatus(MY_I2C, I2C_FLAG_ADDR) == RESET)
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 5\n");
		  return 0;
	  }
  }     
  
  /*!< Disable Acknowledgment */
  I2C_AcknowledgeConfig(MY_I2C, DISABLE);   
  
  /* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
  (void)MY_I2C->SR2;
  
  /*!< Send STOP Condition */
  I2C_GenerateSTOP(MY_I2C, ENABLE);
  
  /* Wait for the byte to be received */
  timeout = my_timeout;
  while(I2C_GetFlagStatus(MY_I2C, I2C_FLAG_RXNE) == RESET)
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 6\n");
		  return 0;
	  }
  }
  
  /*!< Read the byte received */
  result = I2C_ReceiveData(MY_I2C);
  
  /* Wait to make sure that STOP flag has been cleared */
  timeout = my_timeout;
  while(MY_I2C->CR1 & I2C_CR1_STOP)
  {
	  if((timeout--) == 0)
	  {
		  my_printf("read reg timeout - 7\n");
		  return 0;
	  }
  }  
  
  /*!< Re-Enable Acknowledgment to be ready for another reception */
  I2C_AcknowledgeConfig(MY_I2C, ENABLE);  
  
  /* Clear AF flag for next communication */
  I2C_ClearFlag(MY_I2C, I2C_FLAG_AF); 
  
  /* Return the byte read */
  return result;
}


/**
	@brief	Write register
*/
uint32_t writeRegister(uint8_t deviceAddress, uint8_t RegisterAddr, uint8_t RegisterValue)
{
  uint32_t result = 0;
  int timeout;

  /*!< While the bus is busy */
  timeout = my_timeout;
  while(I2C_GetFlagStatus(MY_I2C, I2C_FLAG_BUSY))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("write reg timeout - 0\n");
		  return 0;
	  }
  }
  
  /* Start the config sequence */
  I2C_GenerateSTART(MY_I2C, ENABLE);

  /* Test on EV5 and clear it */
  timeout = my_timeout;
  while (!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("write reg timeout - 1\n");
		  return 0;
	  }
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(MY_I2C, deviceAddress, I2C_Direction_Transmitter);

  /* Test on EV6 and clear it */
  timeout = my_timeout;
  while (!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("write reg timeout - 2\n");
		  return 0;
	  }
  }

  /* Transmit the first address for write operation */
  I2C_SendData(MY_I2C, RegisterAddr);

  /* Test on EV8 and clear it */
  timeout = my_timeout;
  while (!I2C_CheckEvent(MY_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("write reg timeout - 3\n");
		  return 0;
	  }
  }
  
  /* Prepare the register value to be sent */
  I2C_SendData(MY_I2C, RegisterValue);
  
  /*!< Wait till all data have been physically transferred on the bus */
  timeout = my_timeout;
  while(!I2C_GetFlagStatus(MY_I2C, I2C_FLAG_BTF))
  {
	  if((timeout--) == 0)
	  {
		  my_printf("write reg timeout - 4\n");
		  return 0;
	  }
  }
  
  /* End the configuration sequence */
  I2C_GenerateSTOP(MY_I2C, ENABLE);	
  
  /* Return the verifying value: 0 (Passed) or 1 (Failed) */
  return result;  
}

