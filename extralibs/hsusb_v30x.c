#include "hsusb_v30x.h"
#include "ch32fun.h"
#include <string.h>

#define UEP_CTRL_H(n) (((uint16_t*)&USBHSD->UEP0_TX_CTRL)[n*2])

struct _USBState HSUSBCTX;

// based on https://github.com/openwch/ch32v307/blob/main/EVT/EXAM/USB/USBHS/DEVICE/CompositeKM

// Mask for the combined USBHSD->INT_FG + USBHSD->INT_ST
#define CRB_U_IS_NAK     (1<<7)
#define CTOG_MATCH_SYNC  (1<<6)
#define CRB_UIF_SETUP_ACT  (1<<5) // CRB_U_SIE_FREE on USBFS
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

#if 0
static inline void DMA7FastCopy( uint8_t * dest, const uint8_t * src, int len )
{
	while( DMA1_Channel7->CNTR );
	DMA1_Channel7->CFGR = 0;
	DMA1_Channel7->MADDR = (uintptr_t)src;
	DMA1_Channel7->PADDR = (uintptr_t)dest;
	DMA1_Channel7->CNTR  = (len+3)/4;
	DMA1_Channel7->CFGR  =
		DMA_M2M_Enable | 
		DMA_DIR_PeripheralDST |
		DMA_Priority_Low |
		DMA_MemoryDataSize_Word |
		DMA_PeripheralDataSize_Word |
		DMA_MemoryInc_Enable |
		DMA_PeripheralInc_Enable |
		DMA_Mode_Normal | DMA_CFGR1_EN;
#if !( FUSB_CURSED_TURBO_DMA == 1 )
	// Somehow, it seems to work (unsafely) without this.
	// Really, though, it's probably fine.
	while( DMA1_Channel7->CNTR );
#endif
}

static inline void DMA7FastCopyComplete() { while( DMA1_Channel7->CNTR ); }

#endif

void USBHS_InternalFinishSetup();

//void USBHSWakeUp_IRQHandler(void) __attribute((interrupt));
//void USBHSWakeUp_IRQHandler(void)
//{
//	printf( "USBHSWakeUp MSTATUS:%08x MTVAL:%08x MCAUSE:%08x MEPC:%08x\n", (int)__get_MSTATUS(), (int)__get_MTVAL(), (int)__get_MCAUSE(), (int)__get_MEPC() );
//}
extern uint8_t scratchpad[];

void USBHS_IRQHandler(void) __attribute((interrupt));
void USBHS_IRQHandler(void)
{
	// Based on https://github.com/openwch/ch32v307/blob/main/EVT/EXAM/USB/USBHS/DEVICE/CompositeKM/User/ch32v30x_usbhs_device.c
	// Combined FG + ST flag
	uint16_t intfgst = *(uint16_t*)(&USBHSD->INT_FG);
	int len = 0;
	struct _USBState * ctx = &HSUSBCTX;
	uint8_t * ctrl0buff = CTRL0BUFF;

	if( intfgst & ( CRB_UIF_SETUP_ACT ) )
	{
		// On the Chapter 22 USB, SETUP Requests are handled here instead of in UIF_TRANSFER, with TOKEN_SETUP.
		USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
		USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;

		/* Store All Setup Values */
		int USBHS_SetupReqType = HSUSBCTX.USBHS_SetupReqType  = pUSBHS_SetupReqPak->bmRequestType;
		int USBHS_SetupReqCode = HSUSBCTX.USBHS_SetupReqCode  = pUSBHS_SetupReqPak->bRequest;
		int USBHS_SetupReqLen = HSUSBCTX.USBHS_SetupReqLen    = pUSBHS_SetupReqPak->wLength;
		int USBHS_SetupReqIndex = pUSBHS_SetupReqPak->wIndex;
		int USBHS_IndexValue = HSUSBCTX.USBHS_IndexValue = ( pUSBHS_SetupReqPak->wIndex << 16 ) | pUSBHS_SetupReqPak->wValue;
		len = 0;

		//printf( "Setup: %d %d %d %d %d\n", USBHS_SetupReqType, USBHS_SetupReqCode, USBHS_SetupReqLen,
		//	USBHS_SetupReqIndex, USBHS_IndexValue );

		if( ( USBHS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
		{
#if HUSB_HID_INTERFACES > 0 
			if( ( USBHS_SetupReqType & USB_REQ_TYP_MASK ) == USB_REQ_TYP_CLASS )
			{
				/* Class Request */
				//printf( "REQ: %d [%02x %02x %04x %04x]\n", USBHS_SetupReqCode, pUSBHS_SetupReqPak->bmRequestType, pUSBHS_SetupReqPak->bRequest,	pUSBHS_SetupReqPak->wValue, pUSBHS_SetupReqPak->wLength );
				switch( USBHS_SetupReqCode )
				{
					case HID_SET_REPORT:
#if HUSB_HID_USER_REPORTS
						len = HandleHidUserSetReportSetup( ctx, pUSBHS_SetupReqPak );
						if( len < 0 ) goto sendstall;
						ctx->USBHS_SetupReqLen = len;
						USBHSD->UEP0_TX_LEN = 0;
						USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
						USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1;
						goto replycomplete;
					case HID_GET_REPORT:
						len = HandleHidUserGetReportSetup( ctx, pUSBHS_SetupReqPak );
						if( len < 0 ) goto sendstall;
						ctx->USBHS_SetupReqLen = len;
						len = len >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : len;
						if( !ctx->pCtrlPayloadPtr )
						{
							len = HandleHidUserReportDataIn( ctx, ctrl0buff, len );
						}
						else
						{
							//DMA7FastCopy( ctrl0buff, ctx->pCtrlPayloadPtr, len );
							memcpy( ctrl0buff, ctx->pCtrlPayloadPtr, len );
							ctx->pCtrlPayloadPtr += len;
						}
						USBHSD->UEP0_TX_LEN = len;
						USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
						ctx->USBHS_SetupReqLen -= len;
						goto replycomplete;
#endif
						break;


					case HID_SET_IDLE:
						if( USBHS_SetupReqIndex < HUSB_HID_INTERFACES )
							HSUSBCTX.USBHS_HidIdle[ USBHS_SetupReqIndex ] = (uint8_t)( USBHS_IndexValue >> 8 );
						break;
					case HID_SET_PROTOCOL:
						if ( USBHS_SetupReqIndex < HUSB_HID_INTERFACES )
							HSUSBCTX.USBHS_HidProtocol[USBHS_SetupReqIndex] = (uint8_t)USBHS_IndexValue;
						break;

					case HID_GET_IDLE:
						if( USBHS_SetupReqIndex < HUSB_HID_INTERFACES )
						{
							ctrl0buff[0] = HSUSBCTX.USBHS_HidIdle[ USBHS_SetupReqIndex ];
							len = 1;
						}
						break;

					case HID_GET_PROTOCOL:
						if( USBHS_SetupReqIndex < HUSB_HID_INTERFACES )
						{
							ctrl0buff[0] = HSUSBCTX.USBHS_HidProtocol[ USBHS_SetupReqIndex ];
							len = 1;
						}
						break;

					default:
						goto sendstall;
						break;
				}
			}
#else
			;
#endif
		}
		else
		{
			/* usb standard request processing */
			switch( USBHS_SetupReqCode )
			{
				/* get device/configuration/string/report/... descriptors */
				case USB_GET_DESCRIPTOR:
				{
					const struct descriptor_list_struct * e = descriptor_list;
					const struct descriptor_list_struct * e_end = e + DESCRIPTOR_LIST_ENTRIES;
					for( ; e != e_end; e++ )
					{
						if( e->lIndexValue == USBHS_IndexValue )
						{
							ctx->pCtrlPayloadPtr = (uint8_t*)e->addr;
							len = e->length;
							break;
						}
					}
					if( e == e_end )
					{
						goto sendstall;
					}


					/* Copy Descriptors to Endp0 DMA buffer */
					int totalLen = USBHS_SetupReqLen;
					if( totalLen > len )
					{
						totalLen = len;
					}
					len = ( totalLen >= DEF_USBD_UEP0_SIZE ) ? DEF_USBD_UEP0_SIZE : totalLen;
					//DMA7FastCopy( ctrl0buff, ctx->pCtrlPayloadPtr, len ); //memcpy( CTRL0BUFF, ctx->pCtrlPayloadPtr, len );
					memcpy( ctrl0buff, ctx->pCtrlPayloadPtr, len );
					ctx->USBHS_SetupReqLen = totalLen - len;
					ctx->pCtrlPayloadPtr += len;
					USBHSD->UEP0_TX_LEN = len;
					USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
					goto replycomplete;
				}

				/* Set usb address */
				case USB_SET_ADDRESS:
					ctx->USBHS_DevAddr = (uint16_t)( ctx->USBHS_IndexValue & 0xFF );
					break;

				/* Get usb configuration now set */
				case USB_GET_CONFIGURATION:
					ctrl0buff[0] = ctx->USBHS_DevConfig;
					if( ctx->USBHS_SetupReqLen > 1 )
						ctx->USBHS_SetupReqLen = 1;
					break;

				/* Set usb configuration to use */
				case USB_SET_CONFIGURATION:
					ctx->USBHS_DevConfig = (uint8_t)( ctx->USBHS_IndexValue & 0xFF );
					ctx->USBHS_DevEnumStatus = 0x01;
					break;

				/* Clear or disable one usb feature */
				case USB_CLEAR_FEATURE:
#if HUSB_SUPPORTS_SLEEP
					if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
					{
						/* clear one device feature */
						if( (uint8_t)( USBHS_IndexValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
						{
							/* clear usb sleep status, device not prepare to sleep */
							ctx->USBHS_DevSleepStatus &= ~0x01;
						}
						else
						{
							goto sendstall;
						}
					}
					else
#endif
					if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
					{
						if( (uint8_t)( USBHS_IndexValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
						{
							/* Clear End-point Feature */
							int ep = USBHS_SetupReqIndex & 0xf;
							if( ( USBHS_SetupReqIndex & DEF_UEP_IN ) && ep < HUSB_CONFIG_EPS ) 
							{
								UEP_CTRL_H(ep) = USBHS_UEP_T_TOG_DATA0 | USBHS_UEP_T_RES_NAK;
							}
							else
							{
								goto sendstall;
							}
						}
						else
						{
							goto sendstall;
						}
					}
					else
					{
						goto sendstall;
					}
					break;

				/* set or enable one usb feature */
				case USB_SET_FEATURE:
					if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
					{
#if HUSB_SUPPORTS_SLEEP
						/* Set Device Feature */
						if( (uint8_t)( USBHS_IndexValue & 0xFF ) == USB_REQ_FEAT_REMOTE_WAKEUP )
						{
							/* Set Wake-up flag, device prepare to sleep */
							USBHS_DevSleepStatus |= 0x01;
						}
						else
#endif
						{
							goto sendstall;
						}
					}
					else if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
					{
						/* Set Endpoint Feature */
						if( (uint8_t)( USBHS_IndexValue & 0xFF ) == USB_REQ_FEAT_ENDP_HALT )
						{
							int ep = USBHS_SetupReqIndex & 0xf;
							if( ( USBHS_SetupReqIndex & DEF_UEP_IN ) && ep < HUSB_CONFIG_EPS )
								UEP_CTRL_H(ep) = ( UEP_CTRL_H(ep) & ~USBHS_UEP_T_RES_MASK ) | USBHS_UEP_T_RES_STALL;
						}
						else
							goto sendstall;
					}
					else
						goto sendstall;
					break;

				/* This request allows the host to select another setting for the specified interface  */
				case USB_GET_INTERFACE:
					ctrl0buff[0] = 0x00;
					if( USBHS_SetupReqLen > 1 ) USBHS_SetupReqLen = 1;
					break;

				case USB_SET_INTERFACE:
					break;

				/* host get status of specified device/interface/end-points */
				case USB_GET_STATUS:
					ctrl0buff[0] = 0x00;
					ctrl0buff[1] = 0x00;
					if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )
					{
#if FUSB_SUPPORTS_SLEEP
						ctrl0buff[0] = (ctx->USBHS_DevSleepStatus & 0x01)<<1;
#else
						ctrl0buff[0] = 0x00;
#endif
					}
					else if( ( USBHS_SetupReqType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
					{
						int ep = USBHS_SetupReqIndex & 0xf;
						if( ( USBHS_SetupReqIndex & DEF_UEP_IN ) && ep < HUSB_CONFIG_EPS )
							ctrl0buff[0] = ( UEP_CTRL_H(ep) & USBHS_UEP_T_RES_MASK ) == USBHS_UEP_T_RES_STALL;
						else
							goto sendstall;
					}
					else
						goto sendstall;
					if( USBHS_SetupReqLen > 2 )
						USBHS_SetupReqLen = 2;
					break;

				default:
					goto sendstall;
					break;
			}
		}


		{
			/* end-point 0 data Tx/Rx */
			if( USBHS_SetupReqType & DEF_UEP_IN )
			{
				len = ( USBHS_SetupReqLen > DEF_USBD_UEP0_SIZE )? DEF_USBD_UEP0_SIZE : USBHS_SetupReqLen;
				USBHS_SetupReqLen -= len;
				USBHSD->UEP0_TX_LEN = len;
				USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
			}
			else
			{
				if( USBHS_SetupReqLen == 0 )
				{
					USBHSD->UEP0_TX_LEN = 0;
					USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
				}
				else
				{
					USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
				}
			}
		}

		goto replycomplete;

		// This might look a little weird, for error handling but it saves a nontrivial amount of storage, and simplifies
		// control flow to hard-abort here.
	sendstall:
		// if one request not support, return stall.  Stall means permanent error.
		USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL;
		USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL;
	replycomplete:
		;
	}
	if( intfgst & ( CRB_UIF_TRANSFER ) )
	{
		int token = ( intfgst & CMASK_UIS_TOKEN) >> 12;
		int ep = ( intfgst & CMASK_UIS_ENDP ) >> 8;
		switch ( token )
		{
		case CUIS_TOKEN_IN:
			if( ep )
			{
				if( ep < HUSB_CONFIG_EPS )
				{
					UEP_CTRL_H(ep) = ( UEP_CTRL_H(ep) & ~USBHS_UEP_T_RES_MASK ) | USBHS_UEP_T_RES_NAK;
					UEP_CTRL_H(ep) ^= USBHS_UEP_T_TOG_DATA1;
					ctx->USBHS_Endp_Busy[ ep ] = 0;
					// Don't set EP in here.  Wait for out.
					// Optimization: Could we set EP here?
				}
			}
			else
			{
				/* end-point 0 data in interrupt */
				if( ctx->USBHS_SetupReqLen == 0 )
				{
					USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
				}

				if( ctx->pCtrlPayloadPtr )
				{
					// Shortcut mechanism, for descriptors or if the user wants it.
					len = ctx->USBHS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : ctx->USBHS_SetupReqLen;
					//DMA7FastCopy( ctrl0buff, ctx->pCtrlPayloadPtr, len ); // FYI -> Would need to do this if using DMA
					memcpy(  ctrl0buff, ctx->pCtrlPayloadPtr, len );
					ctx->USBHS_SetupReqLen -= len;
					if( ctx->USBHS_SetupReqLen > 0 )
						ctx->pCtrlPayloadPtr += len;
					else
						ctx->pCtrlPayloadPtr = 0;

					USBHSD->UEP0_TX_LEN = len;
					USBHSD->UEP0_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
				}
				else if ( ( ctx->USBHS_SetupReqType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )
				{

#if HUSB_HID_USER_REPORTS
					len = ctx->USBHS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : ctx->USBHS_SetupReqLen;
					if( len && HSUSBCTX.USBHS_SetupReqCode == HID_GET_REPORT )
					{
						len = HandleHidUserReportDataIn( ctx, ctrl0buff, len );
						USBHSD->UEP0_TX_LEN = len;
						USBHSD->UEP0_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
						ctx->USBHS_SetupReqLen -= len;
						ctx->pCtrlPayloadPtr += len;
					}	
#endif
				}
				else
				{
					switch( HSUSBCTX.USBHS_SetupReqCode )
					{
						case USB_GET_DESCRIPTOR:
							break;

						case USB_SET_ADDRESS:
							USBHSD->DEV_AD = ctx->USBHS_DevAddr;
							break;

						default:
							break;
					}
				}
			}

		/* data-out stage processing */
		case CUIS_TOKEN_OUT:
			switch( ep )
			{
				/* end-point 0 data out interrupt */
				case DEF_UEP0:
				{
					// XXX WARNINGS:
					// 1. intfgst & CRB_UIS_TOG_OK is not set for non-odd transactions, i.e. first, third, etc, are all fine.
					// 2. HandleHidUserReportOutComplete doesn't seem to work.
					//if( intfgst & CRB_UIS_TOG_OK )

#if HUSB_HID_USER_REPORTS
					int len = USBHSD->RX_LEN;
					uint8_t * cptr = ctx->pCtrlPayloadPtr;
					if( !cptr )
					{
						HandleHidUserReportDataOut( ctx, ctrl0buff, len );
					}
					else
					{
						int remain = ctx->USBHS_SetupReqLen - len;
						if( remain < 0 )
						{
							len += remain;
							remain = 0;
						}
						//DMA7FastCopy( cptr, ctrl0buff, len );
						memcpy( cptr, ctrl0buff, len  );
						ctx->USBHS_SetupReqLen = remain;
						if( remain > 0 )
							ctx->pCtrlPayloadPtr = cptr + len;
						else
							ctx->pCtrlPayloadPtr = 0;
					}
#endif

					if( ctx->USBHS_SetupReqLen == 0 )
					{
#if HUSB_HID_USER_REPORTS
						//DMA7FastCopyComplete();
						HandleHidUserReportOutComplete( ctx );
#endif
					}

					// See above comment
					//	//USBHSD->UEP0_RX_CTRL ^= USBFS_UEP_R_TOG;

					break;
				}
				default:
					// Any other out. (also happens with In)
					HSUSBCTX.USBHS_Endp_Busy[ ep ] = 0x02;
					USBHSD_UEP_RXCTRL( ep ) = ((USBHSD_UEP_RXCTRL( ep )) & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_NAK;
#if HUSB_BULK_USER_REPORTS
					HandleGotEPComplete( ctx, ep );
#endif
					break;
			}
			break;
		case CUIS_TOKEN_SETUP: // Not actually used on this chip (It's done as a separate flag)
		case CUIS_TOKEN_SOF: // Sof pack processing
			break;

		default :
			break;
		}
	}
	if(intfgst & USBHS_UIF_BUS_RST)
	{
		/* usb reset interrupt processing */
		ctx->USBHS_DevConfig = 0;
		ctx->USBHS_DevAddr = 0;
		ctx->USBHS_DevSleepStatus = 0;
		ctx->USBHS_DevEnumStatus = 0;

		USBHSD->DEV_AD = 0;
		USBHS_InternalFinishSetup( );
	}
	if(intfgst & USBHS_UIF_SUSPEND)
	{
		USBHSD->INT_FG = USBHS_UIF_SUSPEND;
		Delay_Us(10);

		// USB suspend interrupt processing
		if(USBHSD->MIS_ST & USBHS_UMS_SUSPEND)
		{
			HSUSBCTX.USBHS_DevSleepStatus |= 0x02;
			if(HSUSBCTX.USBHS_DevSleepStatus == 0x03)
			{
				// TODO: Handle usb sleep here
			}
		}
		else
		{
			HSUSBCTX.USBHS_DevSleepStatus &= ~0x02;
		}
	}

	USBHSD->INT_FG = intfgst;
}

void USBHS_InternalFinishSetup()
{

	// To reconfigure your endpoints for TX/RX do it here.

#if HUSB_CONFIG_EPS > 5
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN | USBHS_UEP1_T_EN
							| USBHS_UEP2_T_EN | USBHS_UEP3_T_EN | USBHS_UEP4_T_EN | USBHS_UEP5_R_EN;
#elif HUSB_CONFIG_EPS > 4
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN | USBHS_UEP1_T_EN
							| USBHS_UEP2_T_EN | USBHS_UEP3_T_EN | USBHS_UEP4_T_EN;
#elif HUSB_CONFIG_EPS > 3
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN | USBHS_UEP1_T_EN
							| USBHS_UEP2_T_EN | USBHS_UEP3_T_EN;
#elif HUSB_CONFIG_EPS > 2
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN | USBHS_UEP1_T_EN
							| USBHS_UEP2_T_EN;
#elif HUSB_CONFIG_EPS > 1
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN | USBHS_UEP1_T_EN;
#else
	USBHSD->ENDP_CONFIG = USBHS_UEP0_T_EN | USBHS_UEP0_R_EN;
#endif

	// This is really cursed.  Somehow it doesn't explode.
	// But, normally the USB wants a separate buffer here.

#if HUSB_CONFIG_EPS > 5
	// Feel free to override any of these.
	USBHSD->UEP5_MAX_LEN = 64;
	USBHSD->UEP5_RX_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[5];
	USBHSD->UEP5_RX_CTRL = USBHS_UEP_R_RES_ACK | USBHS_UEP_R_TOG_AUTO; 	// For bulk-out, I think you need to do this.
#endif
#if HUSB_CONFIG_EPS > 4
	USBHSD->UEP4_MAX_LEN = 64; // TODO: change to dynamic size, as USB HS supports more than 64?
	USBHSD->UEP4_TX_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[4];
#endif
#if HUSB_CONFIG_EPS > 3
	USBHSD->UEP3_MAX_LEN = 64; // TODO: change to dynamic size, as USB HS supports more than 64?
	USBHSD->UEP3_TX_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[3];
#endif
#if HUSB_CONFIG_EPS > 2
	USBHSD->UEP2_MAX_LEN = 64; // TODO: change to dynamic size, as USB HS supports more than 64?
	USBHSD->UEP2_TX_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[2];
#endif
#if HUSB_CONFIG_EPS > 1
	USBHSD->UEP1_MAX_LEN = 64; // TODO: change to dynamic size, as USB HS supports more than 64?
	USBHSD->UEP1_TX_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[1];
#endif
#if HUSB_CONFIG_EPS > 0
	USBHSD->UEP0_MAX_LEN = 64;
	USBHSD->UEP0_DMA = (uintptr_t)HSUSBCTX.ENDPOINTS[0];
#else
#error You must have at least EP0!
#endif

	UEP_CTRL_H(0) = USBHS_UEP_R_RES_ACK | USBHS_UEP_T_RES_NAK;
	int i;
	for( i = 1; i < HUSB_CONFIG_EPS; i++ )
		UEP_CTRL_H(i) = USBFS_UEP_T_RES_NAK;

	for(uint8_t i=0; i< sizeof(HSUSBCTX.USBHS_Endp_Busy)/sizeof(HSUSBCTX.USBHS_Endp_Busy[0]); i++ )
	{
		HSUSBCTX.USBHS_Endp_Busy[ i ] = 0;
	}
}

int HSUSBSetup()
{
	// Set USB clock source to USBPHY
	RCC->CFGR2 &= ~(1 << 31);
	RCC->CFGR2 |= RCC_USBCLK48MCLKSource_USBPHY << 31;

	// Set PLL clock source to HSE
	RCC->CFGR2 &= ~(1 << 27);
	RCC->CFGR2 |= RCC_HSBHSPLLCLKSource_HSE << 27;

	// Configure PLL for USB
	RCC->CFGR2 &= ~(7 << 24);
	RCC->CFGR2 |= RCC_USBPLL_Div2 << 24;

	// Configure reference clock
	RCC->CFGR2 &= ~(3 << 28);
	RCC->CFGR2 |= RCC_USBHSPLLCKREFCLK_4M << 28;

	// Enable USB high-speed peripheral
	RCC->CFGR2 |= (1 << 30);
	RCC->AHBPCENR |= RCC_AHBPeriph_USBHS | RCC_AHBPeriph_DMA1;

	// Initialize USB module
	USBHSD->CONTROL = USBHS_UC_CLR_ALL | USBHS_UC_RESET_SIE;
	Delay_Us(10);
	USBHSD->CONTROL = 0;

	// Initialize USB device config
	USBHSD->HOST_CTRL = USBHS_UH_PHY_SUSPENDM;
	USBHSD->CONTROL = USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY | USBHS_UC_SPEED_HIGH;
	USBHSD->INT_EN = USBHS_UIE_SETUP_ACT | USBHS_UIE_TRANSFER | USBHS_UIE_DETECT | USBHS_UIE_SUSPEND;
	
	USBHS_InternalFinishSetup();

	USBHSD->CONTROL |= USBHS_UC_DEV_PU_EN;
	NVIC_EnableIRQ(USBHS_IRQn);

	// Go on-bus.
	return 0;
}


