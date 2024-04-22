#ifndef _FSUSB_H
#define _FSUSB_H

/*	Full speed USB infrastructure for CH32X035.
	Based off of the official USB stack, but, with significant perf and space
	reductions.

	Limitations:
	 * Limited to EPs 0..4.
	 * By default, functions are using dedicated buffers, not DMA (but there is no reason DMA could not be used instead)
	 * 


	General Notes:
	 * They seem to be using DMA->RX but TX via non-DMA (the opposite of what they should have done)

	*/



#include <stdint.h>
#include "ch32v003fun.h"
#include "usb_defines.h"
#include "usb_config.h"

extern uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;

struct _USBState;

int FSUSBSetup();
uint8_t USBFS_Endp_DataUp(uint8_t endp, const uint8_t *pbuf, uint16_t len, uint8_t mod);

static inline uint8_t * USBFS_GetEPBufferIfAvailable( int endp );
static inline void USBFS_SendEndpoint( int endp, int len );


// Implement the following:
#if FUSB_HID_NONSTANDARD
int HandleHidUserReportSetup( struct _USBState * ctx, tusb_control_request_t * req );
int HandleHidUserReportOut( struct _USBState * ctx, uint8_t * data, int len );
void HandleHidUserReportIn( struct _USBState * ctx, uint8_t * data, int len );
#endif




struct _USBState
{
	// Setup Request
	volatile uint8_t  USBFS_SetupReqCode;
	volatile uint8_t  USBFS_SetupReqType;
	volatile uint16_t USBFS_SetupReqLen;
	volatile uint32_t USBFS_IndexValue;

	// USB Device Status
	volatile uint8_t  USBFS_DevConfig;
	volatile uint8_t  USBFS_DevAddr;
	volatile uint8_t  USBFS_DevSleepStatus;
	volatile uint8_t  USBFS_DevEnumStatus;

	#define pUSBFS_SetupReqPak			((tusb_control_request_t*)USBFSD_UEP_BUF(0))
	#define CTRL0BUFF					USBFSD_UEP_BUF(0)

#if FUSB_HID_INTERFACES > 0
	uint8_t USBFS_HidIdle[FUSB_HID_INTERFACES];
	uint8_t USBFS_HidProtocol[FUSB_HID_INTERFACES];
#endif

	volatile uint8_t  USBFS_Endp_Busy[FUSB_CONFIG_EPS];
	const uint8_t  *pUSBFS_Descr;
};

extern struct _USBState FSUSBCTX;

// To TX, you can use USBFS_GetEPBufferIfAvailable or USBFSD_UEP_DMA( endp )

static inline uint8_t * USBFS_GetEPBufferIfAvailable( int endp )
{
	if( FSUSBCTX.USBFS_Endp_Busy[ endp ] ) return 0;
	return USBFSD_UEP_BUF( endp );
}

static inline void USBFS_SendEndpoint( int endp, int len )
{
	USBFSD_UEP_TLEN( endp ) = len;
	USBFSD_UEP_TX_CTRL( endp ) = ( USBFSD_UEP_TX_CTRL( endp ) & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_ACK;
	FSUSBCTX.USBFS_Endp_Busy[ endp ] = 0x01;
}

#endif

