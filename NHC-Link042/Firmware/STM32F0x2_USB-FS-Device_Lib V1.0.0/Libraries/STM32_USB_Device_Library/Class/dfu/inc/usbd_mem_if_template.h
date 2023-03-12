/**
  ******************************************************************************
  * @file    usbd_mem_if_template.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   Header for usbd_mem_if_template.c file.
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
#ifndef __MEM_IF_MAL_H
#define __MEM_IF_MAL_H

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbd_dfu_mal.h"

/* Exported defines ----------------------------------------------------------*/
#define MEM_START_ADD                 0x00000000 /* Dummy start address */
#define MEM_END_ADD                   (uint32_t)(MEM_START_ADD + (5 * 1024)) /* Dummy Size = 5KB */

#define MEM_IF_STRING                 "@Dummy Memory   /0x00000000/01*002Kg,03*001Kg"

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern DFU_MAL_Prop_TypeDef DFU_Mem_cb;

/* Exported functions ------------------------------------------------------- */

#endif /* __MEM_IF_MAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
