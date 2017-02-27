/*
 * MainMemory.cpp
 *
 * Created: 04.04.2013 18:06:40
 *  Author: JochenAlt
 */ 


#include "MainMemory.h"
#include <avr/eeprom.h>

char mainMemory_EE[sizeof(MainMemory::persistentMem)] EEMEM;
MainMemory memory;

MainMemory::MainMemory()
: MemoryBase(mainMemory_EE,(char*)&(persistentMem),sizeof(MainMemory::persistentMem)) {
	// initialization for the very first start, when EEPROM is not yet initialized
	persistentMem.volume = INITIAL_VOLUME;
	persistentMem.speechRate = INITIAL_SPEECH_RATE;
}

	