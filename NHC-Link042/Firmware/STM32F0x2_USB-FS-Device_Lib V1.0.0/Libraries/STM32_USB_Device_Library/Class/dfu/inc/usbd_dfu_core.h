/**
  ******************************************************************************
  * @file    usbd_dfu_core.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   header file for the usbd_dfu_core.c file.
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
#ifndef __USB_DFU_CORE_H_
#define __USB_DFU_CORE_H_

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_mal.h"
#include "usbd_req.h"

/* Exported defines ----------------------------------------------------------*/
#define USB_DFU_CONFIG_DESC_SIZ       (18 + (9 * USBD_ITF_MAX_NUM))
#define USB_DFU_DESC_SIZ              9

#define DFU_DESCRIPTOR_TYPE           0x21


/*---------------------------------------------------------------------*/
/*  DFU definitions                                                    */
/*---------------------------------------------------------------------*/
/**************************************************/
/* DFU Requests  DFU states                       */
/**************************************************/


#define STATE_appIDLE                 0x00
#define STATE_appDETACH               0x01
#define STATE_dfuIDLE                 0x02
#define STATE_dfuDNLOAD_SYNC          0x03
#define STATE_dfuDNBUSY               0x04
#define STATE_dfuDNLOAD_IDLE          0x05
#define STATE_dfuMANIFEST_SYNC        0x06
#define STATE_dfuMANIFEST             0x07
#define STATE_dfuMANIFEST_WAIT_RESET  0x08
#define STATE_dfuUPLOAD_IDLE          0x09
#define STATE_dfuERROR                0x0A

/**************************************************/
/* DFU Requests  DFU status                       */
/**************************************************/

#define STATUS_OK                   0x00
#define STATUS_ERRTARGET            0x01
#define STATUS_ERRFILE              0x02
#define STATUS_ERRWRITE             0x03
#define STATUS_ERRERASE             0x04
#define STATUS_ERRCHECK_ERASED      0x05
#define STATUS_ERRPROG              0x06
#define STATUS_ERRVERIFY            0x07
#define STATUS_ERRADDRESS           0x08
#define STATUS_ERRNOTDONE           0x09
#define STATUS_ERRFIRMWARE          0x0A
#define STATUS_ERRVENDOR            0x0B
#define STATUS_ERRUSBR              0x0C
#define STATUS_ERRPOR               0x0D
#define STATUS_ERRUNKNOWN           0x0E
#define STATUS_ERRSTALLEDPKT        0x0F

/**************************************************/
/* DFU Requests  DFU states Manifestation State   */
/**************************************************/

#define Manifest_complete           0x00
#define Manifest_In_Progress        0x01


/**************************************************/
/* Special Commands  with Download Request        */
/**************************************************/

#define CMD_GETCOMMANDS              0x00
#define CMD_SETADDRESSPOINTER        0x21
#define CMD_ERASE                    0x41

/**************************************************/
/* Other defines                                  */
/**************************************************/
/* Bit Detach capable = bit 3 in bmAttributes field */
#define DFU_DETACH_MASK              (uint8_t)(1 << 4)

/* Exported types ------------------------------------------------------------*/
/**************************************************/
/* DFU Requests                                   */
/**************************************************/

typedef enum _DFU_REQUESTS {
  DFU_DETACH = 0,
  DFU_DNLOAD = 1,
  DFU_UPLOAD,
  DFU_GETSTATUS,
  DFU_CLRSTATUS,
  DFU_GETSTATE,
  DFU_ABORT
} DFU_REQUESTS;

typedef  void (*pFunction)(void);

/* Exported macros -----------------------------------------------------------*/
/**********  Descriptor of DFU interface 0 Alternate setting n ****************/  
#define USBD_DFU_IF_DESC(n)   0x09,   /* bLength: Interface Descriptor size */ \
                              USB_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */ \
                              0x00,   /* bInterfaceNumber: Number of Interface */ \
                              (n),      /* bAlternateSetting: Alternate setting */ \
                              0x00,   /* bNumEndpoints*/ \
                              0xFE,   /* bInterfaceClass: Application Specific Class Code */ \
                              0x01,   /* bInterfaceSubClass : Device Firmware Upgrade Code */ \
                              0x02,   /* nInterfaceProtocol: DFU mode protocol */ \
                              USBD_IDX_INTERFACE_STR + (n) + 1 /* iInterface: Index of string descriptor */ \
                              /* 18 */
                              
/* Exported variables --------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  DFU_cb;

/* Exported functions ------------------------------------------------------- */ 

#endif  /* __USB_DFU_CORE_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
