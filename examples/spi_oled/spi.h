/*
 * Single-File-Header for SPI
 * 05-05-2023 E. Brombaugh
 */

#ifndef _SPI_H
#define _SPI_H

/*
 * init SPI and GPIO
 */
void spi_init(void)
{
	// Enable GPIOC and SPI
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;
	
	// PC5 is SCK, 10MHz Output, alt func, p-p
	GPIOC->CFGLR &= ~(0xf<<(4*5));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
	
	// PC6 is MOSI, 10MHz Output, alt func, p-p
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
	
	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_8b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		SPI_BaudRatePrescaler_2;

	// enable SPI port
	SPI1->CTLR1 |= CTLR1_SPE_Set;

}

/*
 * packet send for polled operation
 */
uint8_t spi_send(uint8_t *data, uint8_t sz)
{
	// send data
	while(sz--)
	{
		// wait for TXE
		while(!(SPI1->STATR & SPI_STATR_TXE));
		
		// Send byte
		SPI1->DATAR = *data++;
	}
	
	// wait for not busy before exiting
	while(SPI1->STATR & SPI_STATR_BSY);
	
	// we're happy
	return 0;
}

#endif
