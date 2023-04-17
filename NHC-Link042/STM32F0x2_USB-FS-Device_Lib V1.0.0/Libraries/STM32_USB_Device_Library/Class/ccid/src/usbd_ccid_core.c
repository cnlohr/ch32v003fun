/**
  ******************************************************************************
  * @file    usbd_ccid_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides all the CCID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                CCID Class  Description
  *          =================================================================== 
  *           This module manages the Specification for Integrated Circuit(s) 
  *             Cards Interface Revision 1.1
  *           This driver implements the following aspects of the specification:
  *             - Bulk Transfers 
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
#include "usbd_ccid_core.h"
    
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/ 
static uint8_t  USBD_CCID_Init (void  *pdev, 
                            uint8_t cfgidx);

static uint8_t  USBD_CCID_DeInit (void  *pdev, 
                              uint8_t cfgidx);

static uint8_t  USBD_CCID_Setup (void  *pdev, 
                             USB_SETUP_REQ *req);

static uint8_t  USBD_CCID_DataIn (void  *pdev, 
                              uint8_t epnum);


static uint8_t  USBD_CCID_DataOut (void  *pdev, 
                               uint8_t epnum);

static uint8_t  *USBD_CCID_GetCfgDesc (uint8_t speed, 
                                      uint16_t *length);

static uint8_t USBD_CCID_EP0_Buff[CCID_EP0_BUFF_SIZ] ;

USBD_Class_cb_TypeDef  USBD_CCID_cb = 
{
  USBD_CCID_Init,
  USBD_CCID_DeInit,
  USBD_CCID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_CCID_DataIn,
  USBD_CCID_DataOut,
  NULL, /*SOF */      
  USBD_CCID_GetCfgDesc,
};

/* USB Mass storage device Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t USBD_CCID_CfgDesc[SMARTCARD_SIZ_CONFIG_DESC] =
{
  
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,   /* bDescriptorType: Configuration */
  SMARTCARD_SIZ_CONFIG_DESC,
  
  0x00,
  0x01,   /* bNumInterfaces: 1 interface */
  0x01,   /* bConfigurationValue: */
  0x04,   /* iConfiguration: */
  0x80,         /*bmAttributes: bus powered */
  0x32,   /* MaxPower 100 mA */
  
  /********************  CCID **** interface ********************/
  0x09,   /* bLength: Interface Descriptor size */
  0x04,   /* bDescriptorType: */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x03,   /* bNumEndpoints: 3 endpoints used */
  0x0B,   /* bInterfaceClass: user's interface for CCID */
  0x00,   /* bInterfaceSubClass : */
  0x00,   /* nInterfaceProtocol : None */
  0x05,   /* iInterface: */

  /*******************  CCID class descriptor ********************/
  0x36,   /* bLength: CCID Descriptor size */
  0x21,	  /* bDescriptorType: Functional Descriptor type. */
  0x10,	  /* bcdCCID(LSB): CCID Class Spec release number (1.00) */
  0x01,   /* bcdCCID(MSB) */
  
  0x00,	  /* bMaxSlotIndex :highest available slot on this device */
  0x03,	  /* bVoltageSupport: bit Wise OR for 01h-5.0V 02h-3.0V
                                    04h 1.8V*/
  
  0x01,0x00,0x00,0x00,	/* dwProtocols: 0001h = Protocol T=0 */
  0x10,0x0E,0x00,0x00,	/* dwDefaultClock: 3.6Mhz = 3600kHz = 0x0E10, 
                             for 4 Mhz the value is (0x00000FA0) : 
                            This is used in ETU and waiting time calculations*/
  0x10,0x0E,0x00,0x00,	/* dwMaximumClock: Maximum supported ICC clock frequency 
                             in KHz. So, 3.6Mhz = 3600kHz = 0x0E10, 
                                           4 Mhz (0x00000FA0) : */
  0x00,			/* bNumClockSupported : no setting from PC 
                             If the value is 00h, the 
                            supported clock frequencies are assumed to be the 
                            default clock frequency defined by dwDefaultClock 
                            and the maximum clock frequency defined by 
                            dwMaximumClock */
  
  0xCD,0x25,0x00,0x00,	/* dwDataRate: Default ICC I/O data rate in bps
                            9677 bps = 0x25CD 
                            for example 10752 bps (0x00002A00) */
                             
  0xCD,0x25,0x00,0x00,	/* dwMaxDataRate: Maximum supported ICC I/O data 
                            rate in bps */
  0x00,                 /* bNumDataRatesSupported :
                         The number of data rates that are supported by the CCID
                         If the value is 00h, all data rates between the default 
                         data rate dwDataRate and the maximum data rate 
                         dwMaxDataRate are supported.
                         Dont support GET_CLOCK_FREQUENCIES
                        */     
				
  0x00,0x00,0x00,0x00,   /* dwMaxIFSD: 0 (T=0 only)   */
  0x00,0x00,0x00,0x00,   /* dwSynchProtocols  */
  0x00,0x00,0x00,0x00,   /* dwMechanical: no special characteristics */
  
  0x38,0x00,EXCHANGE_LEVEL_FEATURE,0x00,	 
                         /* dwFeatures: clk, baud rate, voltage : automatic */
                         /* 00000008h Automatic ICC voltage selection 
                         00000010h Automatic ICC clock frequency change
                         00000020h Automatic baud rate change according to 
                         active parameters provided by the Host or self 
                         determined 00000100h CCID can set 
                         ICC in clock stop mode      
                         
                         Only one of the following values may be present to 
                         select a level of exchange:
                         00010000h TPDU level exchanges with CCID
                         00020000h Short APDU level exchange with CCID
                         00040000h Short and Extended APDU level exchange 
                         If none of those values : character level of exchange*/
  0x0F,0x01,0x00,0x00,  /* dwMaxCCIDMessageLength: Maximum block size + header*/
                        /* 261 + 10   */
  
  0x00,			/* bClassGetResponse*/
  0x00,			/* bClassEnvelope */
  0x00,0x00,		/* wLcdLayout : 0000h no LCD. */
  0x00,			/* bPINSupport : no PIN verif and modif  */
  0x01,			/* bMaxCCIDBusySlots	*/

  /********************  CCID   Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  CCID_BULK_IN_EP,   /*Endpoint address (IN, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(CCID_BULK_EPIN_SIZE),
  HIBYTE(CCID_BULK_EPIN_SIZE),
  0x00,   /*Polling interval in milliseconds */
  
  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  CCID_BULK_OUT_EP,   /*Endpoint address (OUT, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(CCID_BULK_EPOUT_SIZE),
  HIBYTE(CCID_BULK_EPOUT_SIZE),
  0x00,   /*Polling interval in milliseconds*/
    
    
  0x07,   /*bLength: Endpoint Descriptor size*/
  0x05,   /*bDescriptorType:*/
  CCID_INTR_IN_EP,    /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,   /* bmAttributes: Interrupt endpoint */
  LOBYTE(CCID_INTR_EPIN_SIZE),
  HIBYTE(CCID_INTR_EPIN_SIZE),
  0x18    /*Polling interval in milliseconds */
};

/* Private function ----------------------------------------------------------*/ 

/**
  * @brief  USBD_CCID_Init
  *         Initialize  the USB CCID Interface 
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
static uint8_t  USBD_CCID_Init (void  *pdev, 
                            uint8_t cfgidx)
{

  DCD_PMA_Config(pdev , CCID_BULK_IN_EP,USB_SNG_BUF,CCID_BULK_TX_ADDRESS);
  DCD_PMA_Config(pdev , CCID_INTR_IN_EP,USB_SNG_BUF,CCID_INT_TX_ADDRESS);
  DCD_PMA_Config(pdev , CCID_BULK_OUT_EP,USB_SNG_BUF,CCID_BULK_RX_ADDRESS);

 
  /* Open EP IN */
  DCD_EP_Open(pdev,
              CCID_BULK_IN_EP,
              CCID_BULK_EPIN_SIZE,
              USB_EP_BULK);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              CCID_BULK_OUT_EP,
              CCID_BULK_EPOUT_SIZE,
              USB_EP_BULK);
 
    /* Open INTR EP IN */
  DCD_EP_Open(pdev,
              CCID_INTR_IN_EP,
              CCID_INTR_EPIN_SIZE,
              USB_EP_INT);
  
  /* Init the CCID  layer */
  CCID_Init(pdev); 
  
  return USBD_OK;
}

/**
  * @brief  USBD_CCID_DeInit
  *         DeInitilaize the usb ccid configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
static uint8_t  USBD_CCID_DeInit (void  *pdev, 
                              uint8_t cfgidx)
{
  /* Close CCID EPs */
  DCD_EP_Close (pdev , CCID_BULK_IN_EP);
  DCD_EP_Close (pdev , CCID_BULK_OUT_EP);
  DCD_EP_Close (pdev , CCID_INTR_IN_EP);
    
  /* Un Init the CCID layer */
  CCID_DeInit(pdev);   
  return USBD_OK;
}

/**
  * @brief  USBD_CCID_Setup
  *         Handle the CCID specific requests
  * @param  pdev: device instance
  * @param  req: USB request
  * @retval status
  */
static uint8_t  USBD_CCID_Setup (void  *pdev, USB_SETUP_REQ *req)
{
  uint8_t slot_nb;
  uint8_t seq_nb;
  uint16_t len;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  /* Class request */
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {
    case REQUEST_ABORT :

        if ((req->wLength == 0) &&
         ((req->bmRequest & 0x80) != 0x80))
        { /* Check bmRequest : No Data-In stage. 0x80 is Data Direction */
          
          /* The wValue field contains the slot number (bSlot) in the low byte 
          and the sequence number (bSeq) in the high byte.*/
          slot_nb = (uint8_t) ((req->wValue) & 0x00ff);
          seq_nb = (uint8_t) (((req->wValue) & 0xff00)>>8);
          
          if ( CCID_CmdAbort(slot_nb, seq_nb) != 0 )
          { /* If error is returned by lower layer : 
                 Generally Slot# may not have matched */
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
          }
        }
      else
      {
         USBD_CtlError(pdev , req);
         return USBD_FAIL; 
      }
      break;
      
    case REQUEST_GET_CLOCK_FREQUENCIES :
      if((req->wValue  == 0) && 
         (req->wLength != 0) &&
        ((req->bmRequest & 0x80) == 0x80))
        {   /* Check bmRequest : Data-In stage. 0x80 is Data Direction */
        
          if ( SC_Request_GetClockFrequencies(USBD_CCID_EP0_Buff, &len) != 0 )
          { /* If error is returned by lower layer */
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
          }
          else
          {
            if (len > CCID_EP0_BUFF_SIZ)
            {
              len = CCID_EP0_BUFF_SIZ;
            }
            
            USBD_CtlSendData (pdev,
                              USBD_CCID_EP0_Buff,
                              len);
          }          
      }
      else
      {
         USBD_CtlError(pdev , req);
         return USBD_FAIL; 
      }
      break;

      case REQUEST_GET_DATA_RATES :
      if((req->wValue  == 0) && 
         (req->wLength != 0) &&
        ((req->bmRequest & 0x80) == 0x80))
       {  /* Check bmRequest : Data-In stage. 0x80 is Data Direction */
         
          if ( SC_Request_GetDataRates(USBD_CCID_EP0_Buff, &len) != 0 )
          { /* If error is returned by lower layer */
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
          }
          else
          {
            if (len > CCID_EP0_BUFF_SIZ)
            {
              len = CCID_EP0_BUFF_SIZ;
            }
            
            USBD_CtlSendData (pdev,
                              USBD_CCID_EP0_Buff,
                              len);
          } 
      }
      else
      {
         USBD_CtlError(pdev , req);
         return USBD_FAIL; 
      }
      break;
      
    default:
       USBD_CtlError(pdev , req);
       return USBD_FAIL; 
    }
    break;
  /* Interface & Endpoint request */
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_INTERFACE :
      break;
      
    case USB_REQ_SET_INTERFACE :
      break;
    
    case USB_REQ_CLEAR_FEATURE:  

      /* Re-activate the EP */      
      DCD_EP_Close (pdev , (uint8_t)req->wIndex);
      if((((uint8_t)req->wIndex) & 0x80) == 0x80)
      {
        DCD_EP_Open(pdev,
                    ((uint8_t)req->wIndex),
                    CCID_BULK_EPIN_SIZE,
                    USB_EP_BULK);
      }
      else
      {
        DCD_EP_Open(pdev,
                    ((uint8_t)req->wIndex),
                    CCID_BULK_EPOUT_SIZE,
                    USB_EP_BULK);
      }
      
      break;
      
    }  
    break;
   
  default:
    break;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_CCID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CCID_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  CCID_BulkMessage_In(pdev , epnum);
  return USBD_OK;
}

/**
  * @brief  USBD_CCID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CCID_DataOut (void  *pdev, 
                               uint8_t epnum)
{
  CCID_BulkMessage_Out(pdev , epnum);
  return USBD_OK;
}

/**
  * @brief  USBD_CCID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CCID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_CCID_CfgDesc);
  return (uint8_t*)USBD_CCID_CfgDesc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
