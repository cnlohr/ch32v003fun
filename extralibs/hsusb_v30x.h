#ifndef _HSUSB_H
#define _HSUSB_H

/* High speed USB infrastructure for CH32V30x.
   Based off of the official USB stack and the current CH32X035 FS implementation.

   This is referenced in Chapter 22 USB Host/Device Controller (USBHD) of CH32FV2x_V3xRM.pdf
*/

#include <stdint.h>
#include "ch32fun.h"
#include "usb_defines.h"
#include "usb_config.h"

struct _USBState
{
	// Setup Request
	uint8_t  USBHS_SetupReqCode;
	uint8_t  USBHS_SetupReqType;
	uint16_t USBHS_SetupReqLen;   // Used for tracking place along send.
	uint32_t USBHS_IndexValue;

	// USB Device Status
	uint16_t  USBHS_DevConfig;
	uint16_t  USBHS_DevAddr;
	uint8_t  USBHS_DevSleepStatus;
	uint8_t  USBHS_DevEnumStatus;

	uint8_t  *  pCtrlPayloadPtr;

	uint8_t ENDPOINTS[HUSB_CONFIG_EPS][64];

	#define CTRL0BUFF					(HSUSBCTX.ENDPOINTS[0])
	#define pUSBHS_SetupReqPak			((tusb_control_request_t*)CTRL0BUFF)

#if HUSB_HID_INTERFACES > 0
	uint8_t USBHS_HidIdle[HUSB_HID_INTERFACES];
	uint8_t USBHS_HidProtocol[HUSB_HID_INTERFACES];
#endif
	volatile uint8_t  USBHS_Endp_Busy[HUSB_CONFIG_EPS];
};

// Provided functions:
int HSUSBSetup();
uint8_t USBHS_Endp_DataUp(uint8_t endp, const uint8_t *pbuf, uint16_t len, uint8_t mod);

// Implement the following:
#if HUSB_HID_USER_REPORTS
int HandleHidUserGetReportSetup( struct _USBState * ctx, tusb_control_request_t * req );
int HandleHidUserSetReportSetup( struct _USBState * ctx, tusb_control_request_t * req );
void HandleHidUserReportDataOut( struct _USBState * ctx, uint8_t * data, int len );
int HandleHidUserReportDataIn( struct _USBState * ctx, uint8_t * data, int len );
void HandleHidUserReportOutComplete( struct _USBState * ctx );
#endif

#if HUSB_BULK_USER_REPORTS
void HandleGotEPComplete( struct _USBState * ctx, int ep );
#endif

extern struct _USBState HSUSBCTX;


// To TX, you can use USBFS_GetEPBufferIfAvailable or USBHSD_UEP_TXBUF( endp )

static inline uint8_t * USBHS_GetEPBufferIfAvailable( int endp )
{
	if( HSUSBCTX.USBHS_Endp_Busy[ endp ] ) return 0;
	return USBHSD_UEP_TXBUF( endp );
}

static inline void USBHS_SendEndpoint( int endp, int len, const uint8_t * data )
{
	if( endp )
	{
		(((uint32_t*)(&USBHSD->UEP1_TX_DMA))[2-1]) = (uintptr_t)data;
	}
	USBHSD_UEP_TLEN( endp ) = len;
	USBHSD_UEP_TXCTRL( endp ) = ( USBHSD_UEP_TXCTRL( endp ) & ~USBHS_UEP_T_RES_MASK ) | USBHS_UEP_T_RES_ACK;
	HSUSBCTX.USBHS_Endp_Busy[ endp ] = 0x01;
}
#endif
