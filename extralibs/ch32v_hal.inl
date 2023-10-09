//
// CH32V Hardware Abstraction layer
// written by Larry Bank
// bitbank@pobox.com
//
// Copyright 2023 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "ch32v_hal.h"

//#define BITBANG

#ifdef BITBANG
uint8_t u8SDA_Pin, u8SCL_Pin;
int iDelay = 1;
#endif
uint32_t SystemCoreClock = 48000000;
uint32_t u32TickMicros = 48000000 / 8000;

// Put CPU into standby mode for a multiple of 82ms tick increments
// max ticks value is 63
void Standby82ms(uint8_t iTicks)
{
uint32_t tmp = 0;

    // init external interrupts
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

//    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
//    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);
    EXTI->INTENR &= ~EXTI_Line9;
    EXTI->EVENR &= ~EXTI_Line9;
    tmp = (uint32_t)EXTI_BASE;
    tmp += EXTI_Mode_Event;
    *(__IO uint32_t *)tmp |= EXTI_Line9;
    EXTI->RTENR &= ~EXTI_Line9;
    EXTI->FTENR &= ~EXTI_Line9;
    tmp = (uint32_t)EXTI_BASE;
    tmp += EXTI_Trigger_Falling;
    *(__IO uint32_t *)tmp |= EXTI_Line9;
    
    // Init GPIOs
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
    RCC->APB2PCENR |= (RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC->APB1PCENR |= RCC_APB1Periph_PWR;

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//    GPIO_Init(GPIOC, &GPIO_InitStructure);
//    GPIO_Init(GPIOD, &GPIO_InitStructure);
// Set all pins to input pull-down mode
    for (int i=0; i<8; i++) {
        GPIOA->CFGLR &= ~(0xf << (4 * i)); // unset all flags
        GPIOC->CFGLR &= ~(0xf << (4 * i));
        GPIOD->CFGLR &= ~(0xf << (4 * i));
        GPIOA->CFGLR |= ((GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD) << (4*i)); // set as input pull-down
        GPIOC->CFGLR |= ((GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD) << (4*i));
        GPIOD->CFGLR |= ((GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD) << (4*i));
    }
    // init wake up timer and enter standby mode
//    RCC_LSICmd(ENABLE);
      RCC->RSTSCKR |= 1; // enable the LSI (low speed internal oscillator)
//    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
      while ((RCC->CTLR & 2) == 0) {}; // wait for LSI to be ready
//    PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
      tmp = PWR->AWUPSC & AWUPSC_MASK;
      tmp |= PWR_AWU_Prescaler_10240;
      PWR->AWUPSC = tmp;
//    PWR_AWU_SetWindowValue(iTicks);
      tmp = (PWR->AWUWR & AWUWR_MASK) | iTicks;
      PWR->AWUWR = tmp;
//    PWR_AutoWakeUpCmd(ENABLE);
      PWR->AWUCSR |= 2;
//    PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);
      PWR->CTLR &= CTLR_DS_MASK;
      PWR->CTLR |= PWR_CTLR_PDDS;
      NVIC->SCTLR |= 4;
      __WFE();
      NVIC->SCTLR &= ~(4);

// we've woken up, disable all GPIO
    RCC->APB2PRSTR &= ~(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD);
    //GPIO_DeInit(GPIOA);
    //GPIO_DeInit(GPIOC);
    //GPIO_DeInit(GPIOD);
} /* Standby82ms() */

void SetClock(uint32_t u32Clock)
{
        uint32_t u32Div = 0;

if (u32Clock > 24000000)
        SystemCoreClock = 48000000;
else if (u32Clock > 12000000) {
        SystemCoreClock = 24000000;
        u32Div = RCC_HPRE_DIV1;
}
else if (u32Clock > 8000000) {
        SystemCoreClock = 12000000;
        u32Div = RCC_HPRE_DIV2;
}
else if (u32Clock > 6000000) {
        SystemCoreClock = 8000000;
        u32Div = RCC_HPRE_DIV3;
}
else if (u32Clock > 4800000) {
        SystemCoreClock = 6000000;
        u32Div = RCC_HPRE_DIV4;
}
else if (u32Clock > 4000000) {
        SystemCoreClock = 4800000;
        u32Div = RCC_HPRE_DIV5;
}
else if (u32Clock > 3428571) {
        SystemCoreClock = 4000000;
        u32Div = RCC_HPRE_DIV6;
}
else if (u32Clock >= 3000000) {
        SystemCoreClock = 3428571;
        u32Div = RCC_HPRE_DIV7;
}
else if (u32Clock > 1500000) {
        SystemCoreClock = 3000000;
        u32Div = RCC_HPRE_DIV8;
}
else if (u32Clock > 750000) {
        SystemCoreClock = 1500000;
        u32Div = RCC_HPRE_DIV16;
}
else if (u32Clock > 375000) {
        SystemCoreClock = 750000;
        u32Div = RCC_HPRE_DIV32;
}
else if (u32Clock > 187500) {
        SystemCoreClock = 375000;
        u32Div = RCC_HPRE_DIV64;
}
else {
        SystemCoreClock = 187500; // slowest setting for now
        u32Div = RCC_HPRE_DIV128;
}
switch (SystemCoreClock) {
case 48000000: // special case - needs PLL
    /* Flash 0 wait state */
    FLASH->ACTLR &= (uint32_t)((uint32_t)~FLASH_ACTLR_LATENCY);
    FLASH->ACTLR |= (uint32_t)FLASH_ACTLR_LATENCY_1;

    /* HCLK = SYSCLK = APB1 */
    RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1;

    /* PLL configuration: PLLCLK = HSI * 2 = 48 MHz */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_PLLSRC));
    RCC->CFGR0 |= (uint32_t)(RCC_PLLSRC_HSI_Mul2);

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }
        break;
default: // simpler - just use the RC clock with a divider
    /* Flash 0 wait state */
    FLASH->ACTLR &= (uint32_t)((uint32_t)~FLASH_ACTLR_LATENCY);
    FLASH->ACTLR |= (SystemCoreClock >= 24000000) ? (uint32_t)FLASH_ACTLR_LATENCY_1 : (uint32_t)FLASH_ACTLR_LATENCY_0;

    /* HCLK = SYSCLK = APB1 */
    RCC->CFGR0 |= u32Div;
        break;
} // switch on clock
    UpdateDelay();
} /* SetClock() */

void UpdateDelay(void)
{
    if (SystemCoreClock == 48000000)
       u32TickMicros = udiv32(SystemCoreClock, 8000);
    else
       u32TickMicros = udiv32(SystemCoreClock, 4000); // PLL disabled
}
void delay(uint32_t u32)
{
    uint32_t targend = SysTick->CNT + (u32 * u32TickMicros);
    while( ((int32_t)( SysTick->CNT - targend )) < 0 );
}
// Arduino-like API defines and function wrappers for WCH MCUs

void pinMode(uint8_t u8Pin, int iMode)
{
GPIO_TypeDef *pGPIO;

    if (u8Pin < 0xa0 || u8Pin > 0xdf) return; // invalid pin number

    switch (u8Pin & 0xf0) {
    case 0xa0:
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
	pGPIO = GPIOA;
    	break;
    case 0xc0:
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	pGPIO = GPIOC;
    	break;
    case 0xd0:
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	pGPIO = GPIOD;
    	break;
    }
    u8Pin &= 0xf; // isolate the pin from this port
    pGPIO->CFGLR &= ~(0xf << (4 * u8Pin)); // unset all flags

    switch (iMode) {
	case OUTPUT:
		pGPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4*u8Pin);
		break;
	case INPUT:
		pGPIO->CFGLR |= (GPIO_CNF_IN_FLOATING << (4*u8Pin));
		break;
	case INPUT_PULLUP:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4*u8Pin));
		pGPIO->BSHR = (1 << u8Pin);
		break;
	case INPUT_PULLDOWN:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4 * u8Pin));
		pGPIO->BCR = (1 << u8Pin);
		break;
    } // switch on iMode
} /* pinMode() */

uint8_t digitalRead(uint8_t u8Pin)
{
    uint32_t u32GPIO = 1 << (u8Pin & 0xf);
    uint32_t u32Value = 0;
    switch (u8Pin & 0xf0) {
    case 0xa0:
    	u32Value = GPIOA->INDR & u32GPIO;
    	break;
    case 0xc0:
    	u32Value = GPIOC->INDR & u32GPIO;
    	break;
    case 0xd0:
    	u32Value = GPIOD->INDR & u32GPIO;
    	break;
    }
    return (u32Value != 0);
} /* digitalRead() */

void digitalWrite(uint8_t u8Pin, uint8_t u8Value)
{
	uint32_t u32Value = 1 << (u8Pin & 0xf); // turn on bit
	if (!u8Value)
		u32Value <<= 16; // turn off bit 

	switch (u8Pin & 0xf0) {
	case 0xa0:
		GPIOA->BSHR = u32Value;
		break;
	case 0xc0:
                GPIOC->BSHR = u32Value;
		break;
	case 0xd0:
                GPIOD->BSHR = u32Value;
		break;
	}
} /* digitalWrite() */

#ifdef BITBANG
uint8_t SDA_READ(void)
{
	return digitalRead(u8SDA_Pin);
}
void SDA_HIGH(void)
{
	pinMode(u8SDA_Pin, INPUT);
}
void SDA_LOW(void)
{
	pinMode(u8SDA_Pin, OUTPUT);
	digitalWrite(u8SDA_Pin, 0);
}
void SCL_HIGH(void)
{
	pinMode(u8SCL_Pin, INPUT);
}
void SCL_LOW(void)
{
	pinMode(u8SCL_Pin, OUTPUT);
	digitalWrite(u8SCL_Pin, 0);
}
void I2CSetSpeed(int iSpeed)
{
	if (iSpeed >= 400000) iDelay = 1;
	else if (iSpeed >= 100000) iDelay = 10;
	else iDelay = 20;
}
void I2CInit(uint8_t u8SDA, uint8_t u8SCL, int iSpeed)
{
	u8SDA_Pin = u8SDA;
	u8SCL_Pin = u8SCL;
	if (iSpeed >= 400000) iDelay = 1;
	else if (iSpeed >= 100000) iDelay = 10;
	else iDelay = 20;
} /* I2CInit() */

void my_sleep_us(int iDelay)
{
	Delay_Us(iDelay);
}
// Transmit a byte and read the ack bit
// if we get a NACK (negative acknowledge) return 0
// otherwise return 1 for success
//

int i2cByteOut(uint8_t b)
{
uint8_t i, ack;

for (i=0; i<8; i++)
{
//    my_sleep_us(iDelay);
    if (b & 0x80)
      SDA_HIGH(); // set data line to 1
    else
      SDA_LOW(); // set data line to 0
    b <<= 1;
//    my_sleep_us(iDelay);
    SCL_HIGH(); // clock high (slave latches data)
    my_sleep_us(iDelay);
    SCL_LOW(); // clock low
    my_sleep_us(iDelay);
} // for i
//my_sleep_us(iDelay);
// read ack bit
SDA_HIGH(); // set data line for reading
//my_sleep_us(iDelay);
SCL_HIGH(); // clock line high
my_sleep_us(iDelay); // DEBUG - delay/2
ack = SDA_READ();
//my_sleep_us(iDelay);
SCL_LOW(); // clock low
my_sleep_us(iDelay); // DEBUG - delay/2
SDA_LOW(); // data low
return (ack == 0); // a low ACK bit means success
} /* i2cByteOut() */

//
// Receive a byte and read the ack bit
// if we get a NACK (negative acknowledge) return 0
// otherwise return 1 for success
//
uint8_t i2cByteIn(uint8_t bLast)
{
uint8_t i;
uint8_t b = 0;

     SDA_HIGH(); // set data line as input
     for (i=0; i<8; i++)
     {
         my_sleep_us(iDelay); // wait for data to settle
         SCL_HIGH(); // clock high (slave latches data)
         my_sleep_us(iDelay);
         b <<= 1;
         if (SDA_READ() != 0) // read the data bit
           b |= 1; // set data bit
         SCL_LOW(); // clock low
     } // for i
     if (bLast)
        SDA_HIGH(); // last byte sends a NACK
     else
        SDA_LOW();
//     my_sleep_us(iDelay);
     SCL_HIGH(); // clock high
     my_sleep_us(iDelay);
     SCL_LOW(); // clock low to send ack
     my_sleep_us(iDelay);
//     SDA_HIGH();
     SDA_LOW(); // data low
  return b;
} /* i2cByteIn() */
//
// Send I2C STOP condition
//
void i2cEnd(void)
{
   SDA_LOW(); // data line low
   my_sleep_us(iDelay);
   SCL_HIGH(); // clock high
   my_sleep_us(iDelay);
   SDA_HIGH(); // data high
   my_sleep_us(iDelay);
} /* i2cEnd() */

int i2cBegin(uint8_t addr, uint8_t bRead)
{
   int rc;
//   SCL_HIGH();
//   my_sleep_us(iDelay);
   SDA_LOW(); // data line low first
   my_sleep_us(iDelay);
   SCL_LOW(); // then clock line low is a START signal
   addr <<= 1;
   if (bRead)
      addr++; // set read bit
   rc = i2cByteOut(addr); // send the slave address and R/W bit
   return rc;
} /* i2cBegin() */

void I2CWrite(uint8_t addr, uint8_t *pData, int iLen)
{
uint8_t b;
int rc;

   i2cBegin(addr, 0);
   rc = 1;
   while (iLen && rc == 1)
   {
      b = *pData++;
      rc = i2cByteOut(b);
      if (rc == 1) // success
      {
         iLen--;
      }
   } // for each byte
   i2cEnd();
//return (rc == 1) ? (iOldLen - iLen) : 0; // 0 indicates bad ack from sending a byte
} /* I2CWrite() */

int I2CRead(uint8_t addr, uint8_t *pData, int iLen)
{
   i2cBegin(addr, 1);
   while (iLen--)
   {
      *pData++ = i2cByteIn(iLen == 0);
   } // for each byte
   i2cEnd();
   return 1;
} /* I2CRead() */

int I2CTest(uint8_t addr)
{
int response = 0;

   if (i2cBegin(addr, 0)) // try to write to the given address
   {
      response = 1;
   }
   i2cEnd();
return response;
} /* I2CTest() */

#else // hardware I2C

void I2CSetSpeed(int iSpeed)
{
#ifdef FUTURE
    I2C_InitTypeDef I2C_InitTSturcture={0};

    I2C_InitTSturcture.I2C_ClockSpeed = iSpeed;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = 0x02; //address; sender's unimportant address
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );
#endif
} /* I2CSetSpeed() */

void I2CInit(uint8_t iSDA, uint8_t iSCL, int iSpeed)
{
    uint16_t tmpreg = 0, freqrange = 0;
    uint16_t result = 0x04;
    uint32_t pclk1 = 8000000;

    (void)iSDA; (void)iSCL; // Use C1/C2

        // Enable GPIOC and I2C
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
        RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

        // PC1 is SDA, 10MHz Output, alt func
        GPIOC->CFGLR &= ~(0xf<<(4*1));
        GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*1);

        // PC2 is SCL, 10MHz Output, alt func
        GPIOC->CFGLR &= ~(0xf<<(4*2));
        GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*2);

// reset I2C registers
        RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
        RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;
    I2C1->CTLR1 |= I2C_CTLR1_SWRST;
    I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

    tmpreg = I2C1->CTLR2;
    tmpreg &= CTLR2_FREQ_Reset;
//    RCC_GetClocksFreq(&rcc_clocks);
    pclk1 = SystemCoreClock; //rcc_clocks.PCLK1_Frequency;
    freqrange = (uint16_t)udiv32(pclk1 , 1000000);
    tmpreg |= freqrange;
    I2C1->CTLR2 = tmpreg;

    I2C1->CTLR1 &= CTLR1_PE_Reset;
    tmpreg = 0;

    if(iSpeed <= 100000)
    {
        result = (uint16_t)udiv32(pclk1, (iSpeed << 1));
        if(result < 0x04)
        {
            result = 0x04;
        }

        tmpreg |= result;
    }
    else
    {
//        if(I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_2)
//        {
//            result = (uint16_t)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed * 3));
//        }
//        else
//        {
            result = (uint16_t)udiv32(pclk1, iSpeed * 25);
            result |= I2C_DutyCycle_16_9;
//        }

        if((result & CKCFGR_CCR_Set) == 0)
        {
            result |= (uint16_t)0x0001;
        }

        tmpreg |= (uint16_t)(result | CKCFGR_FS_Set);
    }

    I2C1->CKCFGR = tmpreg;

    tmpreg = I2C1->CTLR1;
    tmpreg &= I2C_CTLR1_CLEAR_Mask;
    tmpreg |= (uint16_t)(uint32_t)(I2C_Mode_I2C | I2C_Ack_Enable);
    I2C1->CTLR1 = tmpreg;

    I2C1->OADDR1 = 0x2 | I2C_AcknowledgedAddress_7bit;
    I2C1->CTLR1 |= I2C_CTLR1_PE; // enable I2C

} /* I2CInit() */

#define  I2C_EVENT_MASTER_MODE_SELECT ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */
#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED              ((uint32_t)0x00030002) /* BUSY, MSL and ADDR flags */
/* I2C FLAG mask */
#define I2C_FLAG_Mask                ((uint32_t)0x00FFFFFF)
#define CTLR1_ACK_Reset          ((uint16_t)0xFBFF)
#define TIMEOUT 100000

uint8_t I2C_CheckEvent(uint32_t event_mask)
{
  uint32_t status = I2C1->STAR1 | (I2C1->STAR2 <<16);
  return (status & event_mask) == event_mask;
}
uint8_t I2C_GetFlagStatus(uint32_t I2C_FLAG)
{
    uint8_t bitstatus = 0;
    uint32_t i2creg = 0, i2cxbase = 0;

    i2cxbase = (uint32_t)I2C1;
    i2creg = I2C_FLAG >> 28;
    I2C_FLAG &= I2C_FLAG_Mask;

    if(i2creg != 0)
    {
        i2cxbase += 0x14;
    }
    else
    {
        I2C_FLAG = (uint32_t)(I2C_FLAG >> 16);
        i2cxbase += 0x18;
    }

    if(((*(uint32_t *)i2cxbase) & I2C_FLAG) != 0)
    {
        bitstatus = 1;
    }
    else
    {
        bitstatus = 0;
    }

    return bitstatus;
}

void I2C_ClearFlag(uint32_t I2C_FLAG)
{
    uint32_t flagpos = 0;

    flagpos = I2C_FLAG & I2C_FLAG_Mask;
    I2C1->STAR1 = (uint16_t)~flagpos;
}

//
// Returns 0 for timeout error
// returns 1 for success
//
int I2CRead(uint8_t u8Addr, uint8_t *pData, int iLen)
{
int iTimeout = 0;
uint16_t u16;

//    while( I2C_GetFlagStatus( I2C_FLAG_BUSY ) != 0) {};
    I2C1->CTLR1 |= I2C_CTLR1_START;
    while( !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C1->DATAR = (u8Addr << 1) | 1; // send 7-bit address, read flag = 1

    while(/*iTimeout < TIMEOUT &&*/ !I2C_CheckEvent( I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) ) {
    	iTimeout++;
    }
    //if (iTimeout >= TIMEOUT) return 0; // error
    u16 = I2C1->STAR2; // clear the status register
    I2C1->CTLR1 |= CTLR1_ACK_Set; // enable acknowledge
    iTimeout = 0;
    while(iLen && iTimeout < TIMEOUT)
    {
//        if( I2C_GetFlagStatus( I2C_FLAG_RXNE ) !=  0 )
        if (I2C1->STAR1 & I2C_STAR1_RXNE)
//	if (I2C_GetFlagStatus( I2C_FLAG_RXNE) == 0) {
//		I2C1->CTLR1 &= CTLR1_ACK_Reset;
//		iTimeout++;
//	} else {
	{
            iTimeout = 0;
            *pData++ = (uint8_t)I2C1->DATAR;
            iLen--;
            if (iLen == 1) { // last byte
               I2C1->CTLR1 &= CTLR1_ACK_Reset; // disable acknowledge
               I2C1->CTLR1 |= I2C_CTLR1_STOP; // send stop signal
            }
        } else {
	    iTimeout++;
	}
    } // while

//    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return (iLen == 0);
} /* I2CRead() */

void I2CWrite(uint8_t u8Addr, uint8_t *pData, int iLen)
{
    I2C1->CTLR1 |= I2C_CTLR1_START;
    while( !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C1->DATAR = (u8Addr << 1) | 0; // write flag = 0

    while( !I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    while(iLen)
    {
        if( I2C_GetFlagStatus( I2C_FLAG_TXE ) !=  0 )
        {
            I2C1->DATAR = pData[0];
            pData++;
            iLen--;
        }
    }

    while( !I2C_CheckEvent( I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    I2C1->CTLR1 |= I2C_CTLR1_STOP;

} /* I2CWrite() */

int I2CTest(uint8_t u8Addr)
{
	int iTimeout = 0;

	I2C_ClearFlag(I2C_FLAG_AF);
    I2C1->CTLR1 |= I2C_CTLR1_START;
    while(iTimeout < TIMEOUT && !I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT ) ) {
    	iTimeout++;
    }
    if (iTimeout >= TIMEOUT) return 0; // no pull-ups, open bus

    I2C1->DATAR = (u8Addr << 1) | 0; // transmit direction

    while(iTimeout < TIMEOUT && !I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ) {
    	iTimeout++;
    }
    if (iTimeout >= TIMEOUT) return 0; // no device at that address; the MTMS flag will never get set

    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    // check ACK failure flag
    return (I2C_GetFlagStatus(I2C_FLAG_AF) == 0); // 0 = fail, 1 = succeed

} /* I2CTest() */
#endif // !BITBANG

//
// Read N bytes starting at a specific I2C internal register
// returns 1 for success, 0 for error
//
void I2CReadRegister(uint8_t iAddr, uint8_t u8Register, uint8_t *pData, int iLen)
{
  I2CWrite(iAddr, &u8Register, 1);
  I2CRead(iAddr, pData, iLen);
} /* I2CReadRegister() */

void SPI_begin(int iSpeed, int iMode)
{
uint32_t u32Prescaler = 0;

    if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/2))
        u32Prescaler = SPI_BaudRatePrescaler_2;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/4))
        u32Prescaler = SPI_BaudRatePrescaler_4;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/8))
        u32Prescaler = SPI_BaudRatePrescaler_8;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/16))
        u32Prescaler = SPI_BaudRatePrescaler_16;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/32))
        u32Prescaler = SPI_BaudRatePrescaler_32;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/64))
        u32Prescaler = SPI_BaudRatePrescaler_64;
    else if (iSpeed >= (FUNCONF_SYSTEM_CORE_CLOCK/128))
        u32Prescaler = SPI_BaudRatePrescaler_128;
    else u32Prescaler = SPI_BaudRatePrescaler_256;

        // Enable GPIOC and SPI
        RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

	// PC5 is SCK, 10MHz Output, alt func, p-p
        GPIOC->CFGLR &= ~(0xf<<(4*5));
        GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

        // PC6 is MOSI, 10MHz Output, alt func, p-p
        GPIOC->CFGLR &= ~(0xf<<(4*6));
        GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);

        // Configure SPI 
        SPI1->CTLR1 =
                SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_8b |
                SPI_Mode_Master | SPI_Direction_1Line_Tx |
                u32Prescaler;

	// Enable DMA on SPI
	SPI1->CTLR2 |= SPI_I2S_DMAReq_Tx;

        // enable SPI port
        SPI1->CTLR1 |= CTLR1_SPE_Set;
	
    //SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE); // enable DMA on transmit

    //SPI_Cmd( SPI1, ENABLE );
} /* SPI_begin() */

// polling write
void SPI_write(uint8_t *pData, int iLen)
{
int i = 0;

    while (i < iLen) {
        while(!(SPI1->STATR & SPI_STATR_TXE)); // wait for TXE
        SPI1->DATAR = *pData++; // send data
	i++;
    }
    while(SPI1->STATR & SPI_STATR_BSY); // wait for not busy
} /* SPI_write() */

// UART functions (polling mode)
void UARTInit(uint32_t u32Baud, int bRemap)
{
uint32_t int_divider, frac_divider, UARTBR;

   if (bRemap) { // USE PC0/PC1 for USART1
       // Enable GPIOC and UART
       RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1;
       RCC->APB2PCENR |= RCC_AFIOEN; // enable alternate function (remap)
       AFIO->PCFR1 |= (1<<21) | (1<<2); // set both bits for TX=PC0, RX=PC1
       // Push-Pull, 10MHz Output, GPIO C0, with AutoFunction
       GPIOC->CFGLR &= ~(0xf<<(4*0)); // C0 = TX
       GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*0);
       GPIOC->CFGLR &= ~(0xf<<(4*1)); // C1 = RX
       GPIOC->CFGLR |= (GPIO_CNF_IN_FLOATING)<<(4*1);
   } else { // use default PD5/PD6
       // Enable GPIOD and UART.
       RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;
       // Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
       GPIOD->CFGLR &= ~(0xf<<(4*5));
       GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
   }
   // 8n1.  Note if you don't specify a mode, UART remains off even when UE_Set.
   USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx;
   USART1->CTLR1 &= ~CTLR1_OVER8_Set;
   USART1->CTLR2 = USART_StopBits_1;
   USART1->CTLR3 = USART_HardwareFlowControl_None;

   // Calculate the baud rate
   int_divider = udiv32(25 * FUNCONF_SYSTEM_CORE_CLOCK, 4 * u32Baud);
   frac_divider = umod32(int_divider, 100);
   UARTBR = udiv32(int_divider, 100) << 4;
   UARTBR |= (udiv32((frac_divider * 16) + 50, 100) & 15);

   USART1->BRR = UARTBR;
   USART1->CTLR1 |= CTLR1_UE_Set; // enable USART1
} /* UARTInit() */

//
// returns an 8-bit character or -1 to indicate timeout
//
int UART_Read(uint32_t u32Timeout)
{
int iTimeout = 0;
int c;

    while((USART1->STATR & USART_FLAG_RXNE) == 0 && iTimeout < u32Timeout)
    {   
        iTimeout++;
    }           
    if (iTimeout >= u32Timeout)
        return -1;
    c = USART1->DATAR;
    return (c & 0xff);   
} /* UART_Read() */

void UART_Write(uint8_t *pData, int iLen)
{
int i;

   for (i = 0; i < iLen; i++) {
      while((USART1->STATR & USART_FLAG_TC) == 0) {};
            USART1->DATAR = *pData++;
   }
} /* UART_Write() */

// 
// 64-bit unsigned divide
// takes less flash space than the 2K used by the RISC-V math library
// 
uint64_t udiv64(uint64_t num, uint64_t den)
{
uint64_t place = 1;
uint64_t ret = 0;
   while ((num >> 1) >= den) {
      place<<=1;
      den<<=1;
   }
   for ( ; place>0; place>>=1, den>>=1) {
      if (num>=den) {
         num-=den;
         ret+=place;
      }
   }
   return ret;
} /* udiv64() */

//
// 32-bit unsigned divide
// takes less flash space than the 2K used by the RISC-V math library
//
uint32_t udiv32(uint32_t num, uint32_t den)
{
uint32_t place = 1;
uint32_t ret = 0;
   while ((num >> 1) >= den) {
      place<<=1;
      den<<=1;
   }
   for ( ; place>0; place>>=1, den>>=1) {
      if (num>=den) {
         num-=den;
         ret+=place;
      }
   }
   return ret;
} /* udiv32() */

uint32_t udivmod32(uint32_t num, uint32_t den, uint32_t *pRemainder)
{
uint32_t place = 1;
uint32_t ret = 0;
   while ((num >> 1) >= den) {
      place<<=1;
      den<<=1;
   }
   for ( ; place>0; place>>=1, den>>=1) {
      if (num>=den) {
         num-=den;
         ret+=place;
      }
   }
   if (pRemainder) *pRemainder = num;
   return ret;
} /* udivmod32() */

// 
// 32-bit unsigned modulus
// takes less flash space than the 2K used by the RISC-V math library
// 
uint32_t umod32(uint32_t num, uint32_t den)
{
uint32_t place = 1;
//uint32_t ret = 0;
   while ((num >> 1) >= den) {
      place<<=1;
      den<<=1;
   }
   for ( ; place>0; place>>=1, den>>=1) {
      if (num>=den) {
         num-=den;
//         ret+=place;
      }
   }
   return num;
} /* umod32() */

