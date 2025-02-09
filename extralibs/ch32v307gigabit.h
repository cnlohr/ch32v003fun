#ifndef _CH32V307GIGABIT_H
#define _CH32V307GIGABIT_H

/* This file is written against the RTL8211E
*/

// #define CH32V307GIGABIT_MCO25 1
// #define CH32V307GIGABIT_PHYADDRESS 0

#define CH32V307GIGABIT_RXBUFNB 8
#define CH32V307GIGABIT_TXBUFNB 8
#define CH32V307GIGABIT_BUFFSIZE 1524 // 1518 + 4, Rounded up.

#define CH32V307GIGABIT_CFG_CLOCK_DELAY 4 // 0..7
#define CH32V307GIGABIT_CFG_CLOCK_PHASE 0

#ifndef CH32V307GIGABIT_MCO25
#define CH32V307GIGABIT_MCO25 1
#endif

#ifndef CH32V307GIGABIT_PHYADDRESS
#define CH32V307GIGABIT_PHYADDRESS 0
#endif

#ifndef CH32V307GIGABIT_PHY_TIMEOUT
#define CH32V307GIGABIT_PHY_TIMEOUT 0x10000
#endif

// Additional definitions, not part of ch32v003fun.h
#ifndef CH32V307GIGABIT_PHY_RSTB
#define CH32V307GIGABIT_PHY_RSTB PA10
#endif

// ETH DMA structure definition (From ch32v30x_eth.c
typedef struct
{
  uint32_t   volatile Status;       /* Status */
  uint32_t   ControlBufferSize;     /* Control and Buffer1, Buffer2 lengths */
  uint32_t   Buffer1Addr;           /* Buffer1 address pointer */
  uint32_t   Buffer2NextDescAddr;   /* Buffer2 or next descriptor address pointer */
} ETH_DMADESCTypeDef;

// You must provide:

void ch32v307ethHandleReconfig( int link, int speed, int duplex );

// Return non-zero to suppress OWN return (for if you are still holding onto the buffer)
int ch32v307ethInitHandlePacket( uint8_t * data, int frame_length, ETH_DMADESCTypeDef * dmadesc );

void ch32v307ethInitHandleTXC( void );

// This library provides:
static void ch32v307ethGetMacInUC( uint8_t * mac );
static int ch32v307ethInit( void );
static int ch32v307ethTransmitStatic(uint8_t * buffer, uint32_t length, int enable_txc);  // Does not copy.
static int ch32v307ethTickPhy( void );

// Data pursuent to ethernet.
uint8_t ch32v307eth_mac[6] = { 0 };
uint16_t ch32v307eth_phyid = 0; // 0xc916 = RTL8211FS / 0xc915 = RTL8211E-VB
ETH_DMADESCTypeDef ch32v307eth_DMARxDscrTab[CH32V307GIGABIT_RXBUFNB] __attribute__((aligned(4)));            // MAC receive descriptor, 4-byte aligned
ETH_DMADESCTypeDef ch32v307eth_DMATxDscrTab[CH32V307GIGABIT_TXBUFNB] __attribute__((aligned(4)));            // MAC send descriptor, 4-byte aligned
uint8_t  ch32v307eth_MACRxBuf[CH32V307GIGABIT_RXBUFNB*CH32V307GIGABIT_BUFFSIZE] __attribute__((aligned(4))); // MAC receive buffer, 4-byte aligned
ETH_DMADESCTypeDef * pDMARxGet;
ETH_DMADESCTypeDef * pDMATxSet;


// Internal functions
static int ch32v307ethPHYRegWrite( uint32_t reg, uint32_t val );
static int ch32v307ethPHYRegAsyncRead( int reg, int * value );
static int ch32v307ethPHYRegRead( uint32_t reg );

static int ch32v307ethPHYRegAsyncRead( int reg, int * value )
{
	static uint8_t reg_request_count = 0;

	uint32_t miiar = ETH->MACMIIAR;
	if( miiar & ETH_MACMIIAR_MB )
	{
		return -1;
	}
	if( ( ( miiar & ETH_MACMIIAR_MR ) >> 6 ) != reg || reg_request_count < 2 )
	{
		ETH->MACMIIAR = ETH_MACMIIAR_CR_Div42 /* = 0, per 27.1.8.1.4 */ |
			((uint32_t)CH32V307GIGABIT_PHYADDRESS << 11) | // ETH_MACMIIAR_PA
			(((uint32_t)reg << 6) & ETH_MACMIIAR_MR) |
			(0 /*!ETH_MACMIIAR_MW*/) | ETH_MACMIIAR_MB;
		reg_request_count++;
		return -1;
	}
	reg_request_count = 0;
	*value = ETH->MACMIIDR;
	ETH->MACMIIAR |= ETH_MACMIIAR_MR; // Poison register.
	return 0;
}

static int ch32v307ethTickPhy(void)
{
	int speed, linked, duplex;
	const int reg = (ch32v307eth_phyid == 0xc916) ? 0x1a : 0x11; // PHYSR (different on each part)
	int miidr;
	if( ch32v307ethPHYRegAsyncRead( reg, &miidr ) ) return -1;

	printf( "REG: %02x / %04x / %04x\n", reg, miidr, ch32v307eth_phyid );

	if( reg == 0x1a )
	{
		speed  = ((miidr>>4)&3);
		linked = ((miidr>>2)&1);
		duplex = ((miidr>>3)&1);		
	}
	else
	{
		speed  = ((miidr>>14)&3);
		linked = ((miidr>>10)&1);
		duplex = ((miidr>>13)&1);
	}

	printf( "LINK INFO: %d %d %d\n", speed, linked, duplex );
	if( linked )
	{
		uint32_t oldmaccr = ETH->MACCR;
		uint32_t newmaccr = (oldmaccr & ~( ( 1<<11 ) | (3<<14) ) ) | (speed<<14) | ( duplex<<11);
		if( newmaccr != oldmaccr )
		{
			ETH->MACCR = newmaccr;
			ch32v307ethHandleReconfig( linked, speed, duplex );
			return 1;
		}
	}
	return 0;
}


// Based on ETH_WritePHYRegister
static int ch32v307ethPHYRegWrite( uint32_t reg, uint32_t val )
{
	ETH->MACMIIDR = val;
	ETH->MACMIIAR = ETH_MACMIIAR_CR_Div42 /* = 0, per 27.1.8.1.4 */ |
		(((uint32_t)CH32V307GIGABIT_PHYADDRESS << 11)) | // ETH_MACMIIAR_PA
		(((uint32_t)reg << 6) & ETH_MACMIIAR_MR) |
		ETH_MACMIIAR_MW | ETH_MACMIIAR_MB;

	uint32_t	timeout = 0x100000;
	while( ( ETH->MACMIIAR & ETH_MACMIIAR_MB ) && --timeout );

	// If timeout = 0, is an error.
	return timeout ? 0 : -1;
}

static int ch32v307ethPHYRegRead( uint32_t reg )
{
	ETH->MACMIIAR = ETH_MACMIIAR_CR_Div42 /* = 0, per 27.1.8.1.4 */ |
		((uint32_t)CH32V307GIGABIT_PHYADDRESS << 11) | // ETH_MACMIIAR_PA
		(((uint32_t)reg << 6) & ETH_MACMIIAR_MR) |
		(0 /*!ETH_MACMIIAR_MW*/) | ETH_MACMIIAR_MB;

	uint32_t	timeout = 0x100000;
	while( ( ETH->MACMIIAR & ETH_MACMIIAR_MB ) && --timeout );

	// If timeout = 0, is an error.
	return timeout ? ETH->MACMIIDR : -1;
}


static void ch32v307ethGetMacInUC( uint8_t * mac )
{
	// Mac is backwards.
	const uint8_t *macaddr = (const uint8_t *)(ROM_CFG_USERADR_ID+5);
	for( int i = 0; i < 6; i++ )
	{
		mac[i] = *(macaddr--);
	}
}

static int ch32v307ethInit( void )
{
	int i;

#ifdef CH32V307GIGABIT_PHY_RSTB
	funPinMode( CH32V307GIGABIT_PHY_RSTB, GPIO_CFGLR_OUT_50Mhz_PP ); //PHY_RSTB (For reset)
	funDigitalWrite( CH32V307GIGABIT_PHY_RSTB, FUN_LOW );
#endif

	// Configure strapping.
	funPinMode( PA1, GPIO_CFGLR_IN_PUPD ); // GMII_RXD3
	funPinMode( PA0, GPIO_CFGLR_IN_PUPD ); // GMII_RXD2
	funPinMode( PC3, GPIO_CFGLR_IN_PUPD ); // GMII_RXD1
	funPinMode( PC2, GPIO_CFGLR_IN_PUPD ); // GMII_RXD0
	funDigitalWrite( PA1, FUN_HIGH );
	funDigitalWrite( PA0, FUN_HIGH );
	funDigitalWrite( PC3, FUN_HIGH ); // No TX Delay
	funDigitalWrite( PC2, FUN_HIGH );

	// Pull-up MDIO
	funPinMode( PD9, GPIO_CFGLR_OUT_50Mhz_PP ); //Pull-up control (DO NOT CHECK IN, ADD RESISTOR)
	funDigitalWrite( PD9, FUN_HIGH );

	// Will be required later.
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

	// https://cnlohr.github.io/microclockoptimizer/?chipSelect=ch32vx05_7%2Cd8c&HSI=1,8&HSE=0,8&PREDIV2=1,1&PLL2CLK=1,7&PLL2VCO=0,72&PLL3CLK=1,1&PLL3VCO=0,100&PREDIV1SRC=1,0&PREDIV1=1,2&PLLSRC=1,0&PLL=0,4&PLLVCO=1,144&SYSCLK=1,2&
	// Clock Tree:
	// 8MHz Input
	// PREDIV2 = 2 (1 in register) = 4MHz
	// PLL2 = 9 (7 in register) = 36MHz / PLL2VCO = 72MHz
	// PLL3CLK = 12.5 (1 in register) = 50MHz = 100MHz VCO
	// PREDIV1SRC = HSE (1 in register) = 8MHz
	// PREDIV1 = 2 (1 in register).
	// PLLSRC = PREDIV1 (0 in register) = 4MHz
	// PLL = 18 (0 in register) = 72MHz 
	// PLLVCO = 144MHz 
	// SYSCLK = PLLVCO = 144MHz
	// Use EXT_125M (ETH1G_SRC)

	// Switch processor back to HSI so we don't eat dirt.
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_HSI;

	// Setup clock tree.
	RCC->CFGR2 |= 
		(1<<RCC_PREDIV2_OFFSET) | // PREDIV = /2; Prediv Freq = 4MHz
		(1<<RCC_PLL3MUL_OFFSET) | // PLL3 = x12.5 (PLL3 = 50MHz)
		(2<<RCC_ETH1GSRC_OFFSET)| // External source for RGMII
		(7<<RCC_PLL2MUL_OFFSET) | // PLL2 = x9 (PLL2 = 36MHz)
		(1<<RCC_PREDIV1_OFFSET) | // PREDIV1 = /2; Prediv freq = 50MHz
		0;

	// Power on PLLs
	RCC->CTLR |= RCC_PLL3ON | RCC_PLL2ON;
	int timeout;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL3RDY) break;
	if( timeout == 0 ) return -5;
	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL2RDY) break;
	if( timeout == 0 ) return -6;

	// PLL = x18 (0 in register)
	RCC->CFGR0 = ( RCC->CFGR0 & ~(0xf<<18)) | 0;
	RCC->CTLR |= RCC_PLLON;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLLRDY) break;
	if( timeout == 0 ) return -7;

	// Switch to PLL.
#ifdef CH32V307GIGABIT_MCO25
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL | (9<<24); // And output clock on PA8
#else
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;
#endif

	// For clock in.
	funPinMode( PB1, GPIO_CFGLR_IN_FLOAT ); //GMII_CLK125

	RCC->CFGR2 |= RCC_ETH1G_125M_EN; // Enable 125MHz clock.

	// Power on and reset.
	RCC->AHBPCENR |= RCC_ETHMACEN | RCC_ETHMACTXEN | RCC_ETHMACRXEN;
	RCC->AHBRSTR |= RCC_ETHMACRST;
	RCC->AHBRSTR &=~RCC_ETHMACRST;

	ETH->DMABMR |= ETH_DMABMR_SR;

	// Wait for reset to complete.
	for( timeout = 10000; timeout > 0 && (ETH->DMABMR & ETH_DMABMR_SR); timeout-- )
	{
		Delay_Us(10);
	}

	// Use RGMII
	EXTEN->EXTEN_CTR |= EXTEN_ETH_RGMII_SEL; //EXTEN_ETH_RGMII_SEL;

	funPinMode( PB13, GPIO_CFGLR_OUT_50Mhz_AF_PP ); //GMII_MDIO
	funPinMode( PB12, GPIO_CFGLR_OUT_50Mhz_AF_PP ); //GMII_MDC

	// For clock output to Ethernet module.
	funPinMode( PA8, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // PHY_CKTAL

	// Release PHY from reset.
#ifdef CH32V307GIGABIT_PHY_RSTB
	funDigitalWrite( CH32V307GIGABIT_PHY_RSTB, FUN_HIGH );
#endif

	Delay_Ms(25); 	// Waiting for PHY to exit sleep.  This is inconsistent at 23ms (But only on the RTL8211FS) None is needed on the RTL8211E

	funPinMode( PB0, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXD3
	funPinMode( PC5, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXD2
	funPinMode( PC4, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXD1
	funPinMode( PA7, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXD0
	funPinMode( PA3, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXCTL
	funPinMode( PA2, GPIO_CFGLR_OUT_50Mhz_AF_PP ); // GMII_TXC
	funPinMode( PA1, GPIO_CFGLR_IN_PUPD ); // GMII_RXD3
	funPinMode( PA0, GPIO_CFGLR_IN_PUPD ); // GMII_RXD2
	funPinMode( PC3, GPIO_CFGLR_IN_PUPD ); // GMII_RXD1
	funPinMode( PC2, GPIO_CFGLR_IN_PUPD ); // GMII_RXD0
	funPinMode( PC1, GPIO_CFGLR_IN_PUPD ); // GMII_RXCTL
	funPinMode( PC0, GPIO_CFGLR_IN_FLOAT ); // GMII_RXC

	funDigitalWrite( PA1, FUN_HIGH ); // SELGRV = 3.3V
	funDigitalWrite( PA0, FUN_HIGH ); // TXDelay = 1
	funDigitalWrite( PC3, FUN_HIGH ); // AN[0] = 1
	funDigitalWrite( PC2, FUN_HIGH ); // AN[1] = 1
	funDigitalWrite( PC1, FUN_LOW );  // PHYAD[0]

	// Configure MDC/MDIO
	// Conflicting notes - some say /42, others don't.
	ETH->MACMIIAR = ETH_MACMIIAR_CR_Div42;

	// Update MACCR
	ETH->MACCR =
		( CH32V307GIGABIT_CFG_CLOCK_DELAY << 29 ) | // No clock delay
		( 0 << 23 ) | // Max RX = 2kB (Revisit if looking into jumbo frames)
		( 0 << 22 ) | // Max TX = 2kB (Revisit if looking into jumbo frames)
		( 0 << 21 ) | // Rated Drive (instead of energy savings mode) (10M PHY only)
		( 1 << 20 ) | // Bizarre re-use of termination resistor terminology? (10M PHY Only)
		( 0 << 17 ) | // IFG = 0, 96-bit guard time.
		( 0 << 14 ) | // FES = 2 = GBE, 1=100MBit/s (UNSET TO START)
		( 0 << 12 ) | // Self Loop = 0
		( 0 << 11 ) | // Full-Duplex Mode (UNSET TO START)
		( 1 << 10 ) | // IPCO = 1, Check TCP, UDP, ICMP header checksums.
		( 1 << 7  ) | // APCS (automatically strip frames)
		( 1 << 3  ) | // TE (Transmit enable!)
		( 1 << 2  ) | // RE (Receive Enable)
		( CH32V307GIGABIT_CFG_CLOCK_PHASE << 1  ) | // TCF = 0 (Potentailly change if clocking is wrong)
		0;

	Delay_Ms(25); 	// Waiting for PHY to exit sleep.  This is inconsistent at 19ms.

	// Reset the physical layer
	ch32v307ethPHYRegWrite( PHY_BCR,
		PHY_Reset |
		1<<12 | // Auto negotiate
		1<<8 | // Duplex
		1<<6 | // Speed Bit.
		0 );

	// De-assert reset.
	ch32v307ethPHYRegWrite( PHY_BCR,
		1<<12 | // Auto negotiate
		1<<8 | // Duplex
		1<<6 | // Speed Bit.
		0 );

	ch32v307ethPHYRegRead( 0x03 );
	ch32v307eth_phyid = ch32v307ethPHYRegRead( 0x03 ); // Read twice to be safe.
	if( ch32v307eth_phyid == 0xc916 )
		ch32v307ethPHYRegWrite( 0x1F, 0x0a43 ); // RTL8211FS needs page select.

	ch32v307ethGetMacInUC( ch32v307eth_mac );

	ETH->MACA0HR = (uint32_t)((ch32v307eth_mac[5]<<8) | ch32v307eth_mac[4]);
	ETH->MACA0LR = (uint32_t)(ch32v307eth_mac[0] | (ch32v307eth_mac[1]<<8) | (ch32v307eth_mac[2]<<16) | (ch32v307eth_mac[3]<<24));

	ETH->MACFFR = (uint32_t)(ETH_ReceiveAll_Disable |
		ETH_SourceAddrFilter_Disable |
		ETH_PassControlFrames_BlockAll |
		ETH_BroadcastFramesReception_Enable |
		ETH_DestinationAddrFilter_Normal |
		ETH_PromiscuousMode_Disable |
		ETH_MulticastFramesFilter_Perfect |
		ETH_UnicastFramesFilter_Perfect);

	ETH->MACHTHR = (uint32_t)0;
	ETH->MACHTLR = (uint32_t)0;
	ETH->MACVLANTR = (uint32_t)(ETH_VLANTagComparison_16Bit);

	ETH->MACFCR = 0; // No pause frames.

	// Configure RX/TX chains.
	ETH_DMADESCTypeDef *tdesc;
	for(i = 0; i < CH32V307GIGABIT_TXBUFNB; i++)
	{
		tdesc = ch32v307eth_DMATxDscrTab + i;
		tdesc->ControlBufferSize = 0;
		tdesc->Status = ETH_DMATxDesc_TCH | ETH_DMATxDesc_IC | ETH_DMATxDesc_FS;
		tdesc->Buffer1Addr = (uint32_t)0; // Populate with data.
		tdesc->Buffer2NextDescAddr = (i < CH32V307GIGABIT_TXBUFNB - 1) ? ((uint32_t)(ch32v307eth_DMATxDscrTab + i + 1)) : (uint32_t)ch32v307eth_DMATxDscrTab;
	}
	ETH->DMATDLAR = (uint32_t)ch32v307eth_DMATxDscrTab;
	for(i = 0; i < CH32V307GIGABIT_RXBUFNB; i++)
	{
		tdesc = ch32v307eth_DMARxDscrTab + i;
		tdesc->Status = ETH_DMARxDesc_OWN;
		tdesc->ControlBufferSize = ETH_DMARxDesc_RCH | (uint32_t)CH32V307GIGABIT_BUFFSIZE;
		tdesc->Buffer1Addr = (uint32_t)(&ch32v307eth_MACRxBuf[i * CH32V307GIGABIT_BUFFSIZE]);
		tdesc->Buffer2NextDescAddr = (i < CH32V307GIGABIT_RXBUFNB - 1) ? (uint32_t)(ch32v307eth_DMARxDscrTab + i + 1) : (uint32_t)(ch32v307eth_DMARxDscrTab);
	}
	ETH->DMARDLAR = (uint32_t)ch32v307eth_DMARxDscrTab;

	pDMARxGet = ch32v307eth_DMARxDscrTab;
	pDMATxSet = ch32v307eth_DMATxDscrTab;

	// Receive a good frame half interrupt mask.
	// Receive CRC error frame half interrupt mask.
	// For the future: Why do we want this?
	ETH->MMCTIMR = ETH_MMCTIMR_TGFM;
	ETH->MMCRIMR = ETH_MMCRIMR_RGUFM | ETH_MMCRIMR_RFCEM;

	ETH->DMAIER = ETH_DMA_IT_NIS | // Normal interrupt enable.
				ETH_DMA_IT_R |     // Receive
				ETH_DMA_IT_T |     // Transmit
				ETH_DMA_IT_AIS |   // Abnormal interrupt 
				ETH_DMA_IT_RBU;    // Receive buffer unavailable interrupt enable

	NVIC_EnableIRQ( ETH_IRQn );

	// Actually enable receiving process.
	ETH->DMAOMR = ETH_DMAOMR_SR | ETH_DMAOMR_ST | ETH_DMAOMR_TSF | ETH_DMAOMR_FEF;

	return 0;
}

void ETH_IRQHandler( void ) __attribute__((interrupt));
void ETH_IRQHandler( void )
{
    uint32_t int_sta;

	do
	{
		int_sta = ETH->DMASR;
		if ( ( int_sta & ( ETH_DMA_IT_AIS | ETH_DMA_IT_NIS ) ) == 0 )
		{
			break;
		}

		// Off nominal situations.
		if (int_sta & ETH_DMA_IT_AIS)
		{
			// Receive buffer unavailable interrupt enable.
		    if (int_sta & ETH_DMA_IT_RBU)
		    {
		        ETH->DMASR = ETH_DMA_IT_RBU;
		        if((INFO->CHIPID & 0xf0) == 0x10)
		        {
		            ((ETH_DMADESCTypeDef *)(((ETH_DMADESCTypeDef *)(ETH->DMACHRDR))->Buffer2NextDescAddr))->Status = ETH_DMARxDesc_OWN;
		            ETH->DMARPDR = 0;
		        }
		    }
		    ETH->DMASR = ETH_DMA_IT_AIS;
		}

		// Nominal interrupts.
		if( int_sta & ETH_DMA_IT_NIS )
		{
		    if( int_sta & ETH_DMA_IT_R )
		    {
				// Received a packet, normally.
				// Status is in Table 27-17 Definitions of RDes0
				do
				{
					// XXX TODO: Is this a good place to acknowledge? REVISIT: Should this go lower?
					// XXX TODO: Restructure this to allow for 
				    ETH->DMASR = ETH_DMA_IT_R;

					uint32_t status = pDMARxGet->Status;
					if( status & ETH_DMARxDesc_OWN ) break;

					// We only have a valid packet in a specific situation.
					// So, we take the status, then mask off the bits we care about
					// And see if they're equal to the ones that need to be set/unset.
					const uint32_t mask = 
						ETH_DMARxDesc_OWN |
						ETH_DMARxDesc_LS |
						ETH_DMARxDesc_ES |
						ETH_DMARxDesc_FS;
					const uint32_t eq = 
						0 |
						ETH_DMARxDesc_LS |
						0 |
						ETH_DMARxDesc_FS;

					int suppress_own = 0;

					if( ( status & mask ) == eq )
					{
						int32_t frame_length = ((status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT) - 4;
						if( frame_length > 0 )
						{
							uint8_t * data = (uint8_t*)pDMARxGet->Buffer1Addr;
							suppress_own = ch32v307ethInitHandlePacket( data, frame_length, pDMARxGet );
						}
					}
					// Otherwise, Invalid Packet

					// Relinquish control back to underlying hardware.
					if( !suppress_own )
						pDMARxGet->Status = ETH_DMARxDesc_OWN;

					// Tricky logic for figuring out the next packet. Originally
					// discussed in ch32v30x_eth.c in ETH_DropRxPkt
					if((pDMARxGet->ControlBufferSize & ETH_DMARxDesc_RCH) != (uint32_t)RESET)
						pDMARxGet = (ETH_DMADESCTypeDef *)(pDMARxGet->Buffer2NextDescAddr);
					else
					{
						if((pDMARxGet->ControlBufferSize & ETH_DMARxDesc_RER) != (uint32_t)RESET)
							pDMARxGet = (ETH_DMADESCTypeDef *)(ETH->DMARDLAR);
						else
							pDMARxGet = (ETH_DMADESCTypeDef *)((uint32_t)pDMARxGet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));
					}
				} while( 1 );
		    }
		    if( int_sta & ETH_DMA_IT_T )
		    {
				ch32v307ethInitHandleTXC();
		        ETH->DMASR = ETH_DMA_IT_T;
		    }
		    ETH->DMASR = ETH_DMA_IT_NIS;
		}
	} while( 1 );
}

static int ch32v307ethTransmitStatic(uint8_t * buffer, uint32_t length, int enable_txc)
{
	// The official SDK waits until ETH_DMATxDesc_TTSS is set.
	// This also provides a transmit timestamp, which could be
	// used for PTP.
	// But we don't want to do that.
	// We just want to go.  If anyone cares, they can check later.

	if( pDMATxSet->Status & ETH_DMATxDesc_OWN )
	{
		ETH->DMATPDR = 0;
		return -1;
	}

    pDMATxSet->ControlBufferSize = (length & ETH_DMATxDesc_TBS1);
	pDMATxSet->Buffer1Addr = (uint32_t)buffer;

	// Status is in Table 27-12 "Definitions of TDes0 bits"
	enable_txc = enable_txc ? ETH_DMATxDesc_IC : 0;
    pDMATxSet->Status = 
		ETH_DMATxDesc_LS |                  // Last Segment (This is all you need to have to transmit)
		ETH_DMATxDesc_FS |                  // First Segment (Beginning of transmission)
		enable_txc |                        // Interrupt when complete
		ETH_DMATxDesc_TCH |                 // Next Descriptor Address Valid
		ETH_DMATxDesc_CIC_TCPUDPICMP_Full | // Do all header checksums.
		ETH_DMATxDesc_OWN;                  // Own back to hardware

	pDMATxSet = (ETH_DMADESCTypeDef*)pDMATxSet->Buffer2NextDescAddr;

	ETH->DMASR = ETH_DMASR_TBUS; // This resets the transmit process (or "starts" it)
	ETH->DMATPDR = 0;

	return 0;
}


#endif

