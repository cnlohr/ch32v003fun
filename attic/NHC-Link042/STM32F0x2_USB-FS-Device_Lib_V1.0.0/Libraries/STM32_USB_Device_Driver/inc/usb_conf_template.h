/**
  ******************************************************************************
  * @file    usb_conf_template.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   General low level driver configuration
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
#ifndef __USB_CONF__H__
#define __USB_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "stm32072b_eval.h"

/* Exported defines ----------------------------------------------------------*/
/* Select D+ pull up: internal or external */
#ifdef USE_STM32072B_EVAL
 /* When using STM32072B_EVAL board the internal pull up must be enabled */
 #define INTERNAL_PULLUP
#endif

/* Define if Low power mode is enabled; it allows entering the device into STOP mode
    following USB Suspend event, and wakes up after the USB wakeup event is received. */
/* #define USB_DEVICE_LOW_PWR_MGMT_SUPPORT */

/* Select the CRS using HSI48 internal clock as USB clock source.
    If this define is commented, PLL will be used as USB clock source.
    User need to ensure that the PLL output clock to USB is 48MHz  for proper 
    USB functioning */
#define USB_CLOCK_SOURCE_CRS

/* Endpoints used by the device */
#define EP_NUM    (3) /* EP0 + EP1 for MSC IN + EP2 for MSC OUT */

/* Buffer table base address */
#define BTABLE_ADDRESS    (0x00)

/* EP0, RX/TX buffers base address */
#define ENDP0_RX_ADDRESS    (0x18)
#define ENDP0_TX_ADDRESS    (0x58)

/* EP1, TX buffer base address */
#define MSC_IN_TX_ADDRESS     (0x98)
    
/* EP2, Rx buffer base address */
#define MSC_OUT_RX_ADDRESS    (0xD8)

/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

#endif /* __USB_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
