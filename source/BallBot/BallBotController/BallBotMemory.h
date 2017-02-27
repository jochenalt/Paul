/*
 * MainMemory.h
 *
 * Created: 04.04.2013 18:07:06
 *  Author: JochenAlt
 */ 


#ifndef BALLBOT_MEMORY_H_
#define BALLBOT_MEMORY_H_

#include "MemoryBase.h"

class BallBotMemory : public MemoryBase {
	public:
	
		BallBotMemory ();

		struct {
			ControlConfigurationType ctrlConfig;
			MotorPIController motorPICtrlConfig;
			IMUControllerConfig imuControllerConfig;
		} persistentMem;
};


extern BallBotMemory memory;


#endif /* BALLBOT_MEMORY_H_ */