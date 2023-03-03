/**
  ******************************************************************************
  * @file    usbd_flash_if.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   Header for usbd_flash_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_MAL_H
#define __FLASH_IF_MAL_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_mal.h"

/* Exported defines ----------------------------------------------------------*/
#define FLASH_START_ADD                  0x08000000

#define FLASH_END_ADD                   0x08040000
#define FLASH_IF_STRING    (uint8_t*) "@Internal Flash   /0x08000000/12*001Ka,116*001Kg"  

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern DFU_MAL_Prop_TypeDef DFU_Flash_cb;

/* Exported functions ------------------------------------------------------- */

#endif /* __FLASH_IF_MAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
