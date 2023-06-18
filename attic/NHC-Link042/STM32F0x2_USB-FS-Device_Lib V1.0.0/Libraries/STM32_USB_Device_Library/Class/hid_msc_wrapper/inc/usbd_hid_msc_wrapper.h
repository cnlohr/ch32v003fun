/**
  ******************************************************************************
  * @file    usbd_msc_hid_wrapper.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   header file for the usbd_msc_hid_wrapper.c file.
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
#ifndef __USB_HID_MSC_WRAPPER_H_
#define __USB_HID_MSC_WRAPPER_H_

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/* Exported defines ----------------------------------------------------------*/
#define HID_INTERFACE 0x0
#define MSC_INTERFACE 0x1

/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define USB_HID_MSC_CONFIG_DESC_SIZ  (CUSTOMHID_SIZ_CONFIG_DESC -9 + USB_MSC_CONFIG_DESC_SIZ)

/* Exported variables --------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  USBD_HID_MSC_cb;

/* Exported functions ------------------------------------------------------- */ 


#endif  /* __USB_HID_MSC_WRAPPER_H_ */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
