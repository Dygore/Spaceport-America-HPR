/*
 * Status_LED.c
 *
 *  Created on: May 10, 2022
 *      Author: Dylan
 */

#include "stm32f4xx_hal.h"
#include "Status_LED.h"

void status_LED_Swap(void){
	if (!HAL_GPIO_ReadPin(GPIOB, Status_LED_Pin)){
		HAL_GPIO_WritePin(GPIOB, Status_LED_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB, Status_LED_Pin, GPIO_PIN_RESET);
	}
}
