/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application 
  *           
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
#include "LibUSB.h"
#include "NHC_WCH_SDI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/ 
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);

static uint8_t  USBD_HID_DataOut (void  *pdev, uint8_t epnum);

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */    
  USBD_HID_GetCfgDesc, 
};
      
static uint32_t  USBD_HID_AltSet = 0;
    
static uint32_t  USBD_HID_Protocol = 0;
 
static uint32_t  USBD_HID_IdleState  = 0;

uint8_t rxBuff[64];
uint8_t txBuff[64];

/* USB HID device Configuration Descriptor */
const uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0xfa,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0xff,         /*bInterfaceClass: HID*/
  0xff,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0xff,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x02,          /*bmAttributes: Interrupt endpoint*/
  0x40,//HID_IN_PACKET, /*wMaxPacketSize: 4 Byte max */
  0x00,
  0x01,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */
  0x07,	/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
			/*	Endpoint descriptor type */
  0x01,	/* bEndpointAddress: */
			/*	Endpoint Address (OUT) */
  0x02,	/* bmAttributes: Interrupt endpoint */
  0x40,	/* wMaxPacketSize: 2 Bytes max  */
  0x00,
  0x01,	/* bInterval: Polling Interval (20 ms) */
    /* 41 */
} ;

const uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] =
{
	0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01,             // Usage (Vendor Usage 1)
    0xA1, 0x01,             // Collection (Application)
    0x19, 0x01,             //      Usage Minimum 
    0x29, 0x40,             //      Usage Maximum 	//64 input usages total (0x01 to 0x40)
    0x15, 0x01,             //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x25, 0x40,      	  	//      Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08,             //      Report Size: 8-bit field size
    0x95, 0x40,             //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00,             //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
    0x19, 0x01,             //      Usage Minimum 
    0x29, 0x40,             //      Usage Maximum 	//64 output usages total (0x01 to 0x40)
    0x91, 0x00,             //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
    0xC0
}; 

/* Private function ----------------------------------------------------------*/ 
/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  DCD_PMA_Config(pdev , HID_IN_EP,USB_SNG_BUF,HID_IN_TX_ADDRESS);
  DCD_PMA_Config(pdev , HID_OUT_EP,USB_SNG_BUF,HID_IN_TX_ADDRESS+0x40);
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              /*HID_IN_PACKET*/0x40,
              USB_EP_BULK);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              /*HID_OUT_PACKET*/0x40,
              USB_EP_BULK);
  DCD_EP_PrepareRx(pdev, HID_OUT_EP, rxBuff, 0x40);
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = (uint8_t *)HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        pbuf = (uint8_t *)USBD_HID_CfgDesc + 0x12;
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return (uint8_t *)USBD_HID_CfgDesc;
}

static uint8_t  USBD_HID_DataIn (void  *pdev,
                              uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  //DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

uint8_t u8Buff[4 * 1024];
#define WCH_SDI_INIT 0xA0
#define WCH_SDI_EXIT 0xA1
#define WCH_SDI_READ 0xA2
#define WCH_SDI_WRITE 0xA3
#define WCH_SDI_READ_MEM 0xA4
#define WCH_SDI_WRITE_MEM 0xA5

/**
  * @brief  USBD_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

uint32_t u32Check = 1;

#define busy_on GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define busy_off GPIO_ResetBits(GPIOA, GPIO_Pin_1)

uint32_t u8tou32(uint8_t *p)
{
    uint32_t u32Tmp;

    u32Tmp = p[3];
    u32Tmp <<= 8;
    u32Tmp += p[2];
    u32Tmp <<= 8;
    u32Tmp += p[1];
    u32Tmp <<= 8;
    u32Tmp += p[0];

    return u32Tmp;
}

#define BUSY_1() GPIO_ResetBits(GPIOC, GPIO_Pin_9)
#define BUSY_0() GPIO_SetBits(GPIOC, GPIO_Pin_9)

void u32tou8(uint32_t u32In, uint8_t *p)
{
    p[0] = u32In;
    u32In >>= 8;
    p[1] = u32In;
    u32In >>= 8;
    p[2] = u32In;
    u32In >>= 8;
    p[3] = u32In;
}

static uint8_t  USBD_HID_DataOut (void  *pdev,
                              uint8_t epnum)
{
	uint32_t u32Address;
    //uint32_t u32Len;
    uint32_t u32Tmp;
    //uint32_t *pu32;
    //uint32_t i;
    //uint32_t n;
    //uint8_t *p;
	
	busy_on;
	
	switch (rxBuff[0]) {
		case WCH_SDI_INIT:
			NHC_WchSdi_Init();
			break;
		case WCH_SDI_EXIT:
			NHC_WchSdi_Exit();
			break;
		case WCH_SDI_READ:
			u32Tmp = u8tou32(rxBuff + 2);
			txBuff[0] = NHC_WchSdi_Read(rxBuff[1], &u32Tmp);
			u32tou8(u32Tmp, txBuff + 1);
			DCD_EP_Tx(pdev, HID_IN_EP, txBuff, 0x40);
			break;
		case WCH_SDI_WRITE:
			u32Tmp = u8tou32(rxBuff + 2);
			NHC_WchSdi_Write(rxBuff[1], u32Tmp);
			break;
		case WCH_SDI_READ_MEM:
			u32Address = u8tou32(rxBuff + 1);
			txBuff[0] = NHC_WchSdi_ReadMem(u32Address, &u32Tmp, rxBuff[5]);
			u32tou8(u32Tmp, txBuff + 1);
			DCD_EP_Tx(pdev, HID_IN_EP, txBuff, 0x40);
			break;
		case WCH_SDI_WRITE_MEM:
			u32Address = u8tou32(rxBuff + 1);
			u32Tmp = u8tou32(rxBuff + 5);
			NHC_WchSdi_WriteMem(u32Address, u32Tmp, rxBuff[9]);
			break;
	}
	
	busy_off;
	
	DCD_EP_PrepareRx(pdev, HID_OUT_EP, rxBuff, 0x40);
	return USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
