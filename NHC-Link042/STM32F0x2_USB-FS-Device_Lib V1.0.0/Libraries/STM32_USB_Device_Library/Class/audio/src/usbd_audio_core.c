/**
  ******************************************************************************
  * @file    usbd_audio_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB Audio Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as Audio Streaming Device
  *           - Audio Streaming data transfer
  *           - AudioControl requests management
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                Audio Class Driver Description
  *          =================================================================== 
  *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          
  *           @note
  *            The Audio Class 1.0 is based on USB Specification 1.0 and thus supports only
  *            Low and Full speed modes and does not allow High Speed transfers.
  *            Please refer to "USB Device Class Definition for Audio Devices V1.0 Mar 18, 98"
  *            for more details.
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - AudioControl Endpoint management
  *             - AudioControl requests other than SET_CUR and GET_CUR
  *             - Abstraction layer for AudioControl requests (only Mute functionality is managed)
  *             - Audio Synchronization type: Adaptive
  *             - Audio Compression modules and interfaces
  *             - MIDI interfaces and modules
  *             - Mixer/Selector/Processing/Extension Units (Feature unit is limited to Mute control)
  *             - Any other application-specific modules
  *             - Multiple and Variable audio sampling rates
  *             - Out Streaming Endpoint/Interface (microphone)
  *      
  *  @endverbatim
  *                                  
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
#include "usbd_audio_core.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup usbd_audio 
  * @brief usbd core module
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*********************************************
   AUDIO Device library callbacks
 *********************************************/
static uint8_t  usbd_audio_Init       (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_DeInit     (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_audio_EP0_RxReady(void *pdev);
static uint8_t  usbd_audio_DataIn     (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_DataOut    (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_SOF        (void *pdev);

static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length);

/*********************************************
   AUDIO Requests management functions
 *********************************************/
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req);

/* Private variables ---------------------------------------------------------*/
 /* Main Buffer for Audio Data Out transfers and its related pointers */
/* This is the main buffer where all out audio data are stored */
uint8_t  IsocOutBuff [TOTAL_OUT_BUF_SIZE];
/* This is the pointer used by the write process (from host to device) */
uint8_t* IsocOutWrPtr         = IsocOutBuff; 
/* This is the pointer used by the read process (from buffers to I2S) */
uint8_t* IsocOutRdPtr         = IsocOutBuff;    

uint8_t  OutEmptyFrameCount   = 0;

BUFF_DESC_TypeDef IsocOutBufDesc[OUT_PACKET_NUM];
uint32_t IsocOutWrBufDescIdx  = 0;
uint32_t IsocOutRdBufDescIdx  = 0;
uint8_t  IsocOutWrState       = STATE_IDLE;
uint8_t  IsocOutRdState       = STATE_IDLE;

/* Main Buffer for Audio Control Requests transfers and its related variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd          = 0;
uint32_t AudioCtlLen          = 0;
uint8_t  AudioCtlUnit         = 0;


/* USB AUDIO device Configuration Descriptor */
const uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
  LOBYTE(AUDIO_CONFIG_DESC_SIZE),       /* wTotalLength  109 bytes*/
  HIBYTE(AUDIO_CONFIG_DESC_SIZE),      
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes  BUS Powred*/
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/

  /* USB Speaker Standard interface descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Class-specific AC Interface Descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdADC */
  0x01,
  0x27,                                 /* wTotalLength = 39*/
  0x00,
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
  /* 09 byte*/

  /* USB Speaker Input Terminal Descriptor */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
  0x01,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
  0x01,
  0x00,                                 /* bAssocTerminal */
  0x01,                                 /* bNrChannels */
  0x00,                                 /* wChannelConfig 0x0000  Mono */
  0x00,
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
  /* 12 byte*/

  /* USB Speaker Audio Feature Unit Descriptor */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
  AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
  0x01,                                 /* bSourceID */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE,                   /* bmaControls(0) */
  0x00,                                 /* bmaControls(1) */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /*USB Speaker Output Terminal Descriptor */
  0x09,      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
  0x03,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType  0x0301*/
  0x03,
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwidth */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
  0x01,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001 */
  0x00,
  /* 07 byte*/

  /* USB Speaker Audio Type III Format Interface Descriptor */
  (0x08 + (3 * SUPPORTED_FREQ_NBR)),    /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                  /* bFormatType */

  DEFAULT_OUT_CHANNEL_NBR,              /* bNrChannels */
  DEFAULT_OUT_CHANNEL_NBR,              /* bSubFrameSize :  2 Bytes per frame (16bits) */
  DEFAULT_OUT_BIT_RESOLUTION,           /* bBitResolution (16-bits per sample) */
  SUPPORTED_FREQ_NBR,                    /* bSamFreqType only one frequency supported */
  SAMPLE_FREQ(DEFAULT_OUT_AUDIO_FREQ),   /* Audio sampling frequency coded on 3 bytes */
#if (SUPPORTED_FREQ_NBR > 1)
  SAMPLE_FREQ(USBD_AUDIO_FREQ_2),       /* Audio sampling frequency coded on 3 bytes */
#endif
#if (SUPPORTED_FREQ_NBR > 2)
  SAMPLE_FREQ(USBD_AUDIO_FREQ_3),       /* Audio sampling frequency coded on 3 bytes */
#endif
#if (SUPPORTED_FREQ_NBR > 3)
  SAMPLE_FREQ(USBD_AUDIO_FREQ_4),       /* Audio sampling frequency coded on 3 bytes */
#endif
#if (SUPPORTED_FREQ_NBR > 4)
 #error "Error: Max nbr of frequencies exceeded, you have to modify the conf descriptor in usbd_audio_core.c file!"
#endif
 /* 11 + 3*N byte */

  /* USB Speaker Streaming Standard Endpoint Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
  USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
  LOBYTE(AUDIO_OUT_MPS),                /* wMaxPacketSize */
  HIBYTE(AUDIO_OUT_MPS),
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/

  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
#if (SUPPORTED_FREQ_NBR > 1)  
  0x01,                                 /* bmAttributes: Freq selection */
#else
  0x00,                                 /* bmAttributes: None */
#endif /* (SUPPORTED_FREQ_NBR > 1)  */
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,
  /* 07 byte*/
};

/* AUDIO interface class callbacks structure */
USBD_Class_cb_TypeDef  AUDIO_cb =
{
  usbd_audio_Init,
  usbd_audio_DeInit,
  usbd_audio_Setup,
  NULL, /* EP0_TxSent */
  usbd_audio_EP0_RxReady,
  usbd_audio_DataIn,
  usbd_audio_DataOut,
  usbd_audio_SOF,
  USBD_audio_GetCfgDesc,
};

/* Private function ----------------------------------------------------------*/
  
/**
  * @brief  usbd_audio_Init
  *         Initializes the AUDIO interface.
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_audio_Init (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Open EP OUT */
  DCD_PMA_Config(pdev, AUDIO_OUT_EP, USB_DBL_BUF, Audio_IN_TX_ADRESS);

  DCD_EP_Open(pdev, AUDIO_OUT_EP, AUDIO_OUT_PACKET, USB_EP_ISOC);

  /* Initialize the Audio output Hardware layer */ 
  if (AUDIO_OUT_fops.Init(DEFAULT_OUT_AUDIO_FREQ, DEFAULT_VOLUME, (DEFAULT_OUT_CHANNEL_NBR << 6 | DEFAULT_OUT_BIT_RESOLUTION)) != USBD_OK)
  {
    return USBD_FAIL;
  }

#if (DEFAULT_OUT_BIT_RESOLUTION == 16)
  WAVADDON_Init((DEFAULT_OUT_CHANNEL_NBR<<6 | DEFAULT_OUT_BIT_RESOLUTION), 0);
#endif /* DEFAULT_OUT_BIT_RESOLUTION */

  /* Prepare Out endpoint to receive audio data */
  DCD_EP_PrepareRx(pdev,
                   AUDIO_OUT_EP,
                   (uint8_t*)IsocOutBuff,
                   AUDIO_OUT_PACKET);  

  return USBD_OK;
}

/**
  * @brief  usbd_audio_DeInit
  *         DeInitializes the AUDIO layer.
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_audio_DeInit (void  *pdev, 
                                   uint8_t cfgidx)
{
  DCD_EP_Close (pdev , AUDIO_OUT_EP);

  /* DeInitialize the Audio output Hardware layer */
  if (AUDIO_OUT_fops.DeInit() != USBD_OK)
  {
    return USBD_FAIL;
  }

  return USBD_OK;
}

/**
  * @brief  usbd_audio_Setup
  *         Handles the Audio control request parsing.
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  usbd_audio_Setup (void  *pdev, 
                                  USB_SETUP_REQ *req)
{
  
  __IO uint32_t  usbd_audio_AltSet = 0;
  uint16_t len=USB_AUDIO_DESC_SIZ;
  uint8_t  *pbuf= (uint8_t *) usbd_audio_CfgDesc + 18;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* AUDIO Class Requests -------------------------------*/
  case USB_REQ_TYPE_CLASS :    
    switch (req->bRequest)
    {
    case AUDIO_REQ_GET_CUR:
      AUDIO_Req_GetCurrent(pdev, req);
      break;

    case AUDIO_REQ_SET_CUR:
      AUDIO_Req_SetCurrent(pdev, req);   
      break;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
    break;

    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
        pbuf = (uint8_t *)usbd_audio_CfgDesc + 18;
        len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&usbd_audio_AltSet,
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM)
      {
        usbd_audio_AltSet = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  usbd_audio_EP0_RxReady
  *         Handles audio control requests data.
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{
  /* Check if an AudioControl request has been issued */
  if (AudioCtlCmd == AUDIO_REQ_SET_CUR)
  {
    /* In this driver, to simplify code, only SET_CUR request is managed */
    /* Check for which addressed unit the AudioControl request has been issued */
    if (AudioCtlUnit == AUDIO_OUT_STREAMING_CTRL)
    {
      /* In this driver, to simplify code, only one unit is manage */
      /* Call the audio interface mute function */
      AUDIO_OUT_fops.MuteCtl(AudioCtl[0]);
      
      /* Reset the AudioCtlCmd variable to prevent re-entering this function */
      AudioCtlCmd = 0;
      AudioCtlLen = 0;
    }
  } 

  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Handles the audio IN data stage.
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{
  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataOut
  *         Handles the Audio Out data stage.
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_audio_DataOut (void *pdev, uint8_t epnum)
{
  uint32_t IsocOutPacketSze = AUDIO_OUT_PACKET;
  if (epnum == AUDIO_OUT_EP)
  {
    /* Set the Isochronous Buffer Out linked chain parameters */
    IsocOutBufDesc[IsocOutWrBufDescIdx].Size = ((USB_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
    IsocOutBufDesc[IsocOutWrBufDescIdx].Next = (uint8_t*)(IsocOutWrPtr + IsocOutBufDesc[IsocOutWrBufDescIdx].Size);

#if (DEFAULT_OUT_BIT_RESOLUTION == 16)
    WAVADDON_AudioProcessing(IsocOutWrPtr, IsocOutBufDesc[IsocOutWrBufDescIdx].Size);
#endif /* DEFAULT_OUT_BIT_RESOLUTION */

    /* Increment the write buffer pointer */
    IsocOutWrPtr += IsocOutBufDesc[IsocOutWrBufDescIdx].Size;

    /* Increment the linked chain member index */
    IsocOutWrBufDescIdx++;

    /* Check if the end of the the buffer has been reached */
    if (IsocOutWrBufDescIdx >= OUT_PACKET_NUM)
    {
      /* All buffers are full: roll back */
      IsocOutWrPtr = IsocOutBuff;
      IsocOutBufDesc[IsocOutWrBufDescIdx - 1].Next = IsocOutBuff;
      IsocOutWrBufDescIdx = 0;
    }

    /* Check and update the buffering state machine */
    switch (IsocOutWrState)
    {
    case STATE_IDLE:
      IsocOutWrState = STATE_BUFFERING;
      break;

    case STATE_BUFFERING:
      /* Check if the buffer index reached the half of total out buffer */
      if (IsocOutWrBufDescIdx >= (OUT_PACKET_NUM / 2))
      {
        /* Switch current state to Ready: allow read operation from the buffer */
        IsocOutWrState = STATE_READY;
      }
      break;

    case STATE_READY:
      /* Decrement the empty frame counter */
      if (OutEmptyFrameCount)
      {
        OutEmptyFrameCount --;
      }      
      break;

    case STATE_CLOSING:
      break;

    default:
      break;
    }

    /* Prepare Out endpoint to receive next audio packet */
    DCD_EP_PrepareRx(pdev,
                     AUDIO_OUT_EP,
                     IsocOutWrPtr,
                     IsocOutPacketSze);
  }

  return USBD_OK;
}

/**
  * @brief  usbd_audio_SOF
  *         Handles the SOF event (data buffer update and synchronization).
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_audio_SOF (void *pdev)
{
  /*Uncomment this line to visualize the shift between SOF and audio clock*/
  STM_EVAL_LEDToggle(LED1);
  
  /* Check on current state */
  if (IsocOutWrState == STATE_READY)
  {
    /* Increment the empty frame counter */
    OutEmptyFrameCount ++;

    /* Check if the output read operation has been started */
    if (IsocOutRdState == STATE_IDLE)
    {
      /* Start playing received packet */
      AUDIO_OUT_fops.AudioCmd(IsocOutBuff,                               /* Samples buffer pointer */
                              IsocOutBufDesc[IsocOutRdBufDescIdx].Size,  /* Number of samples in Bytes */
                              AUDIO_CMD_PLAY);                           /* Command to be processed */   

      /* Set the current read pointer */
      IsocOutRdPtr = IsocOutBufDesc[IsocOutRdBufDescIdx].Next; 

      /* Increment the read buffer descriptor index */
      IsocOutRdBufDescIdx++;

      /* Update the out read operation state */
      IsocOutRdState = STATE_RUN;
    }
  }

  /* Check if the empty frame counter reached the threshold (means audio streaming stopped) */
  if (OutEmptyFrameCount >= EMPTY_FRAME_THRESHOLD)
  {
    IsocOutWrState = STATE_CLOSING;
    OutEmptyFrameCount = 0;
  }

  /* Check if there are available data in stream buffer.
  The play operation must be executed as soon as possible after the SOF detection. */
  if (/*(IsocOutWrState == STATE_CLOSING) && */
      (IsocOutRdPtr == IsocOutWrPtr) && 
        (IsocOutRdState != STATE_IDLE))
  {
    /* Pause the audio stream */
    AUDIO_OUT_fops.AudioCmd(IsocOutBufDesc[IsocOutRdBufDescIdx - 1].Next,  /* Samples buffer pointer */
                            IsocOutBufDesc[IsocOutRdBufDescIdx].Size,      /* Number of samples in Bytes */
                            AUDIO_CMD_PAUSE);                              /* Command to be processed */

    IsocOutWrState = STATE_IDLE;
    IsocOutRdState = STATE_IDLE;

    /* Reset buffer pointers */
    IsocOutRdPtr = IsocOutBuff;
    IsocOutWrPtr = IsocOutBuff;
    IsocOutRdBufDescIdx = 0;
    IsocOutWrBufDescIdx = 0;
  }

  return USBD_OK;
}

/**
  * @brief  usbd_audio_BuffXferCplt
  *         Manage end of buffer transfer for each device.
  * @param  Direction: could be DIRECTION_IN or DIRECTION_OUT
  * @param  pbuf: Pointer to the address of the current buffer
  * @param  pSize: pointer to the variable which holds current buffer size
  * @retval status
  */
void usbd_audio_BuffXferCplt (uint8_t** pbuf, uint32_t* pSize)
{
  uint8_t* pRdPtr = IsocOutRdPtr;
  uint32_t currDistanceOut = 0;
  
  if (IsocOutRdState & STATE_RUN)
  {
    STM_EVAL_LEDToggle(LED2);
    
    if (IsocOutRdBufDescIdx == 0)
    {
      /* Start playing received packet */
      AUDIO_OUT_fops.AudioCmd(IsocOutBufDesc[OUT_PACKET_NUM - 1].Next,       /* Samples buffer pointer */
                              IsocOutBufDesc[IsocOutRdBufDescIdx].Size,      /* Number of samples in Bytes */
                              AUDIO_CMD_PLAY);                               /* Command to be processed */
      
      /* Set the current read pointer */
      IsocOutRdPtr = IsocOutBufDesc[OUT_PACKET_NUM - 1].Next;
    }
    else
    {
      /* Start playing received packet */
      AUDIO_OUT_fops.AudioCmd(IsocOutBufDesc[IsocOutRdBufDescIdx - 1].Next,  /* Samples buffer pointer */
                              IsocOutBufDesc[IsocOutRdBufDescIdx].Size,      /* Number of samples in Bytes */
                              AUDIO_CMD_PLAY);                               /* Command to be processed */
      
      /* Set the current read pointer */
      IsocOutRdPtr = IsocOutBufDesc[IsocOutRdBufDescIdx - 1].Next;
    }    
    
    /* Check if roll-back has been detected */
    if (((uint32_t)IsocOutWrPtr) >= (uint32_t)pRdPtr)
    {/* No roll-back */
      currDistanceOut = (uint32_t)(((uint32_t)IsocOutWrPtr) - (uint32_t)pRdPtr);
    }
    else
    {/* Roll-back occured */
      currDistanceOut = (uint32_t)(((uint32_t)(IsocOutBufDesc[OUT_PACKET_NUM-2].Next) +  \
        IsocOutBufDesc[OUT_PACKET_NUM-1].Size) - \
          (uint32_t)pRdPtr + ((uint32_t)IsocOutWrPtr) - \
            (uint32_t)(IsocOutBufDesc[OUT_PACKET_NUM-1].Next));    
    }
    
    /* Speed-up the timer (trigger of DAC) when the distance between write buffer
    and read buffer pointers is higher than 3/4 buffer size */
    if(currDistanceOut > 1176)/* 3/4 buffer size = 1568 * 3/4 = 1176 */
    {
      TIM6->ARR = 998;/* This value is hard-coded for 48KHz: 48MHz/48KHz - 2 */
    }
    /* Slow-down the timer (trigger of DAC) when the distance between write buffer
    and read buffer pointers is lower than 1/4 buffer size */
    else if(currDistanceOut < 392)/* 1/4 buffer size = 1568 * 1/4 = 392 */
    {
      TIM6->ARR = 1000;/* This value is hard-coded for 48KHz: 48MHz/48KHz */
    }
    /* Set the default timer frequency */
    else
    {
      TIM6->ARR = 999;/* This value is hard-coded for 48KHz: 48MHz/48KHz - 1 */
    }
    /* Increment the read buffer descriptor index */
    IsocOutRdBufDescIdx++;  
    
    /* Check if the end of the the buffer has been reached */
    if (IsocOutRdBufDescIdx == OUT_PACKET_NUM)
    {
      /* All buffers are full: roll back */
      IsocOutRdBufDescIdx = 0;
    }
  }
}

/******************************************************************************
     AUDIO Class requests management
******************************************************************************/
/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req)
{
  /* Send the current mute state */
  USBD_CtlSendData (pdev,
                    AudioCtl,
                    req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev, 
                       AudioCtl,
                       req->wLength);

    /* Set the global variables indicating current request and its length 
    to the function usbd_audio_EP0_RxReady() which will process the request */
    AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    AudioCtlLen = req->wLength;          /* Set the request data length */
    AudioCtlUnit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}

/**
  * @brief  USBD_audio_GetCfgDesc 
  *         Returns configuration descriptor.
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_audio_CfgDesc);
  return (uint8_t *)usbd_audio_CfgDesc;
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
