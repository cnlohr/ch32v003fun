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

#ifdef BITBANG
uint8_t u8SDA_Pin, u8SCL_Pin;
int iDelay = 1;
#endif

void delay(int i)
{
	Delay_Ms(i);
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
    return (u32Value == 0);
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
    I2C_InitTypeDef I2C_InitTSturcture={0};

    I2C_InitTSturcture.I2C_ClockSpeed = iSpeed;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = 0x02; //address; sender's unimportant address
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );
} /* I2CSetSpeed() */

void I2CInit(uint8_t iSDA, uint8_t iSCL, int iSpeed)
{
	(void)iSDA; (void)iSCL;

    GPIO_InitTypeDef GPIO_InitStructure={0};

    // Fixed to pins C1/C2 for now
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    I2C_DeInit(I2C1);
    I2CSetSpeed(iSpeed);

    I2C_Cmd( I2C1, ENABLE );

    I2C_AcknowledgeConfig( I2C1, ENABLE );
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );
} /* I2CInit() */

//
// Returns 0 for timeout error
// returns 1 for success
//
int I2CRead(uint8_t u8Addr, uint8_t *pData, int iLen)
{
	int iTimeout = 0;

    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C_Send7bitAddress( I2C1, u8Addr<<1, I2C_Direction_Receiver );

    while(iTimeout < 10000 && !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) ) {
    	iTimeout++;
    }
    if (iTimeout >= 10000) return 0; // error

    iTimeout = 0;
    while(iLen && iTimeout < 10000)
    {
        if( I2C_GetFlagStatus( I2C1, I2C_FLAG_RXNE ) !=  RESET )
        {
        	iTimeout = 0;
            pData[0] = I2C_ReceiveData( I2C1 );
            pData++;
            iLen--;
        } else {
        	iTimeout++;
        }
    }

    I2C_GenerateSTOP( I2C1, ENABLE );
    return (iLen == 0);

} /* I2CRead() */

void I2CWrite(uint8_t u8Addr, uint8_t *pData, int iLen)
{
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C_Send7bitAddress( I2C1, u8Addr<<1, I2C_Direction_Transmitter );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    while(iLen)
    {
        if( I2C_GetFlagStatus( I2C1, I2C_FLAG_TXE ) !=  RESET )
        {
            I2C_SendData( I2C1, pData[0] );
            pData++;
            iLen--;
        }
    }

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    I2C_GenerateSTOP( I2C1, ENABLE );

} /* I2CWrite() */

int I2CTest(uint8_t u8Addr)
{
	int iTimeout = 0;

	I2C_ClearFlag(I2C1, I2C_FLAG_AF);
    I2C_GenerateSTART( I2C1, ENABLE );
    while(iTimeout < 10000 && !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) ) {
    	iTimeout++;
    }
    if (iTimeout >= 10000) return 0; // no pull-ups, open bus

    I2C_Send7bitAddress( I2C1, u8Addr<<1, I2C_Direction_Transmitter );

    while(iTimeout < 10000 && !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ) {
    	iTimeout++;
    }
    if (iTimeout >= 10000) return 0; // no device at that address; the MTMS flag will never get set

    I2C_GenerateSTOP( I2C1, ENABLE );
    // check ACK failure flag
    return (I2C_GetFlagStatus(I2C1, /*I2C_FLAG_TXE*/I2C_FLAG_AF) == RESET); // 0 = fail, 1 = succeed

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
    else
        u32Prescaler = SPI_BaudRatePrescaler_256;

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
