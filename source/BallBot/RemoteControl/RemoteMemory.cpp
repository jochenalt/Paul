/*
 * MainMemory.cpp
 *
 * Created: 04.04.2013 18:06:40
 *  Author: JochenAlt
 */ 


#include "RemoteMemory.h"
char remoteMemory_EE[sizeof(RemoteMemory::persistentMem)] EEMEM;

RemoteMemory memory;
RemoteMemory::RemoteMemory() 
	: MemoryBase(remoteMemory_EE,(char*)&(persistentMem),sizeof(RemoteMemory::persistentMem)) {
		
	// initialization for the very first start, when EEPROM is not yet initialized
	persistentMem.displayBrightness = 100;
	persistentMem.displayTheme = 0;
	persistentMem.language = English;
	persistentMem.voice = PerfectPaul;
	persistentMem.volume = INITIAL_VOLUME;				// default volume of EMIC-2 is (256*48)/64, that's loud
	persistentMem.speechRate = INITIAL_SPEECH_RATE;		// 100 is quite slow
	persistentMem.joystickOffsetX = 0;
	persistentMem.joystickOffsetY = 0;
	persistentMem.joystickOffsetZ = 0;
	
}
