#ifndef _FSUSB_H
#define _FSUSB_H

#include <stdint.h>

extern uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;

int FSUSBSetup();
void USBFS_Poll();

#endif

