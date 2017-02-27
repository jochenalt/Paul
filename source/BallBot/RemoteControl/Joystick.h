/*
 * Joystick.h
 *
 * Created: 18.03.2013 15:52:34
 *  Author: JochenAlt
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "Arduino.h"
#include "setup.h"
#include "TimePassedBy.h"
#include "RemoteMemory.h"

#define X_MAX_P 404
#define X_MAX_N 455
#define Y_MAX_P 399
#define Y_MAX_N 439
#define Z_MAX_P 401
#define Z_MAX_N 454

class Joystick {
	public:
		// initialize joystick with calibration values from memory.persistentMem.
		void setup();
		void calibrate();
			// read joystick values and sets speed
		 void loop(int16_t &pSpeedX,int16_t& pSpeedY, int16_t &omega);
	private:
		void readValues(int16_t &pX, int16_t &pY, int16_t &pZ, boolean &pButtonPressed);	
		bool getJoystickButton();
		bool getJoystickButtonPressed();
		TimePassedBy JoyStickButtonBouncingTimer;
		bool joystickButtonValue;
		TimePassedBy joystickLoopTimer;		
};

extern Joystick joystick;

#endif /* JOYSTICK_H_ */