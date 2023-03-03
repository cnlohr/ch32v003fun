/**
  ******************************************************************************
  * @file    usbd_ccid_core.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides all the CCID core functions.
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
#ifndef _USB_CCID_CORE_H_
#define _USB_CCID_CORE_H_

/* Includes ------------------------------------------------------------------*/
#include "usbd_req.h"
#include "sc_itf.h"

/* Exported defines ----------------------------------------------------------*/
#define TPDU_EXCHANGE                  0x01
#define SHORT_APDU_EXCHANGE            0x02
#define EXTENDED_APDU_EXCHANGE         0x04
#define CHARACTER_EXCHANGE             0x00

#define EXCHANGE_LEVEL_FEATURE         TPDU_EXCHANGE

#define REQUEST_ABORT                  0x01
#define REQUEST_GET_CLOCK_FREQUENCIES  0x02
#define REQUEST_GET_DATA_RATES         0x03

#define SMARTCARD_SIZ_CONFIG_DESC      93 

#define CCID_BULK_EPIN_SIZE          CCID_BULK_EP_MAX_PACKET 
#define CCID_BULK_EPOUT_SIZE         CCID_BULK_EP_MAX_PACKET  
#define CCID_INTR_EPIN_SIZE          CCID_INTR_EP_MAX_PACKET
#define CCID_EP0_BUFF_SIZ            64

/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  USBD_CCID_cb;
/* Exported functions ------------------------------------------------------- */

#endif  /* _USB_CCID_CORE_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
