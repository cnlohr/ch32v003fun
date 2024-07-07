/*
 * Single-File-Header for SSD1306 SPI interface
 * 05-05-2023 E. Brombaugh
 */

#ifndef _SSD1306_SPI_H
#define _SSD1306_SPI_H

// control pins
#define SSD1306_RST_PORT GPIOC
#define SSD1306_RST_PIN 2
#define SSD1306_RST_HIGH() SSD1306_RST_PORT->BSHR = (1<<(SSD1306_RST_PIN))
#define SSD1306_RST_LOW() SSD1306_RST_PORT->BSHR = (1<<(16+SSD1306_RST_PIN))
#define SSD1306_CS_PORT GPIOC
#define SSD1306_CS_PIN 3
#define SSD1306_CS_HIGH() SSD1306_CS_PORT->BSHR = (1<<(SSD1306_CS_PIN))
#define SSD1306_CS_LOW() SSD1306_CS_PORT->BSHR = (1<<(16+SSD1306_CS_PIN))
#define SSD1306_DC_PIN 4
#define SSD1306_DC_PORT GPIOC
#define SSD1306_DC_HIGH() SSD1306_DC_PORT->BSHR = (1<<(SSD1306_DC_PIN))
#define SSD1306_DC_LOW() SSD1306_DC_PORT->BSHR = (1<<(16+SSD1306_DC_PIN))

/*
 * init SPI and GPIO for SSD1306 OLED
 */
uint8_t ssd1306_spi_init(void)
{
	// Enable GPIOC and SPI
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;
	
	// setup GPIO for reset, chip select and data/cmd
	SSD1306_RST_PORT->CFGLR &= ~(0xf<<(4*SSD1306_RST_PIN));
	SSD1306_RST_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SSD1306_RST_PIN);
	SSD1306_RST_HIGH();
	SSD1306_CS_PORT->CFGLR &= ~(0xf<<(4*SSD1306_CS_PIN));
	SSD1306_CS_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SSD1306_CS_PIN);
	SSD1306_CS_HIGH();
	SSD1306_DC_PORT->CFGLR &= ~(0xf<<(4*SSD1306_DC_PIN));
	SSD1306_DC_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SSD1306_DC_PIN);
	SSD1306_DC_LOW();

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
	
	// always succeed
	return 0;
}

/*
 * toggle reset line
 */
void ssd1306_rst(void)
{
	SSD1306_RST_LOW();
	Delay_Ms(10);
	SSD1306_RST_HIGH();
}

/*
 * packet send for blocking polled operation via spi
 */
uint8_t ssd1306_pkt_send(uint8_t *data, uint8_t sz, uint8_t cmd)
{
	if(cmd)
		SSD1306_DC_LOW();
	else
		SSD1306_DC_HIGH();
	SSD1306_CS_LOW();
	
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
	
	SSD1306_CS_HIGH();
	
	// we're happy
	return 0;
}

#endif
