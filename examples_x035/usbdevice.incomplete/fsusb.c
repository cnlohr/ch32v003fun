#include "fsusb.h"
#include "ch32v003fun.h"
#include <string.h>

uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;


// based on https://github.com/openwch/ch32x035/tree/main/EVT/EXAM/USB/USBFS/DEVICE/CompositeKM/User

#define DEF_USBD_UEP0_SIZE		   64	 /* usb hs/fs device end-point 0 size */
#define EP_SIZE 64

#define DEF_UEP_IN                    0x80
#define DEF_UEP_OUT                   0x00

#define DEF_UEP0 0
#define DEF_UEP1 1
#define DEF_UEP2 2
#define DEF_UEP3 3
#define DEF_UEP4 4
#define DEF_UEP5 5
#define DEF_UEP6 6
#define DEF_UEP7 7
#define NUM_EP 8

#define CRB_U_IS_NAK	 (1<<7)
#define CTOG_MATCH_SYNC  (1<<6)
#define CRB_U_SIE_FREE   (1<<5)
#define CRB_UIF_FIFO_OV  (1<<4)
#define CRB_UIF_HST_SOF  (1<<3)
#define CRB_UIF_SUSPEND  (1<<2)
#define CRB_UIF_TRANSFER (1<<1)
#define CRB_UIF_BUS_RST  (1<<0)
#define CSETUP_ACT	   (1<<15)
#define CRB_UIS_TOG_OK   (1<<14)
#define CMASK_UIS_TOKEN  (3<<12)
#define CMASK_UIS_ENDP   (0xf<<8)

#define CUSBFS_UIS_ENDP_MASK		 0x0

#define CUIS_TOKEN_OUT		0x0
#define CUIS_TOKEN_SOF		0x1
#define CUIS_TOKEN_IN		 0x2
#define CUIS_TOKEN_SETUP	  0x3


// Hardware specific

/* R8_USB_CTRL */
#define USBFS_UC_HOST_MODE          0x80
#define USBFS_UC_LOW_SPEED          0x40
#define USBFS_UC_DEV_PU_EN          0x20
#define USBFS_UC_SYS_CTRL_MASK      0x30
#define USBFS_UC_SYS_CTRL0          0x00
#define USBFS_UC_SYS_CTRL1          0x10
#define USBFS_UC_SYS_CTRL2          0x20
#define USBFS_UC_SYS_CTRL3          0x30
#define USBFS_UC_INT_BUSY           0x08
#define USBFS_UC_RESET_SIE          0x04
#define USBFS_UC_CLR_ALL            0x02
#define USBFS_UC_DMA_EN             0x01

/* R8_USB_INT_EN */
#define USBFS_UIE_DEV_SOF           0x80
#define USBFS_UIE_DEV_NAK           0x40
#define USBFS_UIE_FIFO_OV           0x10
#define USBFS_UIE_HST_SOF           0x08
#define USBFS_UIE_SUSPEND           0x04
#define USBFS_UIE_TRANSFER          0x02
#define USBFS_UIE_DETECT            0x01
#define USBFS_UIE_BUS_RST           0x01

/* R8_USB_DEV_AD */
#define USBFS_UDA_GP_BIT            0x80
#define USBFS_USB_ADDR_MASK         0x7F

/* R8_USB_MIS_ST */
#define USBFS_UMS_SOF_PRES          0x80
#define USBFS_UMS_SOF_ACT           0x40
#define USBFS_UMS_SIE_FREE          0x20
#define USBFS_UMS_R_FIFO_RDY        0x10
#define USBFS_UMS_BUS_RESET         0x08
#define USBFS_UMS_SUSPEND           0x04
#define USBFS_UMS_DM_LEVEL          0x02
#define USBFS_UMS_DEV_ATTACH        0x01

// XXX TODO: Put these in a struct.

/* Setup Request */
volatile uint8_t  USBFS_SetupReqCode;
volatile uint8_t  USBFS_SetupReqType;
volatile uint16_t USBFS_SetupReqValue;
volatile uint16_t USBFS_SetupReqIndex;
volatile uint16_t USBFS_SetupReqLen;

/* USB Device Status */
volatile uint8_t  USBFS_DevConfig;
volatile uint8_t  USBFS_DevAddr;
volatile uint8_t  USBFS_DevSleepStatus;
volatile uint8_t  USBFS_DevEnumStatus;

/* Endpoint Buffer */
__attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[64];
__attribute__ ((aligned(4))) uint8_t USBFS_EP1_Buf[64];
__attribute__ ((aligned(4))) uint8_t USBFS_EP2_Buf[64];
__attribute__ ((aligned(4))) uint8_t USBFS_EP0_4Buf[ DEF_USBD_UEP0_SIZE*2 ];  //ep0_4(64)

int USBFS_HidIdle[2];
int USBFS_HidProtocol[2];

const uint8_t  *pUSBFS_Descr;

/* USB IN Endpoint Busy Flag */
volatile uint8_t  USBFS_Endp_Busy[NUM_EP];




void USBFS_IRQHandler() __attribute__((section(".text.vector_handler")))  __attribute__((interrupt));



void USBFS_IRQHandler()
{
	// Based on https://github.com/openwch/ch32x035/blob/main/EVT/EXAM/USB/USBFS/DEVICE/CompositeKM/User/ch32x035_usbfs_device.c

	// Combined FG + ST flag.
	uint16_t intfgst = *(uint16_t*)(&USBFS->INT_FG);
	int len = 0, errflag = 0;

	GPIOA->BSHR = 1;

	// TODO: Check if needs to be do-while to re-check.
	if( intfgst & CRB_UIF_TRANSFER )
	{
		int token = ( intfgst & CMASK_UIS_TOKEN) >> 12;
		int ep = intfgst & CUSBFS_UIS_ENDP_MASK;
		switch ( token )
		{
		case CUIS_TOKEN_IN:
			switch( ep )
			{
				/* end-point 0 data in interrupt */
				case DEF_UEP0:
					if( USBFS_SetupReqLen == 0 )
					{
						USBFS->UEP0_CTRL_H = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
					}

					if ( ( USBFS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
					{
						/* Non-standard request endpoint 0 Data upload */
					}
					else
					{
						switch( USBFS_SetupReqCode )
						{
							case USB_GET_DESCRIPTOR:
								len = USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
								memcpy( USBFS_EP0_Buf, pUSBFS_Descr, len );
								USBFS_SetupReqLen -= len;
								pUSBFS_Descr += len;
								USBFS->UEP0_TX_LEN = len;
								USBFS->UEP0_CTRL_H ^= USBFS_UEP_T_TOG;
								break;

							case USB_SET_ADDRESS:
								USBFS->DEV_ADDR = ( USBFS->DEV_ADDR & USBFS_UDA_GP_BIT ) | USBFS_DevAddr;
								break;

							default:
								break;
						}
					}
					break;

				/* end-point 1 data in interrupt */
				case DEF_UEP1:
					USBFS->UEP1_CTRL_H = ( USBFS->UEP1_CTRL_H & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_NAK;
					USBFS->UEP1_CTRL_H ^= USBFS_UEP_T_TOG;
					USBFS_Endp_Busy[ DEF_UEP1 ] = 0;
					break;

				/* end-point 2 data in interrupt */
				case DEF_UEP2:
					USBFS->UEP2_CTRL_H = ( USBFS->UEP2_CTRL_H & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_NAK;
					USBFS->UEP2_CTRL_H ^= USBFS_UEP_T_TOG;
					USBFS_Endp_Busy[ DEF_UEP2 ] = 0;
					break;

				default :
					break;
			}
			break;

		/* data-out stage processing */
		case CUIS_TOKEN_OUT:
			switch( ep )
			{
				/* end-point 0 data out interrupt */
				case DEF_UEP0:
					if( intfgst & CRB_UIS_TOG_OK )
					{
						if( ( USBFS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
						{
							if( ( USBFS_SetupReqType & USB_REQ_TYP_MASK ) == USB_REQ_TYP_CLASS )
							{
								switch( USBFS_SetupReqCode )
								{
									case HID_SET_REPORT:
										//KB_LED_Cur_Status = USBFS_EP0_Buf[ 0 ];
										// Do stuff...
										USBFS_SetupReqLen = 0;
										break;
									default:
										break;
								}
							}
						}
						else
						{
							/* Standard request end-point 0 Data download */
							/* Add your code here */
						}
					}
					if( USBFS_SetupReqLen == 0 )
					{
						USBFS->UEP0_TX_LEN  = 0;
						USBFS->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
					}
					break;

				default:
					break;
			}
			break;

		/* Setup stage processing */
		case CUIS_TOKEN_SETUP:
			USBFS->UEP0_CTRL_H = USBFS_UEP_T_TOG|USBFS_UEP_T_RES_NAK|USBFS_UEP_R_TOG|USBFS_UEP_R_RES_NAK;


			/* Store All Setup Values */
			USBFS_SetupReqType  = pUSBFS_SetupReqPak->bRequestType;
			USBFS_SetupReqCode  = pUSBFS_SetupReqPak->bRequest;
			USBFS_SetupReqLen   = pUSBFS_SetupReqPak->wLength;
			USBFS_SetupReqValue = pUSBFS_SetupReqPak->wValue;
			USBFS_SetupReqIndex = pUSBFS_SetupReqPak->wIndex;
			len = 0;
			errflag = 0;

			if( ( USBFS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
			{
				if( ( USBFS_SetupReqType & USB_REQ_TYP_MASK ) == USB_REQ_TYP_CLASS )
				{
					/* Class Request */
					switch( USBFS_SetupReqCode )
					{
						case HID_SET_REPORT:
							break;

						case HID_SET_IDLE:
							if( USBFS_SetupReqIndex == 0x00 )
							{
								USBFS_HidIdle[ 0 ] = (uint8_t)( USBFS_SetupReqValue >> 8 );
							}
							else if( USBFS_SetupReqIndex == 0x01 )
							{
								USBFS_HidIdle[ 1 ] = (uint8_t)( USBFS_SetupReqValue >> 8 );
							}
							else
							{
								errflag = 0xFF;
							}
							break;

						case HID_SET_PROTOCOL:
							if( USBFS_SetupReqIndex == 0x00 )
							{
								USBFS_HidProtocol[ 0 ] = (uint8_t)USBFS_SetupReqValue;
							}
							else if( USBFS_SetupReqIndex == 0x01 )
							{
								USBFS_HidProtocol[ 1 ] = (uint8_t)USBFS_SetupReqValue;
							}
							else
							{
								errflag = 0xFF;
							}
							break;

						case HID_GET_IDLE:
							if( USBFS_SetupReqIndex == 0x00 )
							{
								USBFS_EP0_Buf[ 0 ] = USBFS_HidIdle[ 0 ];
								len = 1;
							}
							else if( USBFS_SetupReqIndex == 0x01 )
							{
								USBFS_EP0_Buf[ 0 ] = USBFS_HidIdle[ 1 ];
								len = 1;
							}
							else
							{
								errflag = 0xFF;
							}
							break;

						case HID_GET_PROTOCOL:
							if( USBFS_SetupReqIndex == 0x00 )
							{
								USBFS_EP0_Buf[ 0 ] = USBFS_HidProtocol[ 0 ];
								len = 1;
							}
							else if( USBFS_SetupReqIndex == 0x01 )
							{
								USBFS_EP0_Buf[ 0 ] = USBFS_HidProtocol[ 1 ];
								len = 1;
							}
							else
							{
								errflag = 0xFF;
							}
							break;

						default:
							errflag = 0xFF;
							break;
					}
				}
			}
			else
			{
				/* usb standard request processing */
				switch( USBFS_SetupReqCode )
				{
					/* get device/configuration/string/report/... descriptors */
					case USB_GET_DESCRIPTOR:
						switch( (uint8_t)( USBFS_SetupReqValue >> 8 ) )
						{
							/* get usb device descriptor */
							case USB_DESCR_TYP_DEVICE:
								pUSBFS_Descr = MyDevDescr;
								len = DEF_USBD_DEVICE_DESC_LEN;
								break;

							/* get usb configuration descriptor */
							case USB_DESCR_TYP_CONFIG:
								pUSBFS_Descr = MyCfgDescr;
								len = DEF_USBD_CONFIG_DESC_LEN;
								break;

							/* get usb hid descriptor */
							case USB_DESCR_TYP_HID:
								if( USBFS_SetupReqIndex == 0x00 )
								{
									pUSBFS_Descr = &MyCfgDescr[ 18 ];
									len = 9;
								}
								else if( USBFS_SetupReqIndex == 0x01 )
								{
									pUSBFS_Descr = &MyCfgDescr[ 43 ];
									len = 9;
								}
								else
								{
									errflag = 0xFF;
								}
								break;

							/* get usb report descriptor */
							case USB_DESCR_TYP_REPORT:
								if( USBFS_SetupReqIndex == 0x00 )
								{
									pUSBFS_Descr = KeyRepDesc;
									len = DEF_USBD_REPORT_DESC_LEN_KB;
								}
								else if( USBFS_SetupReqIndex == 0x01 )
								{
									pUSBFS_Descr = MouseRepDesc;
									len = DEF_USBD_REPORT_DESC_LEN_MS;
								}
								else
								{
									errflag = 0xFF;
								}
								break;

							/* get usb string descriptor */
							case USB_DESCR_TYP_STRING:
								switch( (uint8_t)( USBFS_SetupReqValue & 0xFF ) )
								{
									/* Descriptor 0, Language descriptor */
									case DEF_STRING_DESC_LANG:
										pUSBFS_Descr = MyLangDescr;
										len = DEF_USBD_LANG_DESC_LEN;
										break;

									/* Descriptor 1, Manufacturers String descriptor */
									case DEF_STRING_DESC_MANU:
										pUSBFS_Descr = MyManuInfo;
										len = DEF_USBD_MANU_DESC_LEN;
										break;

									/* Descriptor 2, Product String descriptor */
									case DEF_STRING_DESC_PROD:
										pUSBFS_Descr = MyProdInfo;
										len = DEF_USBD_PROD_DESC_LEN;
										break;

									/* Descriptor 3, Serial-number String descriptor */
									case DEF_STRING_DESC_SERN:
										pUSBFS_Descr = MySerNumInfo;
										len = DEF_USBD_SN_DESC_LEN;
										break;

									default:
										errflag = 0xFF;
										break;
								}
								break;

							default :
								errflag = 0xFF;
								break;
						}

						/* Copy Descriptors to Endp0 DMA buffer */
						if( USBFS_SetupReqLen > len )
						{
							USBFS_SetupReqLen = len;
						}
						len = ( USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE ) ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
						memcpy( USBFS_EP0_Buf, pUSBFS_Descr, len );
						pUSBFS_Descr += len;
						break;

					/* Set usb address */
					case USB_SET_ADDRESS:
						USBFS_DevAddr = (uint8_t)( USBFS_SetupReqValue & 0xFF );
						break;

					/* Get usb configuration now set */
					case USB_GET_CONFIGURATION:
						USBFS_EP0_Buf[ 0 ] = USBFS_DevConfig;
						if( USBFS_SetupReqLen > 1 )
						{
							USBFS_SetupReqLen = 1;
						}
						break;

					/* Set usb configuration to use */
					case USB_SET_CONFIGURATION:
						USBFS_DevConfig = (uint8_t)( USBFS_SetupReqValue & 0xFF );
						USBFS_DevEnumStatus = 0x01;
						break;

					/* Clear or disable one usb feature */
					case USB_CLEAR_FEATURE:
						if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
						{
							/* clear one device feature */
							if( (uint8_t)( USBFS_SetupReqValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
							{
								/* clear usb sleep status, device not prepare to sleep */
								USBFS_DevSleepStatus &= ~0x01;
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							if( (uint8_t)( USBFS_SetupReqValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
							{
								/* Clear End-point Feature */
								switch( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) )
								{
									case ( DEF_UEP_IN | DEF_UEP1 ):
										/* Set End-point 1 OUT ACK */
										USBFS->UEP1_CTRL_H = USBFS_UEP_T_RES_NAK;
										break;

									case ( DEF_UEP_IN | DEF_UEP2 ):
										/* Set End-point 2 IN NAK */
										USBFS->UEP2_CTRL_H = USBFS_UEP_T_RES_NAK;
										break;

									default:
										errflag = 0xFF;
										break;
								}
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else
						{
							errflag = 0xFF;
						}
						break;

					/* set or enable one usb feature */
					case USB_SET_FEATURE:
						if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
						{
							/* Set Device Feature */
							if( (uint8_t)( USBFS_SetupReqValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
							{
								if( MyCfgDescr[ 7 ] & 0x20 )
								{
									/* Set Wake-up flag, device prepare to sleep */
									USBFS_DevSleepStatus |= 0x01;
								}
								else
								{
									errflag = 0xFF;
								}
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							/* Set Endpoint Feature */
							if( (uint8_t)( USBFS_SetupReqValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
							{
								switch( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) )
								{
									case ( DEF_UEP_IN | DEF_UEP1 ):
										USBFS->UEP1_CTRL_H = ( USBFS->UEP1_CTRL_H & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_STALL;
										break;

									case ( DEF_UEP_IN | DEF_UEP2 ):
										USBFS->UEP2_CTRL_H = ( USBFS->UEP2_CTRL_H & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_STALL;
										break;

									default:
										errflag = 0xFF;
										break;
								}
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else
						{
							errflag = 0xFF;
						}
						break;

					/* This request allows the host to select another setting for the specified interface  */
					case USB_GET_INTERFACE:
						USBFS_EP0_Buf[ 0 ] = 0x00;
						if( USBFS_SetupReqLen > 1 )
						{
							USBFS_SetupReqLen = 1;
						}
						break;

					case USB_SET_INTERFACE:
						break;

					/* host get status of specified device/interface/end-points */
					case USB_GET_STATUS:
						USBFS_EP0_Buf[ 0 ] = 0x00;
						USBFS_EP0_Buf[ 1 ] = 0x00;
						if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
						{
							if( USBFS_DevSleepStatus & 0x01 )
							{
								USBFS_EP0_Buf[ 0 ] = 0x02;
							}
							else
							{
								USBFS_EP0_Buf[ 0 ] = 0x00;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							if( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) == ( DEF_UEP_IN | DEF_UEP1 ) )
							{
								if( ( USBFS->UEP1_CTRL_H & USBFS_UEP_T_RES_MASK ) == USBFS_UEP_T_RES_STALL )
								{
									USBFS_EP0_Buf[ 0 ] = 0x01;
								}
							}
							else if( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) == ( DEF_UEP_IN | DEF_UEP2 ) )
							{
								if( ( USBFS->UEP2_CTRL_H & USBFS_UEP_T_RES_MASK ) == USBFS_UEP_T_RES_STALL )
								{
									USBFS_EP0_Buf[ 0 ] = 0x01;
								}
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else
						{
							errflag = 0xFF;
						}
						if( USBFS_SetupReqLen > 2 )
						{
							USBFS_SetupReqLen = 2;
						}
						break;

					default:
						errflag = 0xFF;
						break;
				}
			}

			/* errflag = 0xFF means a request not support or some errors occurred, else correct */
			if( errflag == 0xFF )
			{
				/* if one request not support, return stall */
				USBFS->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_STALL|USBFS_UEP_R_TOG | USBFS_UEP_R_RES_STALL;
			}
			else
			{
				/* end-point 0 data Tx/Rx */
				if( USBFS_SetupReqType & DEF_UEP_IN )
				{
					len = ( USBFS_SetupReqLen > DEF_USBD_UEP0_SIZE )? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
					USBFS_SetupReqLen -= len;
					USBFS->UEP0_TX_LEN = len;
					USBFS->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
				}
				else
				{
					if( USBFS_SetupReqLen == 0 )
					{
						USBFS->UEP0_TX_LEN = 0;
						USBFS->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
					}
					else
					{
						USBFS->UEP0_CTRL_H = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
					}
				}
			}
			break;

		/* Sof pack processing */
		case CUIS_TOKEN_SOF:
			break;

		default :
			break;
		}
	}
	else if( intfgst & CRB_UIF_BUS_RST )
	{
		/* usb reset interrupt processing */
		USBFS_DevConfig = 0;
		USBFS_DevAddr = 0;
		USBFS_DevSleepStatus = 0;
		USBFS_DevEnumStatus = 0;

		USBFS->DEV_ADDR = 0;
		USBFS_Device_Endp_Init( );
	}
	else if( intfgst & CRB_UIF_SUSPEND )
	{
		/* usb suspend interrupt processing */
		if( USBFS->MIS_ST & USBFS_UMS_SUSPEND )
		{
			USBFS_DevSleepStatus |= 0x02;
			if( USBFS_DevSleepStatus == 0x03 )
			{
				/* Handling usb sleep here */
				//TODO: MCU_Sleep_Wakeup_Operate( );
			}
		}
		else
		{
			USBFS_DevSleepStatus &= ~0x02;
		}
	}

	// Handle any other interrupts and just clear them out.
	*(uint16_t*)(&USBFS->INT_FG) = intfgst;

	//intfgst = *(uint16_t*)(&USBFS->INT_FG);
	GPIOA->BSHR = 1<<16;
}

void USBFS_Device_Endp_Init()
{
	USBFS->UEP4_1_MOD = EP_SIZE;
	USBFS->UEP2_3_MOD = EP_SIZE;
	USBFS->UEP567_MOD = 0;

	USBFS->UEP0_DMA = (intptr_t)USBFS_EP0_Buf;
	USBFS->UEP1_DMA = (intptr_t)USBFS_EP1_Buf;
	USBFS->UEP2_DMA = (intptr_t)USBFS_EP2_Buf;

	USBFS->UEP0_CTRL_H = USBFS_UEP_R_RES_ACK | USBFS_UEP_T_RES_NAK;
	USBFS->UEP1_CTRL_H = USBFS_UEP_T_RES_NAK;
	USBFS->UEP2_CTRL_H = USBFS_UEP_T_RES_NAK;

    /* Clear End-points Busy Status */
    for(uint8_t i=0; i< sizeof(USBFS_Endp_Busy)/sizeof(USBFS_Endp_Busy[0]); i++ )
    {
        USBFS_Endp_Busy[ i ] = 0;
    }
}


void USBFS_Poll()
{
	USBDEBUG2 = USBFS->INT_ST;//EP0_DATA[1];
	USBDEBUG1 = USBFS->MIS_ST;
}

int FSUSBSetup()
{
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
	RCC->AHBPCENR |= RCC_USBFS;

	NVIC_EnableIRQ( USBFS_IRQn );

	AFIO->CTLR |= USB_PHY_V33;

	USBFS->BASE_CTRL = RB_UC_RESET_SIE | RB_UC_CLR_ALL;
	USBFS->BASE_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN;

	// Enter device mode.
	USBFS->INT_EN = RB_UIE_SUSPEND | RB_UIE_TRANSFER | RB_UIE_BUS_RST;
	USBFS->DEV_ADDR = 0x00;
	USBFS->UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;

	USBFS_Device_Endp_Init();


	// Go on-bus.

	// From the TRM:
	//
	//   USB multiplexing IO pins enable:
	//
	// Enabling USB requires, in addition to USB_IOEN set to
	// 1, the setting of: MODE=0 in GPIO configuration register
	// GPIOC_CFGXR corresponding to PC16 and PC17 to
	// select the input mode.
	//
	// for USB device, CNF=10 corresponding to PC17 to select
	// the input mode with up and down For USB devices, PC17
	// corresponding to CNF=10 selects the input mode with up
	// and down pull, PC17 corresponding to bit 1 in
	// GPIOC_OUTDR selects the up pull, and PC16
	// corresponding to CNF=01 selects the floating input.


	AFIO->CTLR = (AFIO->CTLR & ~(UDP_PUE_11 | UDM_PUE_11 )) | USB_PHY_V33 | USB_IOEN | UDP_PUE_11; //1.5k pullup

	// Enable PC16/17 Alternate Function (USB)
	// According to EVT, GPIO16 = GPIO_Mode_IN_FLOATING, GPIO17 = GPIO_Mode_IPU
	GPIOC->CFGXR &= ~(0xf<<(4*0));
	GPIOC->CFGXR |= (GPIO_CFGLR_IN_FLOAT)<<(4*0); // MSBs are CNF, LSBs are MODE
	GPIOC->CFGXR &= ~(0xf<<(4*1));
	GPIOC->CFGXR |= (GPIO_CFGLR_IN_PUPD)<<(4*1);
	GPIOC->BSXR = 1<<1; // PC17 on.

	USBFS->UDEV_CTRL = RB_UD_PORT_EN;

	// Go on-bus.
	return 0;
}

