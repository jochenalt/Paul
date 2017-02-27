/*
 * Joystick.cpp
 *
 * Created: 18.03.2013 15:55:16
 *  Author: JochenAlt
 */ 

#include "Joystick.h"


Joystick joystick;

void Joystick::setup()  {
	// button with pull up
	pinMode(JOYSTICK_BUTTON_PIN,INPUT);
	digitalWrite(JOYSTICK_BUTTON_PIN, HIGH); 

	pinMode(JOYSTICK_X_PIN, INPUT);
	pinMode(JOYSTICK_Y_PIN,INPUT);
	pinMode(JOYSTICK_Z_PIN,INPUT);
			
	joystickButtonValue = false;
}

void Joystick::loop(int16_t &pSpeedX,int16_t& pSpeedY, int16_t &pOmega) {
	if (joystickLoopTimer.isDue_ms(100)) {
		// read joystick values
		int16_t joystickX;
		int16_t joystickY;
		int16_t joystickZ;
		boolean joystickButton;
		joystick.readValues(joystickX,joystickY, joystickZ, joystickButton);
		/*
		Serial.print("j=(");
		Serial.print(joystickX);
		Serial.print(",");
		Serial.print(joystickY);
		Serial.print(",");
		Serial.print(joystickZ);
		Serial.println(")");
		*/
		
		// set speed values, joystick goes from -128 to +128, speed is the same
		pSpeedX = (-joystickX);
		pSpeedY = joystickY;
		pOmega = -joystickZ; // omega has a different orientation than the joystick
	}
}


void Joystick::calibrate() {
	// calibrate to null values
	int16_t xSum = 0,ySum = 0,zSum = 0,x,y,z;
	memory.persistentMem.joystickOffsetX = 0;
	memory.persistentMem.joystickOffsetZ = 0;
	memory.persistentMem.joystickOffsetY = 0;

	boolean button;
	readValues(x,y,z, button); // throw away first measurement, load any capacities
	delay(100);	  // wait for the magneto field to build up
	int i = 0;
	for (;i<5;i++) {
		xSum += analogRead(JOYSTICK_X_PIN);
		ySum += analogRead(JOYSTICK_Y_PIN);
		zSum += analogRead(JOYSTICK_Z_PIN);
	}			
	memory.persistentMem.joystickOffsetX = xSum/i;
	memory.persistentMem.joystickOffsetY = ySum/i;
	memory.persistentMem.joystickOffsetZ = zSum/i;
}
		
void Joystick::readValues(int16_t &pX, int16_t &pY, int16_t &pZ, boolean &pButtonPressed) {
	int32_t x = int32_t(analogRead(JOYSTICK_X_PIN) - memory.persistentMem.joystickOffsetX);
	int32_t y = int32_t(analogRead(JOYSTICK_Y_PIN) - memory.persistentMem.joystickOffsetY);
	int32_t z = int32_t(analogRead(JOYSTICK_Z_PIN) - memory.persistentMem.joystickOffsetZ);
	/*
	Serial.print("jraw=(");
	Serial.print(x);
	Serial.print(",");
	Serial.print(y);
	Serial.print(",");
	Serial.print(z);
	Serial.println(")");
	*/
	pX = (x>0)?x*128/X_MAX_P:x*128/X_MAX_N;
	pY = (y>0)?y*128/Y_MAX_P:y*128/Y_MAX_N;
	pZ = (z>0)?z*128/Z_MAX_P:z*128/Z_MAX_N;
	pX = constrain(pX,-128,128);
	pY = constrain(pY,-128,128);
	pZ = constrain(pZ,-128,128);
	pButtonPressed = getJoystickButtonPressed();
}
	


bool Joystick::getJoystickButton() {
	if (JoyStickButtonBouncingTimer.isDue_ms(50)) {
		static uint8_t oldPinValue = HIGH;
		uint8_t pinValue = digitalRead(JOYSTICK_BUTTON_PIN);
		if ((pinValue == oldPinValue)) {
			joystickButtonValue = (pinValue==0)?true:false;
		}
		oldPinValue = joystickButtonValue;
	}
	return joystickButtonValue;
}

bool Joystick::getJoystickButtonPressed() {
	static bool buttonSwitchToggle = false;
	if (getJoystickButton() && (buttonSwitchToggle == false)) {
		buttonSwitchToggle = true;
		return true;
	}	
	if (!getJoystickButton() && (buttonSwitchToggle == true)) {
		buttonSwitchToggle = false;
	}		
	return false;
}
