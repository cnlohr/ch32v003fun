/**
  ******************************************************************************
  * @file    usbd_conf_template.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   usb device configuration template file
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
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "audio_app_conf.h" /* Audio example Include*/

/* Exported defines ----------------------------------------------------------*/
/*Common Defines*/
#define USBD_SELF_POWERED 
#define USBD_CFG_MAX_NUM                1 

/*Audio Class user defines*/
#define AUDIO_TOTAL_IF_NUM              0x02
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ            200 
#define AUDIO_OUT_EP                    0x01

/*CCID Class user defines*/
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ            64               
#define USB_SUPPORT_USER_STRING_DESC
#define CCID_BULK_IN_EP              	0x81
#define CCID_BULK_OUT_EP             	0x01
#define CCID_INTR_IN_EP              	0x82
#define CCID_BULK_EP_MAX_PACKET         64
#define CCID_INTR_EP_MAX_PACKET         8

/*Composite HID + CDC Classes user defines*/
#define USBD_ITF_MAX_NUM           	1
#define USB_MAX_STR_DESC_SIZ       	64               
#define CUSTOMHID_SIZ_REPORT_DESC	163
#define CUSTOMHID_SIZ_CONFIG_DESC	41
#define HID_IN_EP                    	0x81
#define HID_OUT_EP                   	0x01

/*2 Bytes max*/
#define HID_IN_PACKET                	2 
#define HID_OUT_PACKET               	2 
#define CDC_IN_EP                       0x83  /* EP1 for data IN */
#define CDC_OUT_EP                      0x03  /* EP3 for data OUT */
#define CDC_CMD_EP                      0x82  /* EP2 for CDC commands */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrate and performance. */
#define CDC_DATA_MAX_PACKET_SIZE       	64   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE             	8    /* Control Endpoint Packet size */
#define CDC_IN_FRAME_INTERVAL          	5    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE               	2048 /* Total size of IN buffer: APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 
                                                should be > CDC_IN_FRAME_INTERVAL */
#define APP_FOPS                        VCP_fops

/*Composite HID + MSC Class user defines*/
#define USBD_ITF_MAX_NUM           	1
#define USB_MAX_STR_DESC_SIZ       	64             
#define CUSTOMHID_SIZ_REPORT_DESC	163
#define CUSTOMHID_SIZ_CONFIG_DESC	41
#define HID_IN_EP			0x81
#define HID_OUT_EP			0x01
/*2 Bytes max*/
#define HID_IN_PACKET			2 
#define HID_OUT_PACKET			2 
#define MSC_IN_EP			0x82
#define MSC_OUT_EP			0x02
#define MSC_MAX_PACKET			64
#define MSC_MEDIA_PACKET		512

/*Custom HID Class user defines*/
#define USBD_ITF_MAX_NUM		1
#define USB_MAX_STR_DESC_SIZ		64              
#define CUSTOMHID_SIZ_REPORT_DESC	163
#define CUSTOMHID_SIZ_CONFIG_DESC	41
#define HID_IN_EP			0x81
#define HID_OUT_EP			0x01
#define HID_IN_PACKET			2
#define HID_OUT_PACKET			2

/*DFU Class user defines*/
#define USBD_ITF_MAX_NUM                MAX_USED_MEDIA
#define USB_MAX_STR_DESC_SIZ            200 
#define USB_SUPPORT_USER_STRING_DESC
#define XFERSIZE                        1024   /* Max DFU Packet Size   = 1024 bytes */
#define DFU_IN_EP                       0x80
#define DFU_OUT_EP                      0x00
 /* Maximum number of supported media (Flash) */
#define MAX_USED_MEDIA                  1
/* Flash memory address from where user application will be loaded 
   This address represents the DFU code protected against write and erase operations.*/
#define APP_DEFAULT_ADD                 0x08003000
#define DFU_MAL_IS_PROTECTED_AREA(add)    (uint8_t)(((add >= 0x08000000) && (add < (APP_DEFAULT_ADD)))? 1:0)
#define TRANSFER_SIZE_BYTES(sze)          ((uint8_t)(sze)), /* XFERSIZEB0 */\
                                          ((uint8_t)(sze >> 8)) /* XFERSIZEB1 */
										  
/*HID Class user defines*/	
#define USBD_ITF_MAX_NUM           	1
#define USB_MAX_STR_DESC_SIZ       	64               
#define USB_SUPPORT_USER_STRING_DESC
#define HID_IN_EP                    	0x81
#define HID_OUT_EP                   	0x01
#define HID_IN_PACKET                	4
#define HID_OUT_PACKET               	4
                                            
/*MSC Class user defines*/	
#define USBD_ITF_MAX_NUM           	1
#define USB_MAX_STR_DESC_SIZ       	64 
#define MSC_IN_EP                    	0x81
#define MSC_OUT_EP                   	0x02
#define MSC_MAX_PACKET			64
#define MSC_MEDIA_PACKET              	512
                                            
/*VCP Class user defines*/	
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ            255 	
#define CDC_IN_EP                       0x81  /* EP1 for data IN */
#define CDC_OUT_EP                      0x03  /* EP3 for data OUT */
#define CDC_CMD_EP                      0x82  /* EP2 for CDC commands */
/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrate and performance. */
#define CDC_DATA_MAX_PACKET_SIZE	64   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE		8    /* Control Endpoint Packet size */
#define CDC_IN_FRAME_INTERVAL		5    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE		2048 /* Total size of IN buffer: 
                                    APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL */
#define APP_FOPS                        VCP_fops					  

/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
