/*
 * CommLinkToPaul.cpp
 *
 * Created: 28.03.2013 11:05:37
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "CommLinkToPaul.h"
#include <avr/wdt.h>

void CommLinkToPaul::setup(void (*pSpareTimeLoop)(void)) {
	linkToBot.setup();
	spareTimeLoop = pSpareTimeLoop;
	missedCommunications = 0;
}


void CommLinkToPaul::callCalibrateIMU(int16_t &offsetX, int16_t &offsetY, int16_t &offsetZ) {
	Datagram imuRequest;
	Datagram imuResponse;
	linkToBot.sendRCCalibrateIMURequest(imuRequest);		
		
	TimePassedBy timeout;
	while (!timeout.isDue_ms(5000)) {
		wdt_reset();
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBot.receive(inputChar,12834,imuResponse)) {
				if (imuResponse.getMessageType() == RC_CALIBRATE_IMU_RES) {
					linkToBot.parseRCCalibrateIMUResponse(imuResponse, offsetX, offsetY, offsetZ);
					break;
				}
				else {
					setErrorLamp(F("calibrate wrong method"));
					break;
				}													
			}
			else {
				setErrorLamp(F("calibrate timeout"));
				break;
			}					
		}
		else {
			if (spareTimeLoop != NULL)
				spareTimeLoop();				
		}			
	}
}						

void CommLinkToPaul::callControlConfiguration(ControlConfigurationType &pCtrlConfig) {
	Datagram configureReq;
	Datagram configureRes;
							
	linkToBot.sendRCConfigureRequest(configureReq, true, pCtrlConfig);
								
	TimePassedBy timeout;
	while (!timeout.isDue_ms(30)) {
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBot.receive(inputChar,15023,configureRes)) {
				if (configureRes.getMessageType() == RC_CONFIGURE_RES) {
					linkToBot.parseRCConfigureResponse(configureRes, pCtrlConfig);
					Serial.println(F("calibration sent"));
					break;
				}			
				else {																					
					setErrorLamp(F("unexpected reply in sendRCConfigureRequest of type "));
					Serial.println(configureRes.getMessageType());
					break;
				}																									
			}
			else{
				setErrorLamp(F("reply timeout"));
				break;
			}	
		}
		else
			if (spareTimeLoop != NULL)
				spareTimeLoop();				
	}
}

void CommLinkToPaul::callSendOption(uint8_t pVolume, VoiceType pVoice, LanguageType pLanguage, uint16_t pSpeechSpeed) {
	Datagram optionsReq;
	Datagram optionsRes;
							
	linkToBot.sendRCOptionsRequest(optionsReq, pVolume, pVoice, pLanguage,pSpeechSpeed);
	TimePassedBy timeout;
	while (!timeout.isDue_ms(30)) {
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBot.receive(inputChar,15120,optionsRes)) {
				if (optionsRes.getMessageType() == RC_SET_OPTIONS_RES) {
					uint8_t volume;
					LanguageType language;
					VoiceType voiceNo;
					uint16_t speed;
 					linkToBot.parseRCOptionsResponse(optionsRes,volume,voiceNo,language,speed);
					break;
				}			
				else {																					
					setErrorLamp(F("sendRCOptionsRequest:reply unexpected"));
					Serial.print(F("message type="));
					Serial.println(optionsRes.getMessageType());

					break;
				}																									
			}
			else{
				setErrorLamp(F("reply timeout"));
				break;
			}	
		} else
			if (spareTimeLoop != NULL)
				spareTimeLoop();				
	}	
}

void CommLinkToPaul::callSpeechRequest(VoiceType pVoiceType, PoemType pPoemNo,  SongType pSongType, char* pText) {
	Datagram optionsReq;
	Datagram speechRes;
							
	linkToBot.sendRCSpeechRequest(optionsReq, pVoiceType, pPoemNo, pSongType, pText);
	TimePassedBy timeout;
	boolean replyReceived = false;
	// speech requests go to EMIC with 9600 baud, this can actually take 1,5s
	while (!timeout.isDue_ms(1500)) {
		wdt_reset();
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBot.receive(inputChar,12834,speechRes)) {
				if (speechRes.getMessageType() == RC_SPEECH_RES) {
					linkToBot.parseRCSpeechResponse(speechRes);
					replyReceived = true;
					break;
				}
				else {
					setErrorLamp(F("wrong method"));
					break;
				}													
			}
			else {
				setErrorLamp(F(" timeout"));
				break;
			}					
		}
		else {
			if (spareTimeLoop != NULL)
				spareTimeLoop();				
		}			
	}
	if (!replyReceived)
		setErrorLamp(F("RC:missing reply to RCSpeechRequest"));
}

void CommLinkToPaul::callSetSpeed(	int16_t speedX, int16_t speedY, int16_t omega, 
									float &pBotVoltage, boolean &pIsbalancing, 
									float &pTiltX, float &pTiltY, int16_t &pPosX, int16_t &pPosY, 
									boolean &pBeingMobbed, char pTextBuffer[], boolean &pCommLinkOn) {
	Datagram setRCSpeedRequest;
	Datagram setRCSpeedResponse;
		
	linkToBot.sendRCSetSpeedRequest(speedX, speedY, omega, setRCSpeedRequest);	

	TimePassedBy timeout;
	pTextBuffer[0] = 0;
	while (!timeout.isDue_ms(50)) {
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBot.receive(inputChar,51230,setRCSpeedResponse)) {
				if (setRCSpeedResponse.getMessageType() == RC_SET_SPEED_RES) {
					int16_t botVoltage_fp8;
					int16_t tiltX_fp9;
					int16_t tiltY_fp9;
					linkToBot.parseRCSetSpeedResponse(setRCSpeedResponse,botVoltage_fp8, pIsbalancing, 
									tiltX_fp9, tiltY_fp9, pPosX, pPosY, 
									pBeingMobbed, pTextBuffer);
					pCommLinkOn = true;
					missedCommunications = 0;
					pBotVoltage = FP2FLOAT(botVoltage_fp8,8);	
					pTiltX = FP2FLOAT(tiltX_fp9,9);
					pTiltY = FP2FLOAT(tiltY_fp9,9);	
					return; // communication worked without error
				}
				break; // leave in error due to wrong message type
			} // if comm.receive	
			else {
				if (inputChar != MESSAGE_MAGIC_NUMBER) {
					// if Paul sends something in the middle of this request (like speech), this is no errorness situation
					Serial.print(inputChar);
				} 
				else 
					setErrorLamp(F("rc:timeout or corrupt"));							
				break; // leave in error due to timeout
			}							
		} // if Serial1.isAvailable
		else
			if (spareTimeLoop != NULL)
				spareTimeLoop();				
	} // while	!timeout of reply

	if (pCommLinkOn) {
		missedCommunications++;
		if (missedCommunications >= 3 ) { // after three corrupt communications in a row we say link is broken
			pCommLinkOn = false;
			missedCommunications = 0;
		}											
	} 
}


