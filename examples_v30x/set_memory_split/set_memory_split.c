// Example changing memory option bytes on ch32v307 to CODE:192kB / RAM:128kB
/* This shows how to use the option bytes.  I.e. how do you disable NRST?
   WARNING Portions of this code are under the following copyright.
*/
/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file provides all the FLASH firmware functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

int FLASH_WaitForLastOperation(uint32_t Timeout);

int main()
{
	SystemInit();

	Delay_Ms( 100 );

	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	FLASH->MODEKEYR = FLASH_KEY1;
	FLASH->MODEKEYR = FLASH_KEY2;

	printf( "Option bytes started as:%04x\n", OB->USER );

	uint16_t rdptmp = RDP_Key;


	int status = FLASH_WaitForLastOperation(EraseTimeout);
	if(status == FLASH_COMPLETE)
	{
		FLASH->OBKEYR = FLASH_KEY1;
		FLASH->OBKEYR = FLASH_KEY2;

		FLASH->CTLR |= CR_OPTER_Set;
		FLASH->CTLR |= CR_STRT_Set;
		status = FLASH_WaitForLastOperation(EraseTimeout);

		if(status == FLASH_COMPLETE)
		{
			FLASH->CTLR &= CR_OPTER_Reset;
			FLASH->CTLR |= CR_OPTPG_Set;
			OB->RDPR = (uint16_t)rdptmp;
			status = FLASH_WaitForLastOperation(ProgramTimeout);

			if(status != FLASH_TIMEOUT)
			{
				FLASH->CTLR &= CR_OPTPG_Reset;
			}
		}
		else
		{
			if(status != FLASH_TIMEOUT)
			{
				FLASH->CTLR &= CR_OPTPG_Reset;
			}
		}
	}


	printf( "After Clear:%04x\n", OB->USER );

    FLASH->OBKEYR = FLASH_KEY1;
    FLASH->OBKEYR = FLASH_KEY2;
    status = FLASH_WaitForLastOperation(10000);

    if(status == FLASH_COMPLETE)
    {
        FLASH->CTLR |= CR_OPTPG_Set;
        OB->USER = 0xc03f; // This sets the memory split.

        status = FLASH_WaitForLastOperation(10000);
        if(status != FLASH_TIMEOUT)
        {
            FLASH->CTLR &= CR_OPTPG_Reset;
        }
    }

	printf( "After Write:%04x\n", OB->USER );

	while(1);
}


int FLASH_GetBank1Status(void)
{
	int flashstatus = FLASH_COMPLETE;

	if((FLASH->STATR & FLASH_FLAG_BANK1_BSY) == FLASH_FLAG_BSY)
	{
		flashstatus = FLASH_BUSY;
	}
	else
	{
		if((FLASH->STATR & FLASH_FLAG_BANK1_WRPRTERR) != 0)
		{
			flashstatus = FLASH_ERROR_WRP;
		}
		else
		{
			flashstatus = FLASH_COMPLETE;
		}
	}
	return flashstatus;
}


int FLASH_WaitForLastOperation(uint32_t Timeout)
{
	int status = FLASH_COMPLETE;

	status = FLASH_GetBank1Status();
	while((status == FLASH_BUSY) && (Timeout != 0x00))
	{
		status = FLASH_GetBank1Status();
		Timeout--;
	}
	if(Timeout == 0x00)
	{
		status = FLASH_TIMEOUT;
	}
	return status;
}

