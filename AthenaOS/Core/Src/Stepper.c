/*
 * Stepper.c
 *
 *  Created on: May 10, 2022
 *      Author: Dylan
 */

#include "stm32f4xx_hal.h"
#include "stepper.h"

void stepper_Step (int dir, int step){
	HAL_GPIO_WritePin(GPIOC, Step_EN_Pin, GPIO_PIN_RESET);

	if (dir == 1){
		HAL_GPIO_WritePin(GPIOB, Step_DIR_Pin, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB, Step_DIR_Pin, GPIO_PIN_SET);
	}

	for(int i = 0; i < step; i++){
		HAL_GPIO_WritePin(GPIOB, Step_PWM_Pin, GPIO_PIN_RESET);
		delay(200);
		HAL_GPIO_WritePin(GPIOB, Step_PWM_Pin, GPIO_PIN_SET);
		delay(200);
	}

	HAL_GPIO_WritePin(GPIOC, Step_EN_Pin, GPIO_PIN_SET);
}
