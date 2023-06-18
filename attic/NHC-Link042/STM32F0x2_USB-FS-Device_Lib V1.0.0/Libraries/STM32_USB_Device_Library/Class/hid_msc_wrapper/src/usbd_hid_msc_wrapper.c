/**
  ******************************************************************************
  * @file    usbd_msc_hid_wrapper.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file calls to the separate MSC and HID class layer handlers.
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
  *****************************************************************************/ 
  /*      
  *    ===================================================================      
  *                          composite MSC_HID
  *    =================================================================== */     

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_msc_wrapper.h"
#include "usbd_custom_hid_core.h"
#include "usbd_msc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t  USBD_HID_MSC_Init         (void *pdev , uint8_t cfgidx);
static uint8_t  USBD_HID_MSC_DeInit       (void *pdev , uint8_t cfgidx);

/* Control Endpoints*/
static uint8_t  USBD_HID_MSC_Setup        (void *pdev , USB_SETUP_REQ  *req);  
static uint8_t  USBD_HID_MSC_EP0_RxReady  (void *pdev);  

/* Class Specific Endpoints*/
static uint8_t  USBD_HID_MSC_DataIn       (void *pdev , uint8_t epnum);   
static uint8_t  USBD_HID_MSC_DataOut      (void *pdev , uint8_t epnum); 

static uint8_t*  USBD_HID_MSC_GetConfigDescriptor( uint8_t speed , uint16_t *length); 

USBD_Class_cb_TypeDef  USBD_HID_MSC_cb = 
{
  USBD_HID_MSC_Init,
  USBD_HID_MSC_DeInit,
  USBD_HID_MSC_Setup,
  NULL, 
  USBD_HID_MSC_EP0_RxReady, 
  USBD_HID_MSC_DataIn, 
  USBD_HID_MSC_DataOut,
  NULL,   
  USBD_HID_MSC_GetConfigDescriptor, 
};


/* USB MSC_HID device Configuration Descriptor */
const uint8_t USBD_HID_MSC_CfgDesc[USB_HID_MSC_CONFIG_DESC_SIZ] =
{  
  0x09, /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
  USB_HID_MSC_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x02,         /*bNumInterfaces: 2 interfaces (1 for MSC, 1 for HID)*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of CUSTOM HID interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
  HID_INTERFACE,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of CUSTOM HID  **************************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  CUSTOMHID_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of CUSTOM HID endpoint *********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  
  HID_IN_EP,      /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_IN_PACKET, /*wMaxPacketSize: 2 Byte max */
  0x00,
  0x20,          /*bInterval: Polling Interval (32 ms)*/
  /* 34 */
  
  0x07,	/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
  /*	Endpoint descriptor type */
  HID_OUT_EP,	/* bEndpointAddress: */
  /*	Endpoint Address (OUT) */
  0x03,	/* bmAttributes: Interrupt endpoint */
  HID_OUT_PACKET,	/* wMaxPacketSize: 2 Bytes max  */
  0x00,
  0x20,	/* bInterval: Polling Interval (20 ms) */
  /* 41 */
  
  /********************  Mass Storage interface ********************/
  0x09,   /* bLength: Interface Descriptor size */
  0x04,   /* bDescriptorType: */
  MSC_INTERFACE,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints*/
  0x08,   /* bInterfaceClass: MSC Class */
  0x06,   /* bInterfaceSubClass : SCSI transparent*/
  0x50,   /* nInterfaceProtocol */
  0X01,          /* iInterface: */
  /********************  Mass Storage Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  MSC_IN_EP,   /*Endpoint address (IN, address 2) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MSC_MAX_PACKET),
  HIBYTE(MSC_MAX_PACKET),
  0x00,   /*Polling interval in milliseconds */
  
  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  MSC_OUT_EP,   /*Endpoint address (OUT, address 2) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MSC_MAX_PACKET),
  HIBYTE(MSC_MAX_PACKET),
  0x00     /*Polling interval in milliseconds*/
     
}; /* USBD_HID_MSC_CfgDesc */

/* Private function prototypes -----------------------------------------------*/
/*********************************************
   MSC Device library callbacks
*********************************************/
extern uint8_t  USBD_MSC_Init (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_MSC_DeInit (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_MSC_Setup (void  *pdev, USB_SETUP_REQ *req);
extern uint8_t  USBD_MSC_DataIn (void  *pdev, uint8_t epnum);
extern uint8_t  USBD_MSC_DataOut (void  *pdev,  uint8_t epnum);
extern uint8_t  *USBD_MSC_GetCfgDesc (uint8_t speed, uint16_t *length);
extern uint8_t  USBD_MSC_CfgDesc[USB_MSC_CONFIG_DESC_SIZ];

/*********************************************
   HID Device library callbacks
*********************************************/
extern uint8_t  USBD_HID_Init (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_HID_DeInit (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_HID_Setup (void  *pdev, USB_SETUP_REQ *req);
extern uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);
extern uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);
extern uint8_t  USBD_HID_DataOut (void  *pdev, uint8_t epnum);
extern uint8_t  USBD_HID_EP0_RxReady (void  *pdev);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USBD_HID_MSC_Init
  *         Initialize the HID & MSC interfaces
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_MSC_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  /* HID initialization */
  USBD_HID_Init (pdev,cfgidx);
  
  /* MSC initialization */
  USBD_MSC_Init (pdev,cfgidx);
  
  return USBD_OK;

}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID/CDC interfaces
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_MSC_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* HID De-initialization */
  USBD_HID_DeInit (pdev,cfgidx);
  
  /* CDC De-initialization */
  USBD_MSC_DeInit (pdev,cfgidx);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_MSC_Setup (void  *pdev, 
                                    USB_SETUP_REQ *req)
{
  switch (req->bmRequest & USB_REQ_RECIPIENT_MASK)
  {
  case USB_REQ_RECIPIENT_INTERFACE:
    if (req->wIndex == HID_INTERFACE)
    {
      return (USBD_HID_Setup (pdev, req));
    }
    else
    {
      return (USBD_MSC_Setup(pdev, req));
    }
  case USB_REQ_RECIPIENT_ENDPOINT:
    if ((req->wIndex == HID_IN_EP) || (req->wIndex == HID_OUT_EP))
    {
      return (USBD_HID_Setup (pdev, req));   
    }
    else
    {
      return (USBD_MSC_Setup(pdev, req));
    }
  }   
  return USBD_OK;
}

/**
  * @brief  USBD_HID_MSC_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_HID_MSC_GetConfigDescriptor (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_MSC_CfgDesc);
  return (uint8_t*)USBD_HID_MSC_CfgDesc;
}

/**
  * @brief  USBD_HID_MSC_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_MSC_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  /*DataIN can be for MSC or HID */
  
  if (epnum == (MSC_IN_EP&~0x80) )
  {
    return (USBD_MSC_DataIn(pdev, epnum));
  }
  else
  {
   return (USBD_HID_DataIn(pdev, epnum));
  }
}

/**
  * @brief  USBD_HID_MSC_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_MSC_DataOut(void *pdev , uint8_t epnum)
{
  /*DataOut can be for MSC or HID */
  
  if (epnum == (MSC_OUT_EP&~0x80) )
  {
    return (USBD_MSC_DataOut(pdev, epnum));
  }
  else
  {
    return (USBD_HID_DataOut(pdev, epnum));
  } 
}

/**
  * @brief  USBD_HID_MSC_EP0_RxReady
  *         handle RxReady processing
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_HID_MSC_EP0_RxReady  (void *pdev)
{
  /*RxReady processing needed for Custom HID only*/
  return (USBD_HID_EP0_RxReady(pdev));
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
