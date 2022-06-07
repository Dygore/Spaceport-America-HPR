/*
 * flash.h
 *
 *  Created on: May 31, 2022
 *      Author: Dylan
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

uint8_t flash_readID(void);

void writeEnable(void);

void writeDisable(void);

uint8_t flashRead(uint8_t addr3, uint8_t addr2, uint8_t addr1);

void flashClearLookup(void);

void flashClearStorage(void);

void flashClearAll(void);

void flashWriteSingle(uint8_t addr3, uint8_t addr2, uint8_t addr1, uint8_t data);

void flashWriteArray(uint8_t addr3, uint8_t addr2, uint8_t addr1, uint8_t data[], int dataSize);

#endif /* INC_FLASH_H_ */
