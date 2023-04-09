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

// uncomment this to enable IRQ-driven operation
#define I2C_IRQ

#ifdef I2C_IRQ
// some stuff that IRQ mode needs
volatile uint8_t i2c_send_buffer[64], *i2c_send_ptr, i2c_send_sz, i2c_irq_state;

// uncomment this to enable time diags in IRQ
//#define IRQ_DIAG
#endif

/*
 * init just I2C
 */
void i2c_setup(void)
{
	uint16_t tempreg;
	
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

#ifdef I2C_IRQ
	// enable IRQ driven operation
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	
	// initialize the state
	i2c_irq_state = 0;
#endif
	
	// Enable I2C
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// set ACK mode
	I2C1->CTLR1 |= I2C_CTLR1_ACK;
}

/*
 * init I2C and GPIO
 */
void i2c_init(void)
{
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
	
#ifdef IRQ_DIAG
	// GPIO diags on PC3/PC4
	GPIOC->CFGLR &= ~(0xf<<(4*3));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3);
	GPIOC->BSHR = (1<<(16+3));
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
	GPIOC->BSHR = (1<<(16+4));
#endif

	// load I2C regs
	i2c_setup();
}

/*
 * error descriptions
 */
char *errstr[] =
{
	"not busy",
	"master mode",
	"transmit mode",
	"tx empty",
	"transmit complete",
};

/*
 * error handler
 */
uint8_t i2c_error(uint8_t err)
{
	// report error
	printf("i2c_error - timeout waiting for %s\n\r", errstr[err]);
	
	// reset & initialize I2C
	i2c_setup();

	return 1;
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

#ifdef I2C_IRQ
/*
 * packet send for IRQ-driven operation
 */
uint8_t i2c_send(uint8_t addr, uint8_t *data, uint8_t sz)
{
	int32_t timeout;
	
#ifdef IRQ_DIAG
	GPIOC->BSHR = (1<<(3));
#endif
	
	// error out if buffer under/overflow
	if((sz > sizeof(i2c_send_buffer)) || !sz)
		return 2;
	
	// wait for previous packet to finish
	while(i2c_irq_state);
	
#ifdef IRQ_DIAG
	GPIOC->BSHR = (1<<(16+3));
	GPIOC->BSHR = (1<<(4));
#endif
	
	// init buffer for sending
	i2c_send_sz = sz;
	i2c_send_ptr = i2c_send_buffer;
	memcpy((uint8_t *)i2c_send_buffer, data, sz);
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
		return i2c_error(0);

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;

	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
		return i2c_error(1);
	
	// send 7-bit address + write flag
	I2C1->DATAR = addr<<1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
		return i2c_error(2);

	// Enable TXE interrupt
	I2C1->CTLR2 |= I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITEVTEN;
	i2c_irq_state = 1;

#ifdef IRQ_DIAG
	GPIOC->BSHR = (1<<(16+4));
#endif
	
	// exit
	return 0;
}

/*
 * IRQ handler for I2C events
 */
void I2C1_EV_IRQHandler(void) __attribute__((interrupt));
void I2C1_EV_IRQHandler(void)
{
	uint16_t STAR1, STAR2 __attribute__((unused));
	
#ifdef IRQ_DIAG
	GPIOC->BSHR = (1<<(4));
#endif

	// read status, clear any events
	STAR1 = I2C1->STAR1;
	STAR2 = I2C1->STAR2;
	
	/* check for TXE */
	if(STAR1 & I2C_STAR1_TXE)
	{
		/* check for remaining data */
		if(i2c_send_sz--)
			I2C1->DATAR = *i2c_send_ptr++;

		/* was that the last byte? */
		if(!i2c_send_sz)
		{
			// disable TXE interrupt
			I2C1->CTLR2 &= ~(I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITEVTEN);
			
			// reset IRQ state
			i2c_irq_state = 0;
			
			// wait for tx complete
			while(!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED));

			// set STOP condition
			I2C1->CTLR1 |= I2C_CTLR1_STOP;
		}
	}

#ifdef IRQ_DIAG
	GPIOC->BSHR = (1<<(16+4));
#endif
}
#else
/*
 * packet send for polled operation
 */
uint8_t i2c_send(uint8_t addr, uint8_t *data, uint8_t sz)
{
	int32_t timeout;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
		return i2c_error(0);

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
		return i2c_error(1);
	
	// send 7-bit address + write flag
	I2C1->DATAR = addr<<1;

	// wait for transmit condition
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
	if(timeout==-1)
		return i2c_error(2);

	// send data one byte at a time
	while(sz--)
	{
		// wait for TX Empty
		timeout = TIMEOUT_MAX;
		while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
		if(timeout==-1)
			return i2c_error(3);
		
		// send command
		I2C1->DATAR = *data++;
	}

	// wait for tx complete
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
	if(timeout==-1)
		return i2c_error(4);

	// set STOP condition
	I2C1->CTLR1 |= I2C_CTLR1_STOP;
	
	// we're happy
	return 0;
}
#endif

#endif
