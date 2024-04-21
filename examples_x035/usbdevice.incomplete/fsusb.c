#include "fsusb.h"
#include "ch32v003fun.h"
#include <string.h>

uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;



struct _USBState FSUSBCTX;

// based on https://github.com/openwch/ch32x035/tree/main/EVT/EXAM/USB/USBFS/DEVICE/CompositeKM/User

// Mask for the combined USBFSD->INT_FG + USBFSD->INT_ST
#define CRB_U_IS_NAK     (1<<7)
#define CTOG_MATCH_SYNC  (1<<6)
#define CRB_U_SIE_FREE   (1<<5)
#define CRB_UIF_FIFO_OV  (1<<4)
#define CRB_UIF_HST_SOF  (1<<3)
#define CRB_UIF_SUSPEND  (1<<2)
#define CRB_UIF_TRANSFER (1<<1)
#define CRB_UIF_BUS_RST  (1<<0)
#define CSETUP_ACT	     (1<<15)
#define CRB_UIS_TOG_OK   (1<<14)
#define CMASK_UIS_TOKEN  (3<<12)
#define CMASK_UIS_ENDP   (0xf<<8)

#define CUIS_TOKEN_OUT	 0x0
#define CUIS_TOKEN_SOF   0x1
#define CUIS_TOKEN_IN    0x2
#define CUIS_TOKEN_SETUP 0x3


void USBFS_IRQHandler() __attribute__((section(".text.vector_handler")))  __attribute__((interrupt));


void USBFS_IRQHandler()
{
	// Based on https://github.com/openwch/ch32x035/blob/main/EVT/EXAM/USB/USBFS/DEVICE/CompositeKM/User/ch32x035_usbfs_device.c

	// Combined FG + ST flag.
	uint16_t intfgst = *(uint16_t*)(&USBFS->INT_FG);
	int len = 0, errflag = 0;
	struct _USBState * ctx = &FSUSBCTX;
	GPIOA->BSHR = 1;

	// TODO: Check if needs to be do-while to re-check.
	if( intfgst & CRB_UIF_TRANSFER )
	{
		int token = ( intfgst & CMASK_UIS_TOKEN) >> 12;
		int ep = ( intfgst & CMASK_UIS_ENDP ) >> 8;

		switch ( token )
		{
		case CUIS_TOKEN_IN:
			switch( ep )
			{
			/* end-point 0 data in interrupt */
			case DEF_UEP0:
				if( ctx->USBFS_SetupReqLen == 0 )
				{
					USBFS->UEP0_CTRL_H = USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
				}

				if ( ( ctx->USBFS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
				{
					/* Non-standard request endpoint 0 Data upload, noted by official docs, but I don't think this would ever really be used. */
				}
				else
				{
					switch( FSUSBCTX.USBFS_SetupReqCode )
					{
						case USB_GET_DESCRIPTOR:
							len = ctx->USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : ctx->USBFS_SetupReqLen;
							memcpy( ctx->USBFS_EP0_Buf, ctx->pUSBFS_Descr, len );
							ctx->USBFS_SetupReqLen -= len;
							ctx->pUSBFS_Descr += len;
							USBFS->UEP0_TX_LEN = len;
							USBFS->UEP0_CTRL_H ^= USBFS_UEP_T_TOG;
							break;

						case USB_SET_ADDRESS:
							USBFS->DEV_ADDR = ( USBFS->DEV_ADDR & USBFS_UDA_GP_BIT ) | ctx->USBFS_DevAddr;
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
				ctx->USBFS_Endp_Busy[ DEF_UEP1 ] = 0;
				break;

			/* end-point 2 data in interrupt */
			case DEF_UEP2:
				USBFS->UEP2_CTRL_H = ( USBFS->UEP2_CTRL_H & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_NAK;
				USBFS->UEP2_CTRL_H ^= USBFS_UEP_T_TOG;
				ctx->USBFS_Endp_Busy[ DEF_UEP2 ] = 0;
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
						if( ( FSUSBCTX.USBFS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
						{
							if( ( FSUSBCTX.USBFS_SetupReqType & USB_REQ_TYP_MASK ) == USB_REQ_TYP_CLASS )
							{
								switch( FSUSBCTX.USBFS_SetupReqCode )
								{
									case HID_SET_REPORT:
										//KB_LED_Cur_Status = USBFS_EP0_Buf[ 0 ];
										// Do stuff...
										FSUSBCTX.USBFS_SetupReqLen = 0;
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
					if( FSUSBCTX.USBFS_SetupReqLen == 0 )
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
			int USBFS_SetupReqType = FSUSBCTX.USBFS_SetupReqType  = pUSBFS_SetupReqPak->bmRequestType;
			int USBFS_SetupReqCode = FSUSBCTX.USBFS_SetupReqCode  = pUSBFS_SetupReqPak->bRequest;
			int USBFS_SetupReqLen = FSUSBCTX.USBFS_SetupReqLen   = pUSBFS_SetupReqPak->wLength;
			int USBFS_SetupReqIndex = pUSBFS_SetupReqPak->wIndex;
			int USBFS_IndexValue = FSUSBCTX.USBFS_IndexValue = ( pUSBFS_SetupReqPak->wIndex << 16 ) | pUSBFS_SetupReqPak->wValue;
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
							if( USBFS_SetupReqIndex < 0x02 )
								FSUSBCTX.USBFS_HidIdle[ USBFS_SetupReqIndex ] = (uint8_t)( USBFS_IndexValue >> 8 );
							else
								errflag = 0xFF;
							break;
						case HID_SET_PROTOCOL:
							if ( USBFS_SetupReqIndex < 0x02 )
								FSUSBCTX.USBFS_HidProtocol[USBFS_SetupReqIndex] = (uint8_t)USBFS_IndexValue;
							else
								errflag = 0xFF;
							break;

						case HID_GET_IDLE:
							if( USBFS_SetupReqIndex < 0x02 )
							{
								FSUSBCTX.USBFS_EP0_Buf[ 0 ] = FSUSBCTX.USBFS_HidIdle[ USBFS_SetupReqIndex ];
								len = 1;
							}
							else
								errflag = 0xFF;
							break;

						case HID_GET_PROTOCOL:
							if( USBFS_SetupReqIndex < 0x02 )
							{
								FSUSBCTX.USBFS_EP0_Buf[ 0 ] = FSUSBCTX.USBFS_HidProtocol[ USBFS_SetupReqIndex ];
								len = 1;
							}
							else
								errflag = 0xFF;
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
					{
						int match = USBFS_IndexValue;
						struct descriptor_list_struct * e = descriptor_list;
						struct descriptor_list_struct * e_end = e + DESCRIPTOR_LIST_ENTRIES;
						for( ; e != e_end; e++ )
						{
							if( e->lIndexValue == USBFS_IndexValue )
							{
								ctx->pUSBFS_Descr = e->addr;
								len = e->length;
								break;
							}
						}
						if( e == e_end )
						{
							errflag = 0xFF;
						}

						/* Copy Descriptors to Endp0 DMA buffer */
						if( USBFS_SetupReqLen > len )
						{
							USBFS_SetupReqLen = len;
						}
						len = ( USBFS_SetupReqLen >= DEF_USBD_UEP0_SIZE ) ? DEF_USBD_UEP0_SIZE : USBFS_SetupReqLen;
						memcpy( ctx->USBFS_EP0_Buf, ctx->pUSBFS_Descr, len );
						ctx->pUSBFS_Descr += len;
						break;
					}

					/* Set usb address */
					case USB_SET_ADDRESS:
						ctx->USBFS_DevAddr = (uint8_t)( ctx->USBFS_IndexValue & 0xFF );
						break;

					/* Get usb configuration now set */
					case USB_GET_CONFIGURATION:
						ctx->USBFS_EP0_Buf[ 0 ] = ctx->USBFS_DevConfig;
						if( ctx->USBFS_SetupReqLen > 1 )
						{
							ctx->USBFS_SetupReqLen = 1;
						}
						break;

					/* Set usb configuration to use */
					case USB_SET_CONFIGURATION:
						ctx->USBFS_DevConfig = (uint8_t)( ctx->USBFS_IndexValue & 0xFF );
						ctx->USBFS_DevEnumStatus = 0x01;
						break;

					/* Clear or disable one usb feature */
					case USB_CLEAR_FEATURE:
						if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
						{
							/* clear one device feature */
							if( (uint8_t)( USBFS_IndexValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
							{
								/* clear usb sleep status, device not prepare to sleep */
								ctx->USBFS_DevSleepStatus &= ~0x01;
							}
							else
							{
								errflag = 0xFF;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							if( (uint8_t)( USBFS_IndexValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
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
#if FUSB_SUPPORTS_SLEEP
							/* Set Device Feature */
							if( (uint8_t)( USBFS_IndexValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
							{
								/* Set Wake-up flag, device prepare to sleep */
								USBFS_DevSleepStatus |= 0x01;
							}
							else
#endif
							{
								errflag = 0xFF;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							/* Set Endpoint Feature */
							if( (uint8_t)( USBFS_IndexValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
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
						ctx->USBFS_EP0_Buf[ 0 ] = 0x00;
						if( USBFS_SetupReqLen > 1 )
						{
							USBFS_SetupReqLen = 1;
						}
						break;

					case USB_SET_INTERFACE:
						break;

					/* host get status of specified device/interface/end-points */
					case USB_GET_STATUS:
						ctx->USBFS_EP0_Buf[ 0 ] = 0x00;
						ctx->USBFS_EP0_Buf[ 1 ] = 0x00;
						if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
						{
							if( ctx->USBFS_DevSleepStatus & 0x01 )
							{
								ctx->USBFS_EP0_Buf[ 0 ] = 0x02;
							}
							else
							{
								ctx->USBFS_EP0_Buf[ 0 ] = 0x00;
							}
						}
						else if( ( USBFS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
						{
							if( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) == ( DEF_UEP_IN | DEF_UEP1 ) )
							{
								if( ( USBFS->UEP1_CTRL_H & USBFS_UEP_T_RES_MASK ) == USBFS_UEP_T_RES_STALL )
								{
									ctx->USBFS_EP0_Buf[ 0 ] = 0x01;
								}
							}
							else if( (uint8_t)( USBFS_SetupReqIndex & 0xFF ) == ( DEF_UEP_IN | DEF_UEP2 ) )
							{
								if( ( USBFS->UEP2_CTRL_H & USBFS_UEP_T_RES_MASK ) == USBFS_UEP_T_RES_STALL )
								{
									ctx->USBFS_EP0_Buf[ 0 ] = 0x01;
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
		ctx->USBFS_DevConfig = 0;
		ctx->USBFS_DevAddr = 0;
		ctx->USBFS_DevSleepStatus = 0;
		ctx->USBFS_DevEnumStatus = 0;

		USBFS->DEV_ADDR = 0;
		USBFS_Device_Endp_Init( );
	}
	else if( intfgst & CRB_UIF_SUSPEND )
	{
		/* usb suspend interrupt processing */
		if( USBFS->MIS_ST & USBFS_UMS_SUSPEND )
		{
			ctx->USBFS_DevSleepStatus |= 0x02;
			if( ctx->USBFS_DevSleepStatus == 0x03 )
			{
				/* Handling usb sleep here */
				//TODO: MCU_Sleep_Wakeup_Operate( );
			}
		}
		else
		{
			ctx->USBFS_DevSleepStatus &= ~0x02;
		}
	}

	// Handle any other interrupts and just clear them out.
	*(uint16_t*)(&USBFS->INT_FG) = intfgst;

	//intfgst = *(uint16_t*)(&USBFS->INT_FG);
	GPIOA->BSHR = 1<<16;
}

void USBFS_Device_Endp_Init()
{
    USBFS->UEP4_1_MOD = RB_UEP1_TX_EN;
    USBFS->UEP2_3_MOD = RB_UEP2_TX_EN;
	USBFS->UEP567_MOD = 0;

	USBFS->UEP0_DMA = (intptr_t)FSUSBCTX.USBFS_EP0_Buf;
	USBFS->UEP1_DMA = (intptr_t)FSUSBCTX.USBFS_EP1_Buf;
	USBFS->UEP2_DMA = (intptr_t)FSUSBCTX.USBFS_EP2_Buf;

	USBFS->UEP0_CTRL_H = USBFS_UEP_R_RES_ACK | USBFS_UEP_T_RES_NAK;
	USBFS->UEP1_CTRL_H = USBFS_UEP_T_RES_NAK;
	USBFS->UEP2_CTRL_H = USBFS_UEP_T_RES_NAK;

    /* Clear End-points Busy Status */
    for(uint8_t i=0; i< sizeof(FSUSBCTX.USBFS_Endp_Busy)/sizeof(FSUSBCTX.USBFS_Endp_Busy[0]); i++ )
    {
        FSUSBCTX.USBFS_Endp_Busy[ i ] = 0;
    }
}


void USBFS_Poll()
{
	//USBDEBUG2 = USBFS->INT_ST;//EP0_DATA[1];
	//USBDEBUG1 = USBFS->MIS_ST;
}

int FSUSBSetup()
{
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
	RCC->AHBPCENR |= RCC_USBFS;

	NVIC_EnableIRQ( USBFS_IRQn );

	AFIO->CTLR |= USB_PHY_V33;

	USBFS->BASE_CTRL = RB_UC_RESET_SIE | RB_UC_CLR_ALL;
	USBFS->BASE_CTRL = 0x00;

	USBFS_Device_Endp_Init();

	// Enter device mode.
	USBFS->INT_EN = RB_UIE_SUSPEND | RB_UIE_TRANSFER | RB_UIE_BUS_RST;
	USBFS->DEV_ADDR = 0x00;
	USBFS->BASE_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN;
	USBFS->INT_FG = 0xff;
	USBFS->UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;

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

#if FUSB_5V_OPERATION
	// XXX This is dubious, copied from x035 - checkme.
	AFIO->CTLR = (AFIO->CTLR & ~(UDP_PUE_MASK | UDM_PUE_MASK | USB_PHY_V33)) | UDP_PUE_10K | USB_IOEN;
#else
	AFIO->CTLR = (AFIO->CTLR & ~(UDP_PUE_MASK | UDM_PUE_MASK )) | USB_PHY_V33 | UDP_PUE_1K5 | USB_IOEN;
#endif

	AFIO->CTLR = (AFIO->CTLR & ~(UDP_PUE_11 | UDM_PUE_11 )) | USB_PHY_V33 | USB_IOEN | UDP_PUE_11; //1.5k pullup

	// Enable PC16/17 Alternate Function (USB)
	// According to EVT, GPIO16 = GPIO_Mode_IN_FLOATING, GPIO17 = GPIO_Mode_IPU
	GPIOC->CFGXR = 	( GPIOC->CFGXR & ~( (0xf<<(4*0)) | (0xf<<(4*1)) ) )  |
					(((GPIO_CFGLR_IN_FLOAT)<<(4*0)) | (((GPIO_CFGLR_IN_PUPD)<<(4*1)))); // MSBs are CNF, LSBs are MODE
	GPIOC->BSXR = 1<<1; // PC17 on.

	// Go on-bus.
	return 0;
}





uint8_t USBFS_Endp_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len, uint8_t mod)
{
    uint8_t endp_mode;
    uint8_t buf_load_offset;

    /* DMA config, endp_ctrl config, endp_len config */
    if( ( endp >= DEF_UEP1 ) && ( endp <= DEF_UEP7 ) )
    {
        if( FSUSBCTX.USBFS_Endp_Busy[ endp ] == 0 )
        {
            if( (endp == DEF_UEP1) || (endp == DEF_UEP4) )
            {
                /* endp1/endp4 */
                endp_mode = USBFSD_UEP_MOD( 0 );
                if( endp == DEF_UEP1 )
                {
                    endp_mode = (uint8_t)( endp_mode >> 4 );
                }
            }
            else if( ( endp == DEF_UEP2 ) || ( endp == DEF_UEP3 ) )
            {
                /* endp2/endp3 */
                endp_mode = USBFSD_UEP_MOD( 1 );
                if( endp == DEF_UEP3 )
                {
                    endp_mode = (uint8_t)( endp_mode >> 4 );
                }
            }
            else if( ( endp == DEF_UEP5 ) || ( endp == DEF_UEP6 ) )
            {
                /* endp5/endp6 */
                endp_mode = USBFSD_UEP_MOD( 2 );
                if( endp == DEF_UEP6 )
                {
                    endp_mode = (uint8_t)( endp_mode >> 4 );
                }
            }
            else
            {
                /* endp7 */
                endp_mode = USBFSD_UEP_MOD( 3 );
            }

            if( endp_mode & USBFSD_UEP_TX_EN )
            {
                if( endp_mode & USBFSD_UEP_RX_EN )
                {
                    if( endp_mode & USBFSD_UEP_BUF_MOD )
                    {
                        if( USBFSD_UEP_TX_CTRL(endp) & USBFS_UEP_T_TOG )
                        {
                            buf_load_offset = 192;
                        }
                        else
                        {
                            buf_load_offset = 128;
                        }
                    }
                    else
                    {
                        buf_load_offset = 64;
                    }
                }
                else
                {
                    if( endp_mode & USBFSD_UEP_BUF_MOD )
                    {
                        /* double tx buffer */
                        if( USBFSD_UEP_TX_CTRL( endp ) & USBFS_UEP_T_TOG )
                        {
                            buf_load_offset = 64;
                        }
                        else
                        {
                            buf_load_offset = 0;
                        }
                    }
                    else
                    {
                        buf_load_offset = 0;
                    }
                }
                if( buf_load_offset == 0 )
                {
                    if( mod == DEF_UEP_DMA_LOAD )
                    {
                        /* DMA mode */
                        USBFSD_UEP_DMA( endp ) = (uint16_t)(uint32_t)pbuf;
                    }
                    else
                    {
                        /* copy mode */
                        memcpy( USBFSD_UEP_BUF( endp ), pbuf, len );
                    }
                }
                else
                {
                    memcpy( USBFSD_UEP_BUF( endp ) + buf_load_offset, pbuf, len );
                }

                /* tx length */
                USBFSD_UEP_TLEN( endp ) = len;
                /* response ack */
                USBFSD_UEP_TX_CTRL( endp ) = ( USBFSD_UEP_TX_CTRL( endp ) & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_ACK;
                /* Set end-point busy */
                FSUSBCTX.USBFS_Endp_Busy[ endp ] = 0x01;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
    return 0;
}


