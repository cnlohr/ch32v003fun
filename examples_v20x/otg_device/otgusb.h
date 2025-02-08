#ifndef _OTGUSB_H
#define _OTGUSB_H

/*	Full speed USB infrastructure for CH32V203
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
#include "ch32fun.h"
#include "usb_defines.h"
#include "usb_config.h"

extern uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;

struct _USBState;

// Provided functions:
int USBOTGSetup();
uint8_t USBOTG_Endp_DataUp(uint8_t endp, const uint8_t *pbuf, uint16_t len, uint8_t mod);
static inline uint8_t * USBOTG_GetEPBufferIfAvailable( int endp );
static inline int USBOTG_SendEndpoint( int endp, int len );

static inline void DMA7FastCopy( uint8_t * dest, const uint8_t * src, int len );
static inline void DMA7FastCopyComplete();




// Implement the following:
#if FUSB_HID_USER_REPORTS
int HandleHidUserGetReportSetup( struct _USBState * ctx, tusb_control_request_t * req );
int HandleHidUserSetReportSetup( struct _USBState * ctx, tusb_control_request_t * req );
void HandleHidUserReportDataOut( struct _USBState * ctx, uint8_t * data, int len );
int HandleHidUserReportDataIn( struct _USBState * ctx, uint8_t * data, int len );
void HandleHidUserReportOutComplete( struct _USBState * ctx );
#endif




struct _USBState
{
	// Setup Request
	uint8_t  USBOTG_SetupReqCode;
	uint8_t  USBOTG_SetupReqType;
	uint16_t USBOTG_SetupReqLen;   // Used for tracking place along send.
	uint32_t USBOTG_IndexValue;

	// USB Device Status
	uint8_t  USBOTG_DevConfig;
	uint8_t  USBOTG_DevAddr;
	uint8_t  USBOTG_DevSleepStatus;
	uint8_t  USBOTG_DevEnumStatus;

	uint8_t  *  pCtrlPayloadPtr;

	uint8_t ENDPOINTS[FUSB_CONFIG_EPS][64];

	#define CTRL0BUFF					(USBOTGCTX.ENDPOINTS[0])
	#define pUSBOTG_SetupReqPak			((tusb_control_request_t*)CTRL0BUFF)

#if FUSB_HID_INTERFACES > 0
	uint8_t USBOTG_HidIdle[FUSB_HID_INTERFACES];
	uint8_t USBOTG_HidProtocol[FUSB_HID_INTERFACES];
#endif
	volatile uint8_t USBOTG_Endp_Busy[FUSB_CONFIG_EPS];
	volatile uint8_t USBOTG_errata_dont_send_endpoint_in_window;
};

extern struct _USBState USBOTGCTX;

#include "otgusb.c"

#endif

