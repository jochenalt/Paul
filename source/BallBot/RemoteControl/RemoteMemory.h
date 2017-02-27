/*
 * MainMemory.h
 *
 * Created: 04.04.2013 18:07:06
 *  Author: JochenAlt
 */ 


#ifndef REMOTEMEMORY_H_
#define REMOTEMEMORY_H_

#include "MemoryBase.h"
#include <avr/eeprom.h>
#include "Paul.h"

class RemoteMemory : public MemoryBase {
	public:
	
		RemoteMemory ();

		struct {
			uint8_t displayBrightness;
			uint8_t displayTheme;		
			uint8_t volume;
			uint16_t speechRate;
			VoiceType voice;
			LanguageType language;								
			int16_t joystickOffsetX;
			int16_t joystickOffsetY;
			int16_t joystickOffsetZ;
		} persistentMem;
};


extern RemoteMemory memory;


#endif /* REMOTEMEMORY_H_ */