/**
 * @file sysclk_config.c
 * @author Piotr Zapart
 * @brief Testing various sysclock configurations for the ch32v003
 * 			LED Pin definition for the NanoCH32V003 board
 * 			Try different conbinations in the funconfig.h file, ie:
 * 
 * 				#define FUNCONF_USE_HSI 1    
 *				#define FUNCONF_USE_PLL 1       
 *				#define CH32V003        1
 * 
 * 				#define FUNCONF_USE_HSE 1    
 *				#define FUNCONF_USE_PLL 0       
 *				#define CH32V003        1
 * 
 * @version 1.0
 * @date 2023-07-19
 */

// LED is on D.6 (nanoCH32 board)
#define LED_PIN     6				

#include <stdarg.h>
#include "ch32v003fun.h"
#include <stdio.h>
#include "ch32v003_GPIO_branchless.h"

const char msg_sep[] = "====================================\r\n";
const char msg_title[] = "System Clock Configuration Example\r\n";
const char msg_en[] = "Enabled";
const char msg_dis[] = "Disabled";
const char *msg_clkSrc[] = {"HSI ", "HSE" , "PLL "};
const char *msg_mco[] = {"Disabled", "SYSCLK", "HSI 24MHz", "HSE", "PLL"};
const char msg_clkFail[] = "HSE Fail, switched to HSI, no PLL\r\n";

typedef enum
{
	SYSCLK_HSI, 
	SYSCLK_HSI_PLL,
	SYSCLK_HSE,
	SYSCLK_HSE_PLL,
	SYSCLK_EXTCLK,
	SYSCLK_EXTCLK_PLL
}rcc_sysclk_status_e;

typedef enum
{
	MCO_DISABLED 		= 0,
	MCO_OUT_SYSCLK 		= (4<<24),
	MCO_OUT_HSI			= (5<<24),
	MCO_OUT_HSE			= (6<<24),
	MCO_OUT_PLL			= (7<<24)
}mco_cfg_e;

void print_sysclk_cfg(void);
static inline void MCO_cfg(mco_cfg_e cfg);
static inline uint8_t getMCOidx(uint32_t mco);
void UART_setup(int uartBRR);
int UART_write(int fd, const char *buf, int size);
static int UART_puts(char *s, int len, void *buf);
int UART_printf(const char* format, ...);		
int mini_vpprintf(int (*puts)(char* s, int len, void* buf), void* buf, const char *fmt, va_list va);

// --------------------------------------------------------
static inline void MCO_cfg(mco_cfg_e cfg)
{
	RCC->CFGR0 &= ~RCC_CFGR0_MCO;
	RCC->CFGR0 |=  cfg & RCC_CFGR0_MCO;
}
// --------------------------------------------------------
static inline uint8_t getMCOidx(uint32_t mco)
{
	mco >>= 24;
	return (mco ? mco-3 : mco);
}
// --------------------------------------------------------
/**
 * @brief prints out al the clock related details on both
 * 		channels: debug printf and UART in cas the HSI is
 * 		disabled and the debug channel can no longer work.
 */
void print_sysclk_cfg(void)
{
	uint32_t ctlr = RCC->CTLR;
	uint32_t cfgr0 = RCC->CFGR0;
	const char *msg_out;
	uint32_t tmp;

	printf("%s", msg_sep); UART_printf("%s", msg_sep);
	printf("%s", msg_title); UART_printf("%s", msg_title);
	printf("%s", msg_sep); UART_printf("%s", msg_sep);
	// HSI state
	msg_out = ctlr & RCC_HSION ? msg_en : msg_dis;
	printf("HSI %s\r\n", msg_out); 
	UART_printf("HSI %s\r\n", msg_out);
	// HSI trim value
	tmp = (ctlr & RCC_HSITRIM)>>3;
	printf("HSI trim = %ld\r\n", tmp); 
	UART_printf("HSI trim = %ld\r\n",tmp); 
	// HSI callibration value
	tmp = (ctlr & RCC_HSICAL)>>8;
	printf("HSI cal = %ld\r\n",tmp); 
	UART_printf("HSI cal = %ld\r\n",tmp);
	// HSE state
	msg_out = ctlr & RCC_HSEON ? msg_en : msg_dis;
	printf("HSE %s\r\n", msg_out); 
	UART_printf("HSE %s\r\n", msg_out);
	// PLL state
	msg_out = ctlr & RCC_PLLON ? msg_en : msg_dis;
	printf("PLL %s\r\n", msg_out); 
	UART_printf("PLL %s\r\n", msg_out);
	// PLL Source
	msg_out = msg_clkSrc[(cfgr0 & (1<<16))>>16];
	printf("PLL Source = %s\r\n", msg_out); 
	UART_printf("PLL Source = %s\r\n", msg_out);
	// Clock Security system
	msg_out = ctlr & RCC_CSSON ? msg_en : msg_dis;
	printf("Clk Security %s\r\n", msg_out);
	UART_printf("Clk Security %s\r\n", msg_out);
	// HSE bypass
	msg_out = ctlr & RCC_HSEBYP ? msg_en : msg_dis;
	printf("HSE bypass %s\r\n", msg_out);
	UART_printf("HSE bypass %s\r\n", msg_out);
	// Sysclock source
	msg_out = msg_clkSrc[(cfgr0 & 0x0C)>>2];
	printf("Sysclk source = %s\r\n", msg_out);
	UART_printf("Sysclk source = %s\r\n", msg_out);
	// MCO setting
	msg_out = msg_mco[getMCOidx(cfgr0 & RCC_CFGR0_MCO)];
	printf("MCO output: %s\r\n", msg_out);	
	UART_printf("MCO output: %s\r\n", msg_out);	
}
// --------------------------------------------------------
void UART_setup( int uartBRR )
{
	// Enable GPIOD and UART.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;
	// Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
	GPIOD->CFGLR &= ~(0xf<<(4*5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
	
	USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx;
	USART1->CTLR2 = USART_StopBits_1;
	USART1->CTLR3 = USART_HardwareFlowControl_None;

	USART1->BRR = uartBRR;
	USART1->CTLR1 |= CTLR1_UE_Set;
}
// --------------------------------------------------------
int UART_write(int fd, const char *buf, int size)
{
	for(int i = 0; i < size; i++){
	    while( !(USART1->STATR & USART_FLAG_TC));
	    USART1->DATAR = *buf++;
	}
	return size;
}
// --------------------------------------------------------
static int UART_puts(char *s, int len, void *buf)
{
	UART_write( 0, s, len );
	return len;
}
// --------------------------------------------------------
int UART_printf(const char* format, ...)
{
	va_list args;
	va_start( args, format );
	int ret_status = mini_vpprintf(UART_puts, 0, format, args);
	va_end( args );
	return ret_status;
}
// --------------------------------------------------------
int main()
{	
	SystemInit();                                        
	UART_setup(UART_BRR);
	GPIO_port_enable(GPIO_port_D);
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, LED_PIN), GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_D, LED_PIN), low);	
	// needed for MCO output
    GPIO_port_enable(GPIO_port_C);
    GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, 4), GPIO_pinMode_O_pushPullMux, GPIO_Speed_50MHz);
	
	MCO_cfg(MCO_OUT_SYSCLK);

	print_sysclk_cfg();

	while(1)
    {
		// blink onboard led to show the mcu is running
        GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_D, LED_PIN), high);
        Delay_Ms(100);
        GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_D, LED_PIN), low);
        Delay_Ms(100);
		// detect clock failsafe, if HSE was enabled but the current source is HSI/NoPLL -
		// the clock security system did it's job.
#if defined(FUNCONF_USE_HSE)		
		if (FUNCONF_USE_HSE && (RCC->CFGR0 & 0x0C) == 0x00)
		{
			printf("%s%s%s", msg_sep, msg_clkFail, msg_sep);
			// UART, depending on the crystal value and PLL setting, most likely will
			// have bad baudrate setting unless the original clock was HSE only, 24MHz, No PLL
			//UART_printf("%s%s%s", msg_sep, msg_clkFail, msg_sep);
		}
#endif		
    }
}