/**
  ******************************************************************************
  * @file    usbd_audio_core.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   header file for the usbd_audio_core.c file.
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
#ifndef __USB_AUDIO_CORE_H_
#define __USB_AUDIO_CORE_H_

/* Includes ------------------------------------------------------------------*/
#include "usbd_req.h"
#include "stm32_audio_out_if.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_audio
  * @brief This file is the Header file for USBD_audio.c
  * @{
  */ 

/* Exported defines ----------------------------------------------------------*/

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define DEFAULT_OUT_BIT_RESOLUTION                    16
#define DEFAULT_OUT_CHANNEL_NBR                       2 /* Mono = 1, Stereo = 2 */
#define AUDIO_OUT_PACKET                              (uint32_t)(((DEFAULT_OUT_AUDIO_FREQ * \
                                                                  (DEFAULT_OUT_BIT_RESOLUTION/8) *\
                                                                   DEFAULT_OUT_CHANNEL_NBR) /1000))

/* Out endpoint max packet size (in Feedback Synchronization mode, this parameter should be set high enough
   in order to tolerate the fluctuations of out packets size) */
#define AUDIO_OUT_MPS                                 384

#define AUDIO_CONFIG_DESC_SIZE                        109
#define AUDIO_INTERFACE_DESC_SIZE                     0x09
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_OUT_STREAMING_CTRL                      0x02

/* Buffering state definitions */
#define STATE_IDLE                                    0
#define STATE_BUFFERING                               1
#define STATE_READY                                   2
#define STATE_CLOSING                                 3
#define STATE_RUN                                     4
#define STATE_RUN_OVR                                 (STATE_RUN + 1)
#define STATE_RUN_UDR                                 (STATE_RUN + 2)
#define STATE_RUN_ERR                                 (STATE_RUN + 3)
#define STATE_ERROR                                   8

/* Number of empty frame after which the communication should be considered as closed. */
#define EMPTY_FRAME_THRESHOLD                         3

/* Exported types ------------------------------------------------------------*/
typedef struct _Buff_Desc
{
  uint32_t  Size;
  uint8_t*  Next;
}BUFF_DESC_TypeDef;

/* Exported macros -----------------------------------------------------------*/
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(AUDIO_OUT_PACKET & 0xFF), \
                                       (uint8_t)((AUDIO_OUT_PACKET >> 8) & 0xFF)
#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* Exported variables --------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  AUDIO_cb;

/* Exported functions ------------------------------------------------------- */ 
/* This function should be called by low layer functions when current buffer transfer is complete */
void usbd_audio_BuffXferCplt (uint8_t** pbuf, uint32_t* pSize);

#endif  /* __USB_AUDIO_CORE_H_ */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
