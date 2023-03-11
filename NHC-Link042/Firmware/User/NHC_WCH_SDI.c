#include "NHC_WCH_SDI.h"

/*
	1-Wire WCH (CH32V003) debug protocol
	Ngo Hung Cuong
	http://vidieukhien.org
	ngohungcuong@gmail.com
	
	Based on STM32F042
*/

static uint16_t u16Capture[100];
static uint8_t u8Data[100];

#define SDI_1 GPIOA->BSRR = 0x00002000
#define SDI_0 GPIOA->BRR = 0x00002000
#define SDI_IN (GPIOA->IDR & 0x00002000)

#define H_VAL 1
#define L_VAL 32
#define IN_VAL 4

void NHC_Delay_Us(uint16_t u16Delay);
void NHC_Delay_Ms(uint32_t u32Delay);

void NHC_Delay_Us(uint16_t u16Delay)
{
	uint16_t u16Tmp;
	
	u16Tmp = u16Delay * 48;
	TIM16->CNT = 0;
	while (TIM16->CNT < u16Tmp)
	{
	}
}

void NHC_Delay_Ms(uint32_t u32Delay)
{
	
	while (u32Delay)
	{
		NHC_Delay_Us(500);
		NHC_Delay_Us(500);
		--u32Delay;
	}
}

void NHC_WchSdi_Init(void)
{
	GPIO_InitTypeDef gpioInit;
	TIM_TimeBaseInitTypeDef timInit;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
	
	timInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timInit.TIM_CounterMode = TIM_CounterMode_Up;
	timInit.TIM_Period = 0xffff;
	timInit.TIM_Prescaler = 1 - 1;
	timInit.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM16, &timInit);
	TIM_Cmd(TIM16, ENABLE);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_13);
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	gpioInit.GPIO_OType = GPIO_OType_OD;
	gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	gpioInit.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_Init(GPIOA, &gpioInit);
	
	SDI_1;
	NHC_Delay_Ms(1);
	SDI_0;
	NHC_Delay_Us(36);
	SDI_1;
}

void NHC_WchSdi_Exit(void)
{
	GPIO_InitTypeDef gpioInit;
	
	GPIO_SetBits(GPIOA, GPIO_Pin_13);
	gpioInit.GPIO_Mode = GPIO_Mode_IN;
	gpioInit.GPIO_OType = GPIO_OType_OD;
	gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	gpioInit.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_Init(GPIOA, &gpioInit);
	
	TIM_DeInit(TIM16);
	TIM_Cmd(TIM16, DISABLE);
}

void NHC_WchSdi_Write(uint8_t u8Address, uint32_t u32Data)
{
	uint32_t i;
	uint8_t *p;
	
	u8Data[0] = 1;
	
	for (i = 0; i < 7; ++i)
	{
		if (u8Address & 0x40)
		{
			u8Data[i + 1] = 1;
		}
		else
		{
			u8Data[i + 1] = 0;
		}
		u8Address <<= 1;
	}
	
	u8Data[8] = 1;
	
	for (i = 0; i < 32; ++i)
	{
		if (u32Data & 0x80000000)
		{
			u8Data[i + 9] = 1;
		}
		else
		{
			u8Data[i + 9] = 0;
		}
		u32Data <<= 1;
	}
	
	p = u8Data;
	
	for (i = 0; i < 41; ++i)
	{
		if (*p++)
		{
			SDI_0;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
			SDI_1;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
		}
		else
		{
			SDI_0;
			TIM16->CNT = 0;
			while (TIM16->CNT < L_VAL)
			{
			}
			SDI_1;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
		}
	}

    NHC_Delay_Us(2);
}

uint32_t NHC_WchSdi_Read(uint8_t u8Address, uint32_t *pu32Data)
{
	uint32_t i;
	uint32_t u32Tmp;
	uint8_t *p;
	uint16_t *pu16;
	
	u8Data[0] = 1;
	for (i = 0; i < 7; ++i)
	{
		if (u8Address & 0x40)
		{
			u8Data[i + 1] = 1;
		}
		else
		{
			u8Data[i + 1] = 0;
		}
		u8Address <<= 1;
	}
	u8Data[8] = 0;
	
	p = u8Data;
	
	for (i = 0; i < 9; ++i)
	{
		if (*p++)
		{
			SDI_0;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
			SDI_1;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
		}
		else
		{
			SDI_0;
			TIM16->CNT = 0;
			while (TIM16->CNT < L_VAL)
			{
			}
			SDI_1;
			TIM16->CNT = 0;
			while (TIM16->CNT < H_VAL)
			{
			}
		}
	}
	
	pu16 = u16Capture;
	
	for (i = 0; i < 32; ++i)
	{
		SDI_0;
		TIM16->CNT = 0;
		while (TIM16->CNT < H_VAL)
		{
		}
		SDI_1;
		TIM16->CNT = 0;
		while (1)
		{
			if (SDI_IN)
			{
				*pu16++ = TIM16->CNT;
				break;
			}
			if (TIM16->CNT > 144)
			{
				return 0;
			}
		}
	}
	
	u32Tmp = 0;
	
	for (i = 0; i < 32; ++i)
	{
		u32Tmp <<= 1;
		if (u16Capture[i] < IN_VAL)
		{
			u32Tmp |= 0x01;
		}
	}
	
	*pu32Data = u32Tmp;
	
    NHC_Delay_Us(2);
	return 1;
}

uint32_t NHC_WchSdi_WriteMem(uint32_t u32Address, uint32_t u32Data, uint8_t u8Width)
{
	uint32_t u32Tmp;
	uint32_t i;
	
	u32Tmp = 2;
	
	if(u8Width == 8)
	{
		u32Tmp = 0;
	}
	else if (u8Width == 16)
	{
		u32Tmp = 1;
	}
	else if (u8Width == 32)
	{
		u32Tmp = 2;
	}
	else if (u8Width == 64)
	{
		u32Tmp = 3;
	}
	else if (u8Width == 128)
	{
		u32Tmp = 4;
	}
	
	u32Tmp <<= 12;
	
	NHC_WchSdi_Write(0x20, 0x00728023  | u32Tmp); //cho nay can thay doi
	NHC_WchSdi_Write(0x21, 0x00100073);
	NHC_WchSdi_Write(0x04, u32Address);
	NHC_WchSdi_Write(0x17, 0x00231005);
	for (i = 0; i < 1000; ++i)
	{
		if (NHC_WchSdi_Read(0x16, &u32Tmp))
		{
			if (!(u32Tmp & (1 << 12)))
			{
				break;
			}
		}
	}
	if (i == 1000)
	{
		return 0;
	}
	if (u32Tmp & (7 << 8))
	{
		return 0;
	}
	NHC_WchSdi_Write(0x04, u32Data);
	NHC_WchSdi_Write(0x17, 0x00271007);
	for (i = 0; i < 1000; ++i)
	{
		if (NHC_WchSdi_Read(0x16, &u32Tmp))
		{
			if (!(u32Tmp & (1 << 12)))
			{
				break;
			}
		}
	}
	if (i == 1000)
	{
		return 0;
	}
	if (u32Tmp & (7 << 8))
	{
		return 0;
	}
	
	return 1;
}

uint32_t NHC_WchSdi_ReadMem(uint32_t u32Address, uint32_t *pu32Data, uint8_t u8Width)
{
	uint32_t u32Tmp;
	uint32_t i;
	
	u32Tmp = 2;
	
	if(u8Width == 8)
	{
		u32Tmp = 0;
	}
	else if (u8Width == 16)
	{
		u32Tmp = 1;
	}
	else if (u8Width == 32)
	{
		u32Tmp = 2;
	}
	else if (u8Width == 64)
	{
		u32Tmp = 3;
	}
	else if (u8Width == 128)
	{
		u32Tmp = 4;
	}
	
	u32Tmp <<= 12;
	
	//NHC_WchSdi_Write(0x10, 0x80000001);
	//NHC_WchSdi_Write(0x10, 0x80000001);
	//NHC_WchSdi_Write(0x10, 0x00000001);
	NHC_WchSdi_Write(0x20, 0x00028303 | u32Tmp); //cho nay can thay doi
	NHC_WchSdi_Write(0x21, 0x00100073);
	NHC_WchSdi_Write(0x04, u32Address);
	NHC_WchSdi_Write(0x17, 0x00271005);
	for (i = 0; i < 1000; ++i)
	{
		if (NHC_WchSdi_Read(0x16, &u32Tmp))
		{
			if (!(u32Tmp & (1 << 12)))
			{
				break;
			}
		}
	}
	if (i == 1000)
	{
		//day loi len
		*pu32Data = 0xffffffff;
		return 0;
	}
	if (u32Tmp & (7 << 8))
	{
		//day loi len
		*pu32Data = u32Tmp;
		return 0;
	}

	NHC_WchSdi_Write(0x17, 0x00221006);
	for (i = 0; i < 1000; ++i)
	{
		if (NHC_WchSdi_Read(0x16, &u32Tmp))
		{
			if (!(u32Tmp & (1 << 12)))
			{
				break;
			}
		}
	}
	if (i == 1000)
	{
		*pu32Data = 0xffffffff;
		return 0;
	}
	if (u32Tmp & (7 << 8))
	{
		*pu32Data = u32Tmp;
		return 0;
	}
	
	if (!NHC_WchSdi_Read(0x04, &u32Tmp))
	{
		*pu32Data = 0x00000000;
		return 0;
	}
	
	*pu32Data = u32Tmp;
	
	return 1;
}
