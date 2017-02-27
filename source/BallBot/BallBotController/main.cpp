/*
 * main.cpp
 *
 * Created: 16.11.2012 23:11:45
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "HardwareSerial.h"
#include "setup.h"
 
#include "Kinematics.h"
#include "IMU/IMUController.h"
#include "FixedPoint.h"
#include "BotController.h"
#include "Motor.h"
#include "MotorController.h"
#include <avr/io.h>
#include "util/delay.h"
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "TimePassedBy.h"
#include "MsgType.h"
#include "LinkMainBot.h"
#include "BallBotMemory.h"
void printMainMenuHelp();
void printBotControlMenu();
boolean botControlMenu();

static uint8_t BitIdels[4]		 = { 0b10000000,0b00000000,0b10000000,0b00000000,}; // boring
static uint8_t BotIsBalancing[3] = { 0b11001000,0b00001100,0b10000000};				// nice!

PatternBlinker LedBlinker;
enum BlinkPatternType {IDLE_MODE,BALANCE_MODE, OFF_MODE};
BlinkPatternType blinkPatternStack[4] = {OFF_MODE,OFF_MODE,OFF_MODE,OFF_MODE};
uint8_t blinkPatternStackPtr = 0;

uint32_t baudRate; 
	
CommMainAndBot comm;


void setErrorLamp(const __FlashStringHelper* pLine_P) {
	if (pLine_P != NULL)
		Serial.print(pLine_P);
}

void resetMainController() {
	baudRate = INITIAL_BAUD_RATE;
}

void setBlinkPattern(BlinkPatternType pPattern) {

	if (pPattern == IDLE_MODE) {
		LedBlinker.set(LED_PIN_BLUE,BitIdels,sizeof(BitIdels));
	}		
	else if (pPattern == BALANCE_MODE) {
		LedBlinker.set(LED_PIN_BLUE,BotIsBalancing,sizeof(BotIsBalancing));
	}	
	else { 
		LedBlinker.off();
	}					
}

void pushBlinkPattern (BlinkPatternType pPattern = OFF_MODE) {
	setBlinkPattern(pPattern);	
	blinkPatternStack[blinkPatternStackPtr] = pPattern;
	blinkPatternStackPtr++;

};
void popBlinkPattern() {
	blinkPatternStackPtr--;
	if (blinkPatternStackPtr>0)
		setBlinkPattern(blinkPatternStack[blinkPatternStackPtr-1]);	
}


// reset uC
void resetMicroController() {
	Serial.println(F("resetMicroController"));
	delay_ms(20);

	// restart by watch dog immediately
	wdt_enable(WDTO_15MS);
	delay_ms(20);
}	
		
// returns the number of milliseconds with an error of below 1%
uint32_t milliseconds() {
	unsigned long m = micros();
	uint32_t n = m>>6;
	return (m + n + (n>> 1) + (n >>7)) >> (10+TIMER_SHIFT); // that is almost as accurate as the division by 1000
}

uint32_t microseconds() {
	return micros() >> TIMER_SHIFT;
}

void delay_ms(uint16_t pMS) {
	delay(pMS << TIMER_SHIFT);
}


void changeLED() {
	static bool on = false;
	if (on)
		digitalWrite(LED_PIN_BLUE,LOW);
	else
		digitalWrite(LED_PIN_BLUE,HIGH);
	on = !on;
}


void resetCalibrationData()  {				
	imu.initData();
	botCtrl.reset();
}

void printCalibrationData() {				
	mCtrl.printCalibrationData();
	imu.printCalibrationData();
	botCtrl.printCalibrationData();
}


bool balancingModeOn = false;		 // to start and stop the balancing mode
bool balancingHelpModeOn = false;	 // print some data on the state during balancing

void setBalancingMode(bool pOn) {
	balancingModeOn = pOn;
	if (balancingModeOn) {
		Motor::init();
		imu.initFilterData();
		botCtrl.init();
	}		
	else {
		mCtrl.stop();
	}
}

void setup() {
	// watch dog resets after 4s, this is for setup only which should take less than 2s
	wdt_enable(WDTO_1S);

	// everyone likes a blinking LED
	pinMode(LED_PIN_BLUE, OUTPUT);
		
	// switch on IMU
	// if it has been reset in resetIMU(), this is at least 65ms ago (enough to reset te IMU)
	pinMode(IMU_RESET_PIN,OUTPUT);
	digitalWrite(IMU_RESET_PIN,LOW); 

	// start setup
	Serial.begin(INITIAL_BAUD_RATE);

	// read EEPROM Memory
	memory.setup();
	// start setup
	changeLED();
	Motor::setup();
	changeLED();
	MotorController::setup();
	kin.setupKinematics();
	changeLED();
	imu.setup();
	
	// setup communication with main board
	comm.setup();	

	// watch dog resets after 250s for normal operations
	wdt_enable(WDTO_250MS);

	// start the nice pattern blinker
	pushBlinkPattern(IDLE_MODE);
}


void printMainMenuHelp() {
	Serial.println(F("Bot Board"));
	Serial.println(F("s - save to EEPROM"));
	Serial.println(F("R - reset calibration data"));
	Serial.println(F("r - read from EEPROM"));
	Serial.println(F("k - test kinematics"));
	Serial.println(F("w - Motor controller menu"));
	Serial.println(F("x - Kinematics menu"));
	Serial.println(F("i - IMU controller menu"));
	Serial.println(F("b - start bot control"));
	Serial.println(F("I - reset IMU"));
	Serial.println(F("h - this page"));
	Serial.println();
	
	printCalibrationData();
}

void loop() {
	while (true) {
		wdt_reset();	
		
		LedBlinker.loop();
		botCtrl.loop(balancingModeOn);
		memory.loop();
		
	if (Serial.available()) {
		char inputChar = Serial.read();

		Datagram request;
		bool quit = false;
		bool botCtrlCmd = false;
		
		if (comm.receive(inputChar, 10000,request)) {
			switch (request.getMessageType()) {
				case CALIBRATE_IMU_REQ: {
						// no content in the request
						imu.calibrate();
						int16_t offsetX, offsetY, offsetZ;
						imu.getOffset(offsetX, offsetY, offsetZ);
						Datagram imuReponse;
						comm.sendCalibrateIMUResponse(offsetX, offsetY, offsetZ, imuReponse);
					}						
					break;
				case CONFIGURE_REQ: {
						bool set;
						ControlConfigurationType cc;
						comm.parseConfigureRequest(request,set, cc);
						if (set) {
							memory.persistentMem.ctrlConfig.angleWeight_fp8 = cc.angleWeight_fp8;
							memory.persistentMem.ctrlConfig.angularSpeedWeight_fp8 = cc.angularSpeedWeight_fp8;

							memory.persistentMem.ctrlConfig.velocityWeight_fp10 = cc.velocityWeight_fp10;
							memory.persistentMem.ctrlConfig.positionWeight_fp10 = cc.positionWeight_fp10;
							memory.persistentMem.ctrlConfig.accelWeight_fp7 = cc.accelWeight_fp7;

							memory.persistentMem.ctrlConfig.bodyVelocityWeight_fp10 = cc.bodyVelocityWeight_fp10;
							memory.persistentMem.ctrlConfig.bodyPositionWeight_fp10 = cc.bodyPositionWeight_fp10;
							memory.persistentMem.ctrlConfig.bodyAccelWeight_fp7 = cc.bodyAccelWeight_fp7;
							memory.persistentMem.ctrlConfig.omegaWeight_fp7= cc.omegaWeight_fp7;
						}
						
						Datagram configureReponse;
						comm.sendConfigureResponse(configureReponse,memory.persistentMem.ctrlConfig);
					}					
					break;

				case SET_SPEED_REQ: {
						int16_t speedX, speedY, omega;
						bool balancingMode;
						comm.parseSetSpeedRequest(request,balancingMode, speedX,speedY,omega);
	
						Datagram setSpeedResponse;
						int16_t angleX_fp9, angleY_fp9, posX, posY, accelX, accelY;
						botCtrl.getFilteredAngle(angleX_fp9, angleY_fp9);
						botCtrl.getAbsolutePosition(posX, posY);
						botCtrl.getFilteredAccel(accelX, accelY);
						
						comm.sendSetSpeedResponse(angleX_fp9,angleY_fp9,posX, posY, accelX, accelY,setSpeedResponse);
						if (balancingMode != balancingModeOn)
							setBalancingMode(balancingMode);
						botCtrl.setSpeed(speedX<<3,speedY<<3,omega<<3);

					}					
					break;
				default:
					Serial.print(F("bot:invalid cmd"));
					Serial.println(request.getMessageType());
			}					
		} 
		else 
		{   // if inputChar was not parsed by communication method
			if (balancingModeOn) {
				botCtrlCmd = botCtrl.dispatchCommand(inputChar, quit);
				if (quit) {
					balancingModeOn = false;
					pushBlinkPattern(IDLE_MODE);
				}				
			}			
		}
				
		if (!botCtrlCmd) {
			// no botController command, try something else
			if (!balancingModeOn) {
					// no, check interactive commands
					switch (inputChar) {
						case 'h':
							printMainMenuHelp();
							break;
						case 'i':
							imu.menuController();
							printMainMenuHelp();
							break;
						case 'I':
							imu.fullReset();
							break;
						case 'b':
							Serial.print(F("balancing mode "));
							if (balancingModeOn) {
								Serial.println(F("off"));
								setBalancingMode(false);
							}								
							else {
								Serial.println(F("on"));
								pushBlinkPattern(BALANCE_MODE);
								setBalancingMode(true);
							}
												
							break;
						case 'x':
							kin.menu();
							printMainMenuHelp();
							break;
						case 'w':
							mCtrl.menu();
							printMainMenuHelp();
							break;
						case 'k':
							kin.testPerformanceKinematics();
							kin.testKinematics();
							kin.testInverseKinematics();
							break;
						case 's':
							memory.save();
							Serial.println(F("calibration saved"));
							break;
						case 'R':
							resetCalibrationData();
							printCalibrationData();
							break;
						case 'r':
							memory.read();
							Serial.println(F("calibration read"));
							break;
						default:
						break;
					}							
			}				
		}		
	}
	}	
}


