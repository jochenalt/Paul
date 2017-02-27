/*
 * MainMemory.cpp
 *
 * Created: 04.04.2013 18:06:40
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "MemoryBase.h"
#include <avr/eeprom.h>
#include "MsgType.h"
#include "MotorController.h"
#include "imu/IMUController.h"
#include "BallBotMemory.h"

char ballBotMemory_EE[sizeof(BallBotMemory::persistentMem)] EEMEM;

BallBotMemory memory;
BallBotMemory::BallBotMemory() 
	: MemoryBase(ballBotMemory_EE,(char*)&(persistentMem),sizeof(BallBotMemory::persistentMem)) {
		
	// initialization for the very first start, when EEPROM is not yet initialized

	persistentMem.ctrlConfig.initDefaultValues();
	persistentMem.motorPICtrlConfig.initDefaultValues();
	persistentMem.imuControllerConfig.initDefaultValues();

}
