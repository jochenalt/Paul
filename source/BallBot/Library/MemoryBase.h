/*
 * Memory.h
 *
 * Created: 04.04.2013 15:40:14
 * Base class to store data in EEPROM
 * o initialized with an EEPROM char array and a RAM char array representing 
 * o able to do a delay a save operations
 *  Author: JochenAlt
 */ 


#ifndef MEMORY_H_
#define MEMORY_H_

#include "Arduino.h"
#include "EEPROM.h"
#include "TimePassedBy.h"

class MemoryBase {
	protected:
		// initialize with memory blocks in RAM and EEPROM
		MemoryBase (char *pMem_EE, char *pMem_RAM, uint8_t pMemSize);
	public:
		
		// initialized the EEPROM with the data that is in RAM 
		// returns true, if this is the first call 
		boolean setup();
		// save operation in pDelayMS ms. Gives the chance to call this whenever 
		// a change happens, since the update is done only once after a while
		void delayedSave(uint16_t pDelayMS);
		// read data from EEPROM
		void read();
		// save immediately in EEPROM
		void save();
		// need to be called regularly to invoke the delayed write operation
		void loop();
	private:
		boolean isEEPROMInitialized();
		void  markEEPROMInitialized();
		TimePassedBy memTimer;
		uint16_t writeDelay;
		boolean somethingToSave;
		char* memEE;
		char* memRAM;
		uint8_t len;
};

#endif /* MEMORY_H_ */