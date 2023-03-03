/**
  ******************************************************************************
  * @file    stm32_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stm32_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{ 
} 

/**
  * @brief  This function handles USB FS Handler.
  * @param  None
  * @retval None
  */
void USB_IRQHandler(void)
{
  USB_Istr();
}

/**
  * @brief  This function handles EXTI14_15_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI4_15_IRQHandler(void)
{
  
  if (EXTI_GetITStatus(TAMPER_BUTTON_EXTI_LINE) != RESET)
  {
    /*
	  if((PrevXferDone)  && (USB_Device_dev.dev.device_status==USB_CONFIGURED))
    {
      Send_Buffer[0] = 0x06; 
      
      if (STM_EVAL_PBGetState(BUTTON_TAMPER) == Bit_RESET)
      {
        Send_Buffer[1] = 0x01;
      }
      else
      {
        Send_Buffer[1] = 0x00;
      }

      USBD_HID_SendReport (&USB_Device_dev, Send_Buffer, 2);  
      PrevXferDone = 0;
    }
    */
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(TAMPER_BUTTON_EXTI_LINE);
  }
}

/**
  * @brief  This function handles DMA1_Channel1 Handler.
  * @param  None
  * @retval None
  */
void DMA1_Channel1_IRQHandler(void)
{  
  /*
	Send_Buffer[0] = 0x07;
  
  if((ADC_ConvertedValueX >>4) - (ADC_ConvertedValueX_1 >>4) > 4)
  {
    if ((PrevXferDone) && (USB_Device_dev.dev.device_status == USB_CONFIGURED))
    {
      Send_Buffer[1] = (uint8_t)(ADC_ConvertedValueX >>4);
      
      //USBD_HID_SendReport (&USB_Device_dev, Send_Buffer, 2);
      
      ADC_ConvertedValueX_1 = ADC_ConvertedValueX;
      PrevXferDone = 0;
    }
  }
	*/
    /* Test DMA1 TC flag */
  //while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET );
  
  //DMA_ClearFlag(DMA1_FLAG_TC1);
}
         
/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f072.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
