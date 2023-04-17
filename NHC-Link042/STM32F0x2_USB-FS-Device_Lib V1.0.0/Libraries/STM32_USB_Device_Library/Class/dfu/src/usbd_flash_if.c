/**
  ******************************************************************************
  * @file    usbd_flash_if.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   Specific media access Layer for internal flash.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
uint16_t FLASH_If_Init(void);
uint16_t FLASH_If_Erase (uint32_t Add);
uint16_t FLASH_If_Write (uint32_t Add, uint32_t Len);
uint8_t *FLASH_If_Read  (uint32_t Add, uint32_t Len);
uint16_t FLASH_If_DeInit(void);
uint16_t FLASH_If_CheckAdd(uint32_t Add);


/* Private variables ---------------------------------------------------------*/
DFU_MAL_Prop_TypeDef DFU_Flash_cb =
{
  FLASH_IF_STRING,
  FLASH_If_Init,
  FLASH_If_DeInit,
  FLASH_If_Erase,
  FLASH_If_Write,
  FLASH_If_Read,
  FLASH_If_CheckAdd,
  40, /* Erase Time in ms : extracted from flash memory datasheet Maximum 
  timming value for Sector Erase*/
  60  /* Programming Time in ms (60us * RAM Buffer size (1024 Bytes) 
    extracted from flash memory datasheet Maximum timming value for  1024 bytes 
    Write)*/
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  FLASH_If_Init
  *         Memory initialization routine.
  * @param  None
  * @retval MAL_OK if operation is successful, MAL_FAIL else.
*/
uint16_t FLASH_If_Init(void)
{
  /* Unlock the internal flash */
  FLASH_Unlock();
  
  return MAL_OK;
}

/**
  * @brief  FLASH_If_DeInit
  *         Memory deinitialization routine.
  * @param  None
  * @retval MAL_OK if operation is successful, MAL_FAIL else.
  */
uint16_t FLASH_If_DeInit(void)
{
  /* Lock the internal flash */
  FLASH_Lock();
  
  return MAL_OK;
}

/**
  * @brief  FLASH_If_Erase
  *         Erase sector.
  * @param  Add: Address to be written to.
  * @retval MAL_OK if operation is successful, MAL_FAIL else.
  */
uint16_t FLASH_If_Erase(uint32_t Add)
{
  /* Call the standard Flash erase function */
  FLASH_ErasePage(Add);  
  
  return MAL_OK;
}

/**
  * @brief  FLASH_If_Write
  *         Memory write routine.
  * @param  Add: Address to be written to.
  * @param  Len: Number of data to be written (in bytes).
  * @retval MAL_OK if operation is successful, MAL_FAIL else.
  */
uint16_t FLASH_If_Write(uint32_t Add, uint32_t Len)
{
  uint32_t idx = 0;
  
  if  (Len & 0x3) /* Not an aligned data */
  {
    for (idx = Len; idx < ((Len & 0xFFFC) + 4); idx++)
    {
      MAL_Buffer[idx] = 0xFF;
    }
  }
  
  /* Data received are Word multiple */
  for (idx = 0; idx <  Len; idx = idx + 4)
  {
    FLASH_ProgramWord(Add, *(uint32_t *)(MAL_Buffer + idx));
    Add += 4;
  }
  return MAL_OK;
}

/**
  * @brief  FLASH_If_Read
  *         Memory read routine.
  * @param  Add: Address to be read from.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *FLASH_If_Read (uint32_t Add, uint32_t Len)
{
  return  (uint8_t *)(Add);
}

/**
  * @brief  FLASH_If_CheckAdd
  *         Check if the address is an allowed address for this memory.
  * @param  Add: Address to be checked.
  * @param  Len: Number of data to be read (in bytes).
  * @retval MAL_OK if the address is allowed, MAL_FAIL else.
  */
uint16_t FLASH_If_CheckAdd(uint32_t Add)
{
  if ((Add >= FLASH_START_ADD) && (Add < FLASH_END_ADD))
  {
    return MAL_OK;
  }
  else
  {
    return MAL_FAIL;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
