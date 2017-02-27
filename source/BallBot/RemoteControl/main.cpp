/*
 * MainController.cpp
 *
 * Created: 15.02.2013 12:54:26
 * Author: JochenAlt
 *
 * Main program of remote control
 *  - reads the joxstick
 *  - sends to-be speed/omega to bot board
 *  - updates the display
 */ 

#include "Arduino.h"
#include "HardwareSerial.h"
#include "setup.h"
#include "TimePassedBy.h"
#include "joystick.h"
#include "Display/DisplayController.h"

#include <avr/io.h>
#include "util/delay.h"
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "MsgType.h"
#include "CommLinkToPaul.h"
#include "RemoteMemory.h"
CommLinkToPaul linkToPaul;

// configuration of bot board
ControlConfigurationType ctrlConfig;

boolean proxyToBot = false;		// proxy to mirror bot
int16_t speedX = 0;				// set speed of bot
int16_t speedY = 0;
int16_t omega = 0;
boolean isBalancing = false;	// true, if bot is balancing
float tiltX;					// currrent tilt angle of bot
float tiltY;
int16_t posX;					// position of bot relative to starting point
int16_t posY;
float botVoltage;				// voltage of bot
float remoteVoltage;			// voltage of remote control
boolean commLink = false;		// true, if communication link with Paul is working
boolean beingMobbed = false;	// true, if Paul has been pushed
boolean productionMode = false; // if true, all input via UART is sent to the display's terminal, no other UART is seen
boolean powerButtonReleased = false; // true, if the power button has been released at least once

// various timers
TimePassedBy commTimer;			// timeout timer for communication with bot
TimePassedBy voltageTimer;		// timer how often to measure the voltage
TimePassedBy powerTimer;		// timer how often the power on switch is checked

float getCurrentVoltage() {
	int16_t adcValue= analogRead( BATT_PIN);
	float voltage = float(adcValue)* (2.56/1024.0 *(10.0+56.0)/10.0);	
	voltage = voltage * 0.99; // correction of resistors 
	remoteVoltage = voltage;
	return voltage;
}

void setErrorLamp(const __FlashStringHelper* pLine_P) {
	if (pLine_P != NULL) {
		if (!productionMode)
			Serial.println(pLine_P);
	}		
}
	
void loopLocalWithoutDisplay();

void setup() {
	
	// being stuck after 4s let the watchdog catch it
	wdt_enable(WDTO_4S);

	// in case anything during setup goes wrong, start with UART
	Serial.begin(REMOTECONTROL_BAUD_RATE);
	
	// first pull the relay down, by pulling POWER_RELAY_PIN to LOW
	pinMode(POWER_RELAY_PIN, OUTPUT);
	digitalWrite(POWER_RELAY_PIN, LOW); // relay stay on
	pinMode(POWER_SWITCH_PIN, INPUT);  // used check whether power button has been pressed while on
	powerButtonReleased = false;		// to switch remote off, power switch needs to be released at least once 
	
	// everyone likes a blinking LED
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN,HIGH); // switch on LED during initialization
		
	// use internal 2.56V reference, used for hall effect joystick only
	analogReference(INTERNAL);
	
	// initialize joystick
	joystick.setup();
	
	// read EEPROM
	if (memory.setup()) {
		joystick.calibrate();
	}
	
	// reset state of bot
	speedX = 0;				// set speed of bot in [mm/s] resp. [°/s]
	speedY = 0;
	omega = 0;
	
	isBalancing = false;	// true, if bot is balancing, is received from bot directly
	tiltX = 0;				// current tilt angle of bot, also received
	tiltY = 0;
	posX = 0;				// position of bot relative to starting point, received
	posY = 0;	
	commLink = false;		// comlink with Paul is off. Becomes false if 3 retries fail

	// Serial1 is used for connection to main board via XBEE
	Serial1.begin(XBEE_BAUD_RATE);
	
	// setup is done, turn lights down
	digitalWrite(LED_PIN,LOW);
		
	// setup communication to bot board
	linkToPaul.setup(&loopLocalWithoutDisplay);

	// parameters of linear state regulator needs to be initialized 
	ctrlConfig.initDefaultValues();
	
    // initialize display (dont try to initialize that earlier)
	delay(300);
	display.setup(&linkToPaul, &loopLocalWithoutDisplay);
	
	// done, switch off the lights
	digitalWrite(LED_PIN,LOW);
	
	// setup line to pc
	Serial.println("Paul's Remote - h for help");	
}

void printMainMenuHelp() {
	Serial.println(F("remote main menu"));
	Serial.println();
	Serial.println(F("d - configure bot controller"));
	Serial.println(F("i - IMU calibration"));
	Serial.println(F("t - proxy to bot"));
	Serial.println(F("j - calibrate joystick"));
	Serial.println(F("p - production mode toggle"));
	

	Serial.println();
	Serial.println(F("Q/W E/R     +/- angular Weight, angular speed weight"));
	Serial.println(F("A/S D/F G/H +/- ball position, velocity, accel"));
	Serial.println(F("Y/X C/V B/N +/- head position, velocity, accel"));
	Serial.println(F("M;          +/- omega Weight"));
	Serial.println();
	Serial.println(F("h - this page"));
	Serial.println();
}

void printMovement() {
	Serial.print(F("v=("));
	Serial.print(speedX);
	Serial.print(F(","));
	Serial.print(speedY);
	Serial.print(F(","));
	Serial.print(omega);
	Serial.print(F(") a=("));
}

void loopLocalWithoutDisplay() {
	// read joystick values and set speed	
	joystick.loop(speedX,speedY, omega);

	// voltage measurement
	if (voltageTimer.isDue_ms(1000)) { 	
		// check voltage of remote
		float voltage = getCurrentVoltage();
		if ((voltage < LOW_BAT_3S) &&  (voltage > 8.2)) { // if it's below 8.2 V we are on the fixed power supply on jochen's desk
			Serial.print(F("Paul's remote has "));
			Serial.print(voltage,2,2);
			Serial.println("V");
		}				

		// check voltage of bot
		if (commLink && (botVoltage < LOW_BAT_2S)) {
			Serial.print(F("Paul has low bat "));
			Serial.print(botVoltage,2,2);
			Serial.println("V");
		}		
		display.setVoltage(botVoltage,remoteVoltage);
		
		// check whether we have to power down the remote after 5 minutes of inactivity
		if (!commLink) {
			if (!display.userActivityHappened(5*60)) {
				display.powerOff();
				digitalWrite(POWER_RELAY_PIN, HIGH); // release relay, it will switch off the remote
			}			
		}
		
		
	}	

	if (powerTimer.isDue_ms(200)) {
		// check whether the power switch has been released
		// (required for shutting remote down)
		if (!powerButtonReleased && (digitalRead(POWER_SWITCH_PIN) == LOW)){
			powerButtonReleased = true;
		}
		
		// if the power button has been released and pushed again, remote is switched off
		if (powerButtonReleased && (digitalRead(POWER_SWITCH_PIN) == HIGH)) {
			display.powerOff();
			// ok, let me read the text
			delay(2000);
			digitalWrite(POWER_RELAY_PIN, HIGH); // release relay, it will switch off the remote
		}
	
		// let the LED blink with 2,5 Hz if communication link is on, otherwise with 1Hz
		if (commLink) {
			(digitalRead(LED_PIN) == LOW)?digitalWrite(LED_PIN,HIGH):digitalWrite(LED_PIN,LOW);	
		} else {
			static uint8_t i = 0;
			i = (i+1) % 4;
			if (i == 0) {
				(digitalRead(LED_PIN) == LOW)?digitalWrite(LED_PIN,HIGH):digitalWrite(LED_PIN,LOW);	
			}				
		}					
	}	
	// check if something has to be saved in EEPROM
	memory.loop();
}

// this called in spare time (i.e. very often)
void loopLocalComponents() {

	loopLocalWithoutDisplay();
	
	// do any updates in the display
	display.loop();
	
}


// call paul and tell him the speed
void loopCallToPaul() {
	if (commTimer.isDue_ms(1000/REMOTECONTROL_FREQUENCY)) {
		char textPiece[SET_SPEED_TEXT_SIZE];
		linkToPaul.callSetSpeed(speedX,speedY,omega,botVoltage,isBalancing,tiltX,tiltY,posX,posY,beingMobbed,textPiece,commLink);
		display.linkToPaul(commLink);
			
		if (commLink) {
			/*
				Serial.print("botV=");
				Serial.print(botVoltage,2,2);
				Serial.print("V");
				Serial.print(" balance=");
				Serial.print(isBalancing?"on":"off");
				Serial.print(" tilt=(");
				Serial.print(tiltX,3,2);
				Serial.print(",");
				Serial.print(tiltY,3,2);
				Serial.print(")");
				Serial.print(" pos=(");
				Serial.print(posX,9);
				Serial.print(",");
				Serial.print(posY);
				Serial.print(") mobbed=");
				Serial.print(beingMobbed);
				Serial.println();
				*/

				display.addToSpokenText(textPiece);
				display.setKinematic(isBalancing, posX,posY,tiltX,tiltY,omega,speedX,speedY, beingMobbed);
		}
	}		
}

void loop() {
	while (true) {
		wdt_reset();
		if (Serial.available()) {
			char inputChar = Serial.read();
			
			if (productionMode) {
				if ((display.getState() != ViewSpeechTalkNo) && !display.viewProxyToPaul()) {
					display.activateView(ViewSpeechTalkNo);
				}
				display.setKey(inputChar);				
			} else {
				// in two views, we redirect all keys directly to Paul via Serial1
				if ((display.getState() == ViewSpeechTalkNo) || display.viewProxyToPaul())
				{					
					display.setKey(inputChar);
					if (display.viewProxyToPaul())
						Serial1.print(inputChar);
				} else {
				if (proxyToBot) {
					switch (inputChar) {
						case 't':
							Serial.println(F("switch proxy off"));
							proxyToBot= false;
							break;
						default:
							// send everything from bot to uart
							// (but that single character switching proxy mode off)
							Serial1.print(inputChar);  // and send to bot 
							break;
					} // switch inputChar					
				} else { // if !proxyOn		
					switch (inputChar) {
						case 'h':
							printMainMenuHelp();
							ctrlConfig.print();
							break;
						case 'j':
							joystick.calibrate();
							break;
						case 'i': {
							int16_t offsetX,offsetY,offsetZ;
							linkToPaul.callCalibrateIMU(offsetX,offsetY,offsetZ);	
							speedX = 0;
							speedY = 0;
							omega = 0;
							Serial.print(F("new offset = ("));
							Serial.print(offsetX);
							Serial.print(F(","));
							Serial.print(offsetY);
							Serial.print(F(","));
							Serial.print(offsetZ);
							Serial.println(F(")"));
							}					
							break;
						case 't':
							Serial.println(F("proxy on"));
							Serial.print(F("bot>"));

							proxyToBot = true;
							break;
						case 'p':
							if (productionMode) {
								Serial.println(F("production mode off"));
								productionMode = false;
							}
							else {
								productionMode = true;
								Serial.println(F("production mode on"));
							}							
							break;
						case 'd':
							linkToPaul.callControlConfiguration(ctrlConfig);
							ctrlConfig.print();
							break;
						case 'Q':
							ctrlConfig.angleWeight_fp8 += FLOAT2FP16(0.5,8);
							ctrlConfig.print();
							break;
						case 'W':
							ctrlConfig.angleWeight_fp8 -= FLOAT2FP16(0.5,8);;
							ctrlConfig.print();
							break;
						case 'E':
							ctrlConfig.angularSpeedWeight_fp8+= FLOAT2FP16(0.5,8);;
							ctrlConfig.print();
							break;
						case 'R':
							ctrlConfig.angularSpeedWeight_fp8-= FLOAT2FP16(0.5,8);;
							ctrlConfig.print();
							break;
						case 'D':
							ctrlConfig.velocityWeight_fp10+= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'F':
							ctrlConfig.velocityWeight_fp10-= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'A':
							ctrlConfig.positionWeight_fp10+= FLOAT2FP16(0.1,10);;
							ctrlConfig.print();
							break;
						case 'S':
							ctrlConfig.positionWeight_fp10-= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'G':
							ctrlConfig.accelWeight_fp7+= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						case 'H':
							ctrlConfig.accelWeight_fp7-= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						case 'C':
							ctrlConfig.bodyVelocityWeight_fp10+= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'V':
							ctrlConfig.bodyVelocityWeight_fp10-= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'Y':
							ctrlConfig.bodyPositionWeight_fp10+= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'X':
							ctrlConfig.bodyPositionWeight_fp10-= FLOAT2FP16(0.1,10);
							ctrlConfig.print();
							break;
						case 'B':
							ctrlConfig.bodyAccelWeight_fp7+= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						case 'N':
							ctrlConfig.bodyAccelWeight_fp7-= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						case 'M':
							ctrlConfig.omegaWeight_fp7+= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						case ';':
							ctrlConfig.omegaWeight_fp7-= FLOAT2FP16(0.1,7);
							ctrlConfig.print();
							break;
						default:
							Serial.print(F("RC:inv cmd: "));
							Serial.print((int)inputChar);
							Serial.println();
							break;
					} // switch 			
				} // if !proxyOn
				} // if !ViewSpeechText	mode					 
			} // if !productionMode			
		} // if Serial.available		

		// display everything from bot
		while (Serial1.available()) {
			char inputChar = Serial1.read();
			
			// if in production mode, catch everything and swallow it
			if (!productionMode) {
				if (((inputChar <32) || (inputChar>127)) && ((inputChar !=10) && (inputChar !=13))) {
					Serial.print('\\');			
					Serial.print((int)inputChar);
					Serial.print(' ');		
				} else {				
					Serial.print(inputChar);
				}				

				// send to terminal widget if that dialogue is open
				display.sendToTerminal(inputChar);	
				if (inputChar == char(10))
					Serial.print(F("Bot>"));			
				
			}
		}

		// call paul and tell him new values
		loopCallToPaul();
		
		// quick call to all components to give time to update
		loopLocalComponents();
	}		
	
}
