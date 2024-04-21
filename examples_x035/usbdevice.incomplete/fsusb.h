#ifndef _FSUSB_H
#define _FSUSB_H

#include <stdint.h>
#include "ch32v003fun.h"
#include "usb_defines.h"
#include "usb_config.h"

extern uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;

int FSUSBSetup();
void USBFS_Poll();
uint8_t USBFS_Endp_DataUp(uint8_t endp, uint8_t *pbuf, uint16_t len, uint8_t mod);


struct _USBState
{
	/* Setup Request */
	volatile uint8_t  USBFS_SetupReqCode;
	volatile uint8_t  USBFS_SetupReqType;
	volatile uint16_t USBFS_SetupReqLen;
	volatile uint32_t USBFS_IndexValue;

	/* USB Device Status */
	volatile uint8_t  USBFS_DevConfig;
	volatile uint8_t  USBFS_DevAddr;
	volatile uint8_t  USBFS_DevSleepStatus;
	volatile uint8_t  USBFS_DevEnumStatus;

	/* Endpoint Buffer */
	__attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[64];
	__attribute__ ((aligned(4))) uint8_t USBFS_EP1_Buf[64];
	__attribute__ ((aligned(4))) uint8_t USBFS_EP2_Buf[64];
	#define pUSBFS_SetupReqPak                 ((tusb_control_request_t*)ctx->USBFS_EP0_Buf)


	int USBFS_HidIdle[2];
	int USBFS_HidProtocol[2];

	const uint8_t  *pUSBFS_Descr;

	/* USB IN Endpoint Busy Flag */
	volatile uint8_t  USBFS_Endp_Busy[UNUM_EP];
};

extern struct _USBState FSUSBCTX;


#if 0
extern const    uint8_t  *pUSBFS_Descr;

/* USB Device Status */
extern volatile uint8_t  USBFS_DevConfig;
extern volatile uint8_t  USBFS_DevAddr;
extern volatile uint8_t  USBFS_DevSleepStatus;
extern volatile uint8_t  USBFS_DevEnumStatus;

/* Endpoint Buffer */
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[];
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP2_Buf[];


/* USB IN Endpoint Busy Flag */
extern volatile uint8_t  USBFS_Endp_Busy[ ];


// ABOVE: TOOD: REWRITE!
#endif


#endif

