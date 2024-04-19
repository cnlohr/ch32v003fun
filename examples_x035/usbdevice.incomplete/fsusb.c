#include "fsusb.h"
#include "ch32v003fun.h"

uint32_t USBDEBUG0, USBDEBUG1, USBDEBUG2;
uint8_t  EP0_DATA[64] __attribute__((aligned(32)));

void USBFS_IRQHandler() __attribute__((section(".text.vector_handler")))  __attribute__((interrupt));

#define CRB_U_IS_NAK     (1<<7)
#define CTOG_MATCH_SYNC  (1<<6)
#define CRB_U_SIE_FREE   (1<<5)
#define CRB_UIF_FIFO_OV  (1<<4)
#define CRB_UIF_HST_SOF  (1<<3)
#define CRB_UIF_SUSPEND  (1<<2)
#define CRB_UIF_TRANSFER (1<<1)
#define CRB_UIF_BUS_RST  (1<<0)
#define CSETUP_ACT       (1<<15)
#define CRB_UIS_TOG_OK   (1<<14)
#define CMASK_UIS_TOKEN  (3<<12)
#define CMASK_UIS_ENDP   (0xf<<8)

#define CUIS_TOKEN_OUT        0x0
#define CUIS_TOKEN_SOF        0x1
#define CUIS_TOKEN_IN         0x2
#define CUIS_TOKEN_SETUP      0x3

void USBFS_IRQHandler()
{
	// Combined FG + ST flag.
	uint16_t intfgst = *(uint16_t*)(&USBFSD->INT_FG);

	GPIOA->BSHR = 1;

	// TODO: Check if needs to be do-while to re-check.


    if( intfgst & CRB_UIF_TRANSFER )
    {
        switch ( ( intfgst & CMASK_UIS_TOKEN) >> 12)
        {
            case CUIS_TOKEN_IN:
                switch ( intst & ( USBFS_UIS_TOKEN_M

/*	while( ( flags = USBFS->INT_FG & 0x9f ) )
	{
		if( flags & RB_UIF_TRANSFER )
		{
			// Handle transfer.
			USBDEBUG0++;
		}
		if( flags & RB_UIF_BUS_RST )
		{
			// Handle transfer.
			USBDEBUG0+=1000;
		}
		//USBDEBUG0 = USBFS->INT_FG;
		USBFS->INT_FG = flags;
	}
*/

	*(uint16_t*)(&USBFSD->INT_FG) = intfgst;
	//intfgst = *(uint16_t*)(&USBFSD->INT_FG);
	GPIOA->BSHR = 1<<16;
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
	USBFS->UEP4_1_MOD = 0;
	USBFS->UEP2_3_MOD = 0;
	USBFS->UEP567_MOD = 0;
	USBFS->UEP0_DMA = ((intptr_t)EP0_DATA) & 0x7fff;
    USBFS->DEV_ADDR = 0x00;
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

