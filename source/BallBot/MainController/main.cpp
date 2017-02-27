/*
 * MainController.cpp
 *
 * Created: 15.02.2013 12:54:26
 * Author: JochenAlt
 *
 * Main program of main controller board, that 
 *  - possibility to proxy all inputs/outpus from BotControllerBoard
 *  - controls BotControllerBoard via UART0
 *  - control of voice modul via Software UART
 *  - control of LED modul via I2C
 */ 

#include "Arduino.h"
#include "HardwareSerial.h"
#include "setup.h"
#include "LEDController.h"
#include "SpeechController.h"
#include "MoodController.h"
#include "FixedPoint.h"

#include <avr/io.h>
#include "util/delay.h"
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "LinkRemoteMain.h"
#include "MainMemory.h"
#include "LinkToBotBoard.h"

boolean proxyOn = false;				// true of main controller works in interactive proxy mode to bot controller board
boolean botBalanceMode = false;		// true, if Paul is in balancing mode

CommLinkMainAndBot linkToBotBoard;	// communication link to bot board
CommRemoteAndMain linkToRc;			// communication link between remote and main board

// movement as being sent and received from bot controller
int16_t speedX = 0; // cm/s
int16_t speedY = 0; // cm/s
int16_t omega = 0;  // °/s
int16_t tiltX_fp9 = 0; 
int16_t tiltY_fp9 = 0;
int16_t posX = 0;
int16_t posY = 0;
int16_t accelX = 0;	// acceleration of balance state regulator in x,y
int16_t accelY = 0;
boolean beingMobbed = false; // true, if bot is mobbed and it has not yet been sent to remote

#define TEXT_BUFFER_SIZE 128
char textBuffer[TEXT_BUFFER_SIZE]; // text buffer of what Paul is supposed to say

// that's the flags set by the heartbeat of the remote control
boolean remoteIsOn = false; 
TimePassedBy remoteOnTimer;

// configuration of bot board
ControlConfigurationType ctrlConfig;

float lastVoltage; // last measurement of voltage 

// real measurement, call not too often
float getCurrentVoltage() {
	int16_t adcValue= analogRead( BATT_PIN);
	float voltage = float(adcValue)* (2.56/1024.0 *(10.0+56.0)/10.0);	
	
	voltage = voltage * 0.98; // correction of resistors 
	lastVoltage = voltage;
	return voltage;
}

uint16_t getLastVoltage_fp8() {
	return FLOAT2FP16(lastVoltage,8);
}


void setErrorLamp(const __FlashStringHelper* pLine_P) {
	digitalWrite(ERROR_LED,HIGH);
	if (pLine_P != NULL)
		Serial.println(pLine_P);
}

TimePassedBy balanceSwitchBouncingTimer;
bool value = false;
bool getBalanceSwitch() {
	if (balanceSwitchBouncingTimer.isDue_ms(50)) {
		static uint8_t oldPinValue = HIGH;
		uint8_t pinValue = digitalRead(BALANCING_SWITCH_PIN);
		if ((pinValue == oldPinValue)) {
			value = (pinValue==0)?true:false;
		}
		oldPinValue = value;
	}
	return value;
}

bool getBalanceSwitchPressed() {
	static bool balanceSwitchToggle = false;
	if (getBalanceSwitch() && (balanceSwitchToggle == false)) {
		balanceSwitchToggle = true;
		return true;
	}	
	if (!getBalanceSwitch() && (balanceSwitchToggle == true)) {
		balanceSwitchToggle = false;
	}		
	return false;
}	


extern void loopDuringBotBoardCall();

void setup() {
	
	wdt_enable(WDTO_2S);
	
	// everyone likes a blinking LED
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN,HIGH);
	
	// use internal 2.56V reference
	analogReference(INTERNAL);
	// initialize randomseed by analog reads of all analog pins
	uint16_t randomseed = analogRead(PIN_A0) + analogRead(PIN_A1) + analogRead(PIN_A2) + analogRead(PIN_A3)
						+ analogRead(PIN_A4) + analogRead(PIN_A5) + analogRead(PIN_A6) + analogRead(PIN_A7);
	
	// randomseed() seems not to work, so do it by yourself
	for (int i = 0;i<randomseed % 31;i++) 
		random();
	
	// error, balancing and don't know LED 
	pinMode(ERROR_LED,OUTPUT);
	pinMode(IS_BALANCING_LED,OUTPUT);
	pinMode(DONT_KNOW_LED,OUTPUT);
	
	// switch for balancing mode incl. pull up
	pinMode(BALANCING_SWITCH_PIN,INPUT);
	digitalWrite(BALANCING_SWITCH_PIN, HIGH); 

	// all LEDS on during startup
	digitalWrite(DONT_KNOW_LED, HIGH); 
	digitalWrite(ERROR_LED,HIGH);
	digitalWrite(IS_BALANCING_LED,HIGH);
	
	// initialize the control configuration
	ctrlConfig.initDefaultValues();
	
	// setup Serial1 used for XBee/UART2USB
	Serial.begin(XBEE_BAUD_RATE);

	// initialize LED driver
	ledCtrl.setup();
	
	// initialize EEPROM, read all default values
	memory.setup();
	
	// clear to-be movement 
	speedX = 0;
	speedY = 0;
	omega = 0;
	
	// start with no balancing mode ( do nothing)
	botBalanceMode = false;
	proxyOn = false;
	
	// in 3s from now Paul will say something
	moodCtrl.setMood(StartupMood);
	
	// setup is done
	digitalWrite(LED_PIN,LOW);
		

	// setup line to BotControllerBoard via Serial2
	Serial1.begin(BOT_CONTROLLER_BOARD_BAUD_RATE);

	// setup communication to remote 
	linkToRc.setup();

	// initialize EMIC-2 (late, since EMIC takes longtest to initialize)
	speechCtrl.setup();

	// setup communication to remote 
	linkToBotBoard.setup();
	
	// startup is done
	digitalWrite(ERROR_LED,LOW);
	digitalWrite(IS_BALANCING_LED,LOW);
	digitalWrite(DONT_KNOW_LED,LOW);
	
	// setup initial voltage
	getCurrentVoltage();

	// flush rc input
	delay(100);
	while (Serial.available()) {
		Serial.read();
		delay(1);
	}
}

void printMainMenuHelp() {
	Serial.print(F("main menu "));
	Serial.print(getCurrentVoltage(),1,2);
	Serial.println(F("V"));

	Serial.println();
	Serial.println(F("t - proxy to BotControllerBoard"));
	Serial.println(F("b - start balancing"));
	Serial.println(F("n - stop balancing"));
	Serial.println(F("r - reset balance board"));

	Serial.println(F("c - calibrate IMU"));
	Serial.println(F("d - configure bot board"));
	
	Serial.println(F("l - LED menu"));
	Serial.println(F("m - speak menu"));
	Serial.println(F("o - mood menu"));
	Serial.println(F("1 - poetry slam"));
	Serial.println(F("2 - sing"));
	Serial.println(F("3 - low batt"));
	Serial.println();
	Serial.println(F("q/w - +/- speed x"));
	Serial.println(F("a/s - +/- speed y"));
	Serial.println(F("y/x - +/- omega"));
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
	Serial.println();
}


void dispatchMenuInputChar(char pInputChar) {
	switch (pInputChar) {
		case 'h':
			printMainMenuHelp();
			ctrlConfig.print();
			break;
		case '1':
			moodCtrl.setMood(PoetrySlam);
			break;
		case '2':
			moodCtrl.setMood(SingingMood);
			break;
		case '3':
			moodCtrl.setMood(LowBattMood);
			break;
		case 'm':
			speechCtrl.printMenuHelp();
			speechCtrl.menu();
			printMainMenuHelp();
			break;
		case 'o':
			moodCtrl.printMenuHelp();
			moodCtrl.menu();
			printMainMenuHelp();
			break;
		case 'd': 
			linkToBotBoard.callControlConfiguration();
			Serial.print(F("configuration sent"));
			break;
		case 'b':
			botBalanceMode = true;
			break;
		case 'n':
			botBalanceMode = false;
			break;
		case 'c': 
			int16_t offsetX, offsetY, offsetZ;
			speedX = 0;speedY = 0;omega = 0;
			linkToBotBoard.callCalibrateIMU(offsetX, offsetY, offsetZ);
			Serial.print(F("new offset = ("));
			Serial.print(offsetX);
			Serial.print(F(","));
			Serial.print(offsetY);
			Serial.print(F(","));
			Serial.print(offsetZ);
			Serial.println(F(")"));
			break;
		case 'l':
			ledCtrl.printMenuHelp();
			ledCtrl.menu();
			printMainMenuHelp();
			break;
		case 'q':
			speedX += 1;
			printMovement();
			break;
		case 'w':
			speedX -= 1;
			printMovement();
			break;
		case 'a':
			speedY += 1;
			printMovement();
			break;
		case 's':
			speedY -= 1;
			printMovement();
			break;
		case 'y':
			omega += 1;
			printMovement();
			break;
		case 'x':
			omega -= 1;
			printMovement();
			break;
		case 't':
			Serial.println(F("switch proxy on"));
			Serial.print(F("Balance> "));			
			proxyOn = true; 
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
			Serial.print(F("main:inv cmd:"));
			Serial.print(pInputChar);Serial.print('=');
			Serial.print((int)pInputChar);
			Serial.println();
		} // switch 			
}


void soundWaveLoop() {
	// set current volume to sound wave if Paul is talking.
	if (speechCtrl.isTalking())
		ledCtrl.setSoundWaveAmplitude(speechCtrl.getCurrentAmplitude());
	else
		ledCtrl.setSoundWaveAmplitude(0);
		
	// loop the LED Controller to draw his beautiful patterns
	ledCtrl.loop();
}

// returns true, if call is succesful
boolean callSetSpeed() {
	boolean lCallOk= linkToBotBoard.callSetSpeed(botBalanceMode, speedX, speedY, omega,tiltX_fp9, tiltY_fp9, posX, posY, accelX, accelY, &loopDuringBotBoardCall);
	if (lCallOk) {
		moodCtrl.setTilt(tiltX_fp9>>4,tiltY_fp9>>4, posX, posY, accelX, accelY);
		ledCtrl.setTilt(moodCtrl.getTiltAngle(),moodCtrl.getTiltStrength());
		if (botBalanceMode && moodCtrl.isBeingMobbed() ) {
			beingMobbed = true; // true until it has been sent to remote, then it will be reset
			moodCtrl.setMood(MobbingMood);
		}									
	}; // error lamp is already set	by callsetSpeed if something went wrong
	return lCallOk;
}		

void loopDuringBotBoardCall() {
	soundWaveLoop();
}


void loop() {
	TimePassedBy voltageTimer;
	TimePassedBy errorOffTimer;
	TimePassedBy balanceLEDBlinker;
	TimePassedBy commTimer;
	while (true) {
		wdt_reset();

		// proxy to BotController board for all characters 
		// besides that one that switches proxy mode off
		if (proxyOn) {
			if (Serial.available()) {
				char inputChar = Serial.read();
				switch (inputChar) {
					case 't':
						Serial.println(F("switch proxy off"));
						proxyOn = false;
						break;
					default:
						// send everything from xbee to bot controller
						// (but that single character switching proxy mode off)
						Serial.println(inputChar);
						Serial1.print(inputChar);
						break;
				}					
			}

			// message from bot controller board? proxy 
			// it to xbee (empty the buffer)
			if (Serial1.available()) {
				/*
				Serial.println();
				Serial.print(F("Ball> "));			
				*/
				while (Serial1.available()) {
					char inputChar = Serial1.read();
					Serial.print(inputChar);
					/*
					if (inputChar == char(10))
						Serial.print(F("Ball> "));			
					 delay(1);
					*/
				}
			}				
		} else { // if !proxyOn		
			if (Serial.available()) {
				char inputChar = Serial.read();
				
				Datagram request;
				if (linkToRc.receive(inputChar, 50192UL,request)) {
					
					inputChar = 0; // we processed the first magic characters, dont try something else
					switch (request.getMessageType()) {
						case RC_CALIBRATE_IMU_REQ: {
							// no content in the request
							int16_t offsetX, offsetY, offsetZ;
							speedX = 0;speedY = 0;omega = 0;
							linkToBotBoard.callCalibrateIMU(offsetX,offsetY,offsetZ);
							speedX = 0;speedY = 0;omega = 0;

							Datagram imuReponse;
							linkToRc.sendRCCalibrateIMUResponse(offsetX, offsetY, offsetZ, imuReponse);
							}							
							break;
						case RC_SPEECH_REQ: {
								uint8_t len;
								VoiceType voiceNo;
								PoemType poemNo;
								SongType songNo;
								Datagram speechResponse;
								linkToRc.parseRCSpeechRequest(request, voiceNo, poemNo, songNo, len, textBuffer);
								
								if (songNo> 0) 
									speechCtrl.sing((SongType)songNo); 		
								else 
									if (poemNo > 0) 
										speechCtrl.poem((PoemType)poemNo); 
									else
										if (len > 0) 
											speechCtrl.say(voiceNo, textBuffer);
								// send it completely to EMIC, no matter how long it takes (maximum is approx. 1s (songs))
								while (!speechCtrl.bufferEmpty()) { 
									soundWaveLoop();
									speechCtrl.loop();
								}								
								linkToRc.sendRCSpeechResponse(speechResponse);
							}							
							break;
						case RC_SET_SPEED_REQ: {
							int16_t parSpeedX, parSpeedY, parOmega;
							linkToRc.parseRCSetSpeedRequest(request,parSpeedX,parSpeedY,parOmega);
							
							speedX = parSpeedX;
							speedY = parSpeedY;
							omega = parOmega;

							Datagram setSpeedReponse;
							
							// send a couple of characters of what Paul is currently talking about
							// use rest of the buffer. Due to Paul's slow tongue it needs to be a couple of characters only
							char text[SET_SPEED_TEXT_SIZE];
							uint8_t idx =0;
							char c;
							do {
								c = speechCtrl.readFromSpeechBuffer();
								text[idx++] = c;
							} while ((c != 0) && (idx<SET_SPEED_TEXT_SIZE-1));
							text[idx++] = 0;

							// in case there is something to be said, do it  here and delay the reply accordingly
							// (since SoftwareSerial & HardwareSerial produce a conflict if working simultaneously, 
							// since the interrupt the hardwareSerial is based on is turned off a very short period 
							// of time when SoftwareSerial sends a byte)
							moodCtrl.loop();			// trigger new saying
							
							// TODO: switch that loop off, check if parts can be sent per request
							while (!speechCtrl.bufferEmpty())
								speechCtrl.loop();		// very quick in most cases, but can take 200ms if Paul says something
								
							linkToRc.sendRCSetSpeedResponse(getLastVoltage_fp8(),botBalanceMode, tiltX_fp9,tiltY_fp9,posX, posY, beingMobbed, text, setSpeedReponse);
	
							// mobbing state has been sent to remote, reset it
							beingMobbed = false;
							
							// now initiate the call to the bot board, so remote has to wait at least 50ms after receiving the reply 
							// until the next cycle starts
							callSetSpeed(); 				
							}					
							break;
						case RC_SET_OPTIONS_REQ: {
							uint8_t volume;
							LanguageType language;
							VoiceType voiceNo;
							uint16_t talkingSpeed;
							Datagram setRCOptionsRes;

							linkToRc.parseRCOptionsRequest(request,volume, voiceNo, language,talkingSpeed);
							linkToRc.sendRCOptionsResponse(setRCOptionsRes, volume,voiceNo,language,talkingSpeed);
							/*
							Serial.print(F("language="));
							Serial.println(language);
							Serial.print(F("voice="));
							Serial.println(voiceNo);
							Serial.print(F("talkingSpeed="));
							Serial.println(talkingSpeed);
							*/
							speechCtrl.setVolume(volume);
							speechCtrl.selectVoice(voiceNo);
							speechCtrl.setLanguage(language);
							speechCtrl.setSpeedRate(talkingSpeed);
							
							// write to EEPROM in 15s (this is done in call of memory.loop())
							memory.delayedSave(15000);
							}							
							break;
						default:
							Serial.print(F("main:inv cmd:"));
							Serial.println(request.getMessageType());
					} // switch 					
					
					// we got a a message from the remote, reset heartbeat
					remoteIsOn = true;				// indicates that remote is appearently alive
					remoteOnTimer.setDueTime(0);	// reset heartbeat timer from remote		
				}

				// if inputchar is the Message magic number, then the receive operation failed
				// due to timeout or checksum
				if ((inputChar != 0) && (inputChar != MESSAGE_MAGIC_NUMBER)) {
					dispatchMenuInputChar(inputChar);
				} // if inputChar != 0				
			} // if Serial.available

			// print everything from bot board to serial
			while (Serial1.available()) {
				char inputChar = Serial1.read();		
				Serial.print(inputChar);
			}	


			// if remote has not been sent a heartbeat in the time frame for three calls, it must be off (i.e. 0,5s)
			if (remoteIsOn && (remoteOnTimer.isDue_ms(3*(1000/REMOTECONTROL_FREQUENCY))))
				remoteIsOn = false; // remote call is missing
		
			if (!remoteIsOn) {
				// if remote is off, call bot board without being triggered by the remote
				// if remote is on, we do this in the synchronous call 
				if (commTimer.isDue_ms((1000/REMOTECONTROL_FREQUENCY))) {				
					 callSetSpeed(); 
				} // if bot timer.due
			} // if remote is On													
						
			// check if something has to be said. Do this directly after the call to the bot board
			// since we have 1000/REMOTECONTROL_FREQUENCY ms time for this one

			moodCtrl.loop();
			speechCtrl.loop(); // no conflict between HardwareSerial & SoftwareSerial, since we are the master of all communication

			// grab amplitude and let LEDS blink, do this very often, since it depends on the current amplitude of the voice
			soundWaveLoop();

			// check if balance button has been pushed
			if (getBalanceSwitchPressed()) {
				botBalanceMode = !botBalanceMode;
			}
			
			// delete error lamp after 1s
			if (errorOffTimer.isDue_ms(1000))
				digitalWrite(ERROR_LED,LOW);
		
			if (voltageTimer.isDue_ms(1000)) { // check the voltage each seconds
				float voltage = getCurrentVoltage();
				if ((voltage < LOW_BAT_3S) &&  (voltage > 8.2)) // if it's below 8.2 V we are on the fixed power supply on jochen's desk
					moodCtrl.setMood(LowBattMood);
			
				// potentially carry out delayed write to EEPROM
				memory.loop();	
			}
			if (botBalanceMode) {
				if (balanceLEDBlinker.isDue_ms(300)) 
					if (digitalRead(IS_BALANCING_LED) == HIGH)
						digitalWrite(IS_BALANCING_LED,LOW);
					else
						digitalWrite(IS_BALANCING_LED,HIGH);
			}
			else {
				digitalWrite(IS_BALANCING_LED,LOW);
			}	
		} // if !proxyOn			 	
	} // while true
}

