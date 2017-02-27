/*
 * KeyboardController.cpp
 *
 * Created: 28.03.2013 17:23:07
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "KeyboardController.h"
#include <avr/wdt.h>
#include "setup.h"

#include <avr/pgmspace.h>

extern void keyboardSetup();
extern void keyboardLoop();
extern uint8_t keyboardGetKey();

KeyboardController keyboard;

KeyboardController::KeyboardController() {
}	

int16_t KeyboardController::getKey() {
	return keyboardGetKey();
}
void KeyboardController::setup()
{
	keyboardSetup();
}

void KeyboardController::loop()
{
	keyboardLoop();
}

