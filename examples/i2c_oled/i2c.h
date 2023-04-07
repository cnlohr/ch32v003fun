/*
 * Single-File-Header for I2C
 * 04-07-2023 E. Brombaugh
 */

#ifndef _I2C_H
#define _I2C_H

// I2C clock rate
#define I2C_CLKRATE 100000

// I2C Timeout count
#define TIMEOUT_MAX 100000

/*
 * init I2C
 */
void i2c_init(void)
{
	uint16_t tempreg;
	
	// Enable GPIOC and I2C
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;
	
	// PC1/PC2 are SDA/SCL, 50MHz Output PP CNF = 11: Mux OD, MODE = 11: Out 50MHz
	GPIOC->CFGLR &= ~(GPIO_CFGLR_MODE1 | GPIO_CFGLR_CNF1 |
						GPIO_CFGLR_MODE1 | GPIO_CFGLR_CNF1);
	GPIOC->CFGLR |= GPIO_CFGLR_CNF1_1 | GPIO_CFGLR_CNF1_0 |
					GPIO_CFGLR_MODE1_1 | GPIO_CFGLR_MODE1_0 |
					GPIO_CFGLR_CNF2_1 | GPIO_CFGLR_CNF2_0 |
					GPIO_CFGLR_MODE2_1 | GPIO_CFGLR_MODE2_0;
		
	// Reset I2C1 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;
	
	// set freq
	tempreg = I2C1->CTLR2;
	tempreg &= ~I2C_CTLR2_FREQ;
	tempreg |= (APB_CLOCK/1000000)&I2C_CTLR2_FREQ;
	I2C1->CTLR2 = tempreg;
	
	// Set clock config
	tempreg = 0;
#if (I2C_CLKRATE <= 100000)
	// standard mode good to 100kHz
	tempreg = (APB_CLOCK/(2*I2C_CLKRATE))&I2C_CKCFGR_CCR;
#else
	// fast mode not yet handled here
#endif
	I2C1->CKCFGR = tempreg;
	
	// Enable I2C
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// set ACK mode
	I2C1->CTLR1 |= I2C_CTLR1_ACK;
}

/*
 * error handler
 */
void i2c_error(void)
{
	// toggle SWRST bit
	I2C1->CTLR1 |= I2C_CTLR1_SWRST;
	I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;
}

// event codes we use
#define  I2C_EVENT_MASTER_MODE_SELECT ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */

/*
 * check for 32-bit event codes
 */
uint8_t i2c_chk_evt(uint32_t event_mask)
{
	/* read order matters here! STAR1 before STAR2!! */
	uint32_t status = I2C1->STAR1 | (I2C1->STAR2<<16);
	return (status & event_mask) == event_mask;
}

/*
 * common packet send
 */
void i2c_send(uint8_t addr, uint8_t *data, uint8_t sz)
{
	int32_t timeout;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for not busy\n\r");
		i2c_error();
		return;
	}

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for master mode\n\r");
		i2c_error();
		return;
	}
	
	// send 7-bit address + write flag
	I2C1->DATAR = addr<<1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for transmit mode\n\r");
		i2c_error();
		return;
	}

	// send data one byte at a time
	while(sz--)
	{
		// wait for TX Empty
		timeout = TIMEOUT_MAX;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
		if(timeout==-1)
		{
			printf("i2c_send :%d - timeout waiting for tx empty\n\r", sz);
			i2c_error();
			return;
		}
		
		// send command
		I2C1->DATAR = *data++;
	}

	// wait for tx complete
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
	if(timeout==-1)
	{
		printf("i2c_send - timeout waiting for byte transmitted\n\r");
		i2c_error();
		return;
	}

	// set STOP condition
	I2C1->CTLR1 |= I2C_CTLR1_STOP;	
}


#endif
