/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file contains user callback structure for USB events Management
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
#include "usbd_usr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define ADC1_DR_Address    0x40012440

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,   
};

//uint32_t ADC_ConvertedValueX = 0;
//uint32_t ADC_ConvertedValueX_1 = 0;

/* Private function prototypes -----------------------------------------------*/
//static void ADC_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Device lib initialization
  * @param  None
  * @retval None
  */
void USBD_USR_Init(void)
{   
  /* Initialize LEDs */
  //STM_EVAL_LEDInit(LED1);
  //STM_EVAL_LEDInit(LED2);
  //STM_EVAL_LEDInit(LED3);
  //STM_EVAL_LEDInit(LED4);
  
  /* Configure the ADC*/
  //ADC_Configuration();
  
}

/**
  * @brief  Reset Event
  * @param  speed : device speed
  * @retval None
  */
void USBD_USR_DeviceReset(uint8_t speed )
{
}

/**
  * @brief  Configuration Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceConfigured (void)
{
}

/**
  * @brief  Device suspend Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceSuspended(void)
{
}


/**
  * @brief  Device resume Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceResumed(void)
{
}

/**
  * @brief  ADC_Configuration
*         Configure the ADC
  * @param  None
  * @retval None
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
