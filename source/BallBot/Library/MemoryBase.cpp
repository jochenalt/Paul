/*
 * Memory.cpp
 *
 * Created: 04.04.2013 15:41:22
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "MemoryBase.h"
#include <avr/eeprom.h>
#include "MsgType.h"


#define EEMEM_MAGICNUMBER 1565 // thats my birthday, used to check if eprom has been initialized
uint16_t magicMemoryNumber_EE EEMEM;

MemoryBase::MemoryBase (char *pMemEE, char *pMem_RAM, uint8_t pLen) {
	somethingToSave = false;
	memEE = pMemEE;
	memRAM = pMem_RAM;
	len = pLen;
}

boolean MemoryBase::setup() {
	if (!isEEPROMInitialized()) {

		// hopefully the defaults have been initialized in the constructor of the derived
		save();
		
		// write magic number in the eeprom to indicate initialization
		markEEPROMInitialized();
		return true;
		
	} else
		read();
	return false;		
}

void MemoryBase::read() {
	eeprom_read_block(memRAM, memEE, len);
}

void MemoryBase::loop() {
	if (somethingToSave) {
		if (memTimer.isDue_ms(writeDelay)) {
			save();
			somethingToSave = false;						
		}
	}
}

void MemoryBase::delayedSave(uint16_t pDelayMS) {
	somethingToSave = true;
	writeDelay = pDelayMS;
	memTimer.setDueTime(0);
}

void MemoryBase::save() {
	eeprom_write_block(memRAM, memEE,len);
	
	somethingToSave = false;
}

boolean MemoryBase::isEEPROMInitialized() {
	return (eeprom_read_word(&magicMemoryNumber_EE) == EEMEM_MAGICNUMBER);
}		

void  MemoryBase::markEEPROMInitialized() {
	eeprom_write_word(&magicMemoryNumber_EE, EEMEM_MAGICNUMBER);
}		