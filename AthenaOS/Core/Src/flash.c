/*
 * flash.c
 *
 *  Created on: May 31, 2022
 *      Author: Dylan
 */

#include "stm32f4xx_hal.h"
#include "stepper.h"

void checkBusyRead(){
	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x05);

	while(W25Q_Spi(0x00) != 0x00){}

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);
}

void checkBusyWrite(){
	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x01);

	while(W25Q_Spi(0x00) != 0x00){}

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);
}

void writeEnable(){
	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x06);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);
}

void writeDisable(){
	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x04);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);
}

uint8_t flash_readID()
{
	uint8_t Temp = 0;
	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x90);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	Temp = W25Q_Spi(0x00);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	return Temp;
}

uint8_t flashRead(uint8_t addr3, uint8_t addr2, uint8_t addr1){
	uint8_t Temp = 0;

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x03);

	W25Q_Spi(addr3);

	W25Q_Spi(addr2);

	W25Q_Spi(addr1);

	Temp = W25Q_Spi(0x00);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(25);

	checkBusyRead();

	return Temp;
}

void flashWriteSingle(uint8_t addr3, uint8_t addr2, uint8_t addr1, uint8_t data){
	checkBusyWrite();
	checkBusyRead();
	writeEnable();

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x02);

	W25Q_Spi(addr3);

	W25Q_Spi(addr2);

	W25Q_Spi(addr1);

	W25Q_Spi(data);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(25);

	checkBusyWrite();
	checkBusyRead();
}

void flashWriteArray(uint8_t addr3, uint8_t addr2, uint8_t addr1, uint8_t data[], int dataSize){
	checkBusyWrite();
	checkBusyRead();
	writeEnable();

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0x02);

	W25Q_Spi(addr3);

	W25Q_Spi(addr2);

	W25Q_Spi(addr1);

	for(int i = 0; i < dataSize; i++){
		W25Q_Spi(data[i]);
	}

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(25);

	checkBusyRead();
	checkBusyWrite();
}

void flashClearLookup(){
	checkBusyWrite();
	checkBusyRead();
	writeEnable();

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0xD8);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(25);

	checkBusyWrite();
}

void flashClearStorage(){
	checkBusyWrite();
	checkBusyRead();
	writeEnable();

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0xD8);

	W25Q_Spi(0x7F);

	W25Q_Spi(0x00);

	W25Q_Spi(0x00);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(25);

	checkBusyWrite();
}

void flashClearAll(){
	checkBusyWrite();
	checkBusyRead();
	writeEnable();

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_RESET);

	W25Q_Spi(0xC7);

	HAL_GPIO_WritePin(GPIOC, Flash_CS_Pin, GPIO_PIN_SET);

	HAL_Delay(50);

	checkBusyWrite();
}
