/*
 * MainMemory.h
 *
 * Created: 04.04.2013 18:07:06
 *  Author: JochenAlt
 */ 


#ifndef MAINMEMORY_H_
#define MAINMEMORY_H_

#include "MemoryBase.h"
#include "Paul.h"


class MainMemory : public MemoryBase {
	public:
		// initialize  default values of memory for the very first start
		MainMemory();

	union  {
		uint8_t volume;
		uint16_t speechRate;										
		LanguageType language;
		VoiceType voice;
	} persistentMem;

};


extern MainMemory memory;


#endif /* MAINMEMORY_H_ */