#include "stm32f0xx.h"
#include  "usbd_usr.h"
#include "usbd_desc.h"

extern USBD_Class_cb_TypeDef  USBD_HID_cb;
USB_CORE_HANDLE  USB_Device_dev ;

GPIO_InitTypeDef gpioInit;

int main(void) {
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;	
	// remap pins to enable USB
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	USBD_Init(&USB_Device_dev,
            &USR_desc, 
            &USBD_HID_cb, 
            &USR_cb);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	gpioInit.GPIO_Mode = GPIO_Mode_OUT;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_1;
	gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_Level_3;
	
	GPIO_Init(GPIOA, &gpioInit);
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	
	while(1) {
		
	}
}
