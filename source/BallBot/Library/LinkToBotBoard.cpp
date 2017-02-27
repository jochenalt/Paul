/*
 * CommLinkToPaul.cpp
 *
 * Created: 28.03.2013 11:05:37
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "MsgType.h"
#include "LinkMainBot.h"
#include "LinkToBotBoard.h"
#include <avr/wdt.h>

extern ControlConfigurationType ctrlConfig;

void CommLinkMainAndBot::setup() {
	linkToBalBoard.setup();
}

void CommLinkMainAndBot::callControlConfiguration() {
	Datagram configureReq;
	Datagram configureRes;
							
	linkToBalBoard.sendConfigureRequest(configureReq, true, ctrlConfig);
					
	TimePassedBy timeout;
	while (!timeout.isDue_ms(30)) {
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBalBoard.receive(inputChar,15283,configureRes)) {
				if (configureRes.getMessageType() == CONFIGURE_RES) {
					linkToBalBoard.parseConfigureResponse(configureRes, ctrlConfig);
					break;
				}			
				else {																					
					setErrorLamp(F("reply unexpected"));
					break;
				}																									
			}
			else{
				setErrorLamp(F("reply timeout"));
				break;
			}	
		}
	}
}																					




void CommLinkMainAndBot::callCalibrateIMU(int16_t &offsetX, int16_t &offsetY, int16_t &offsetZ) {
	Datagram imuRequest;
	Datagram imuResponse;
	linkToBalBoard.sendCalibrateIMURequest(imuRequest);		
							
	// wait for reply, IMU calibration takes 3 seconds
	wdt_enable(WDTO_4S);
	delay(1000); // takes 4 seconds, so wait 3
	wdt_enable(WDTO_4S);						
	delay(1000); // takes 4 seconds, so wait 3
	wdt_enable(WDTO_4S);						
	delay(1000); // takes 4 seconds, so wait 3
	wdt_enable(WDTO_4S);						
		
	TimePassedBy timeout;
	while (!timeout.isDue_ms(3000)) {
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			if (linkToBalBoard.receive(inputChar,11723,imuResponse)) {
				if (imuResponse.getMessageType() == CALIBRATE_IMU_RES) {
					linkToBalBoard.parseCalibrateIMUResponse(imuResponse, offsetX, offsetY, offsetZ);
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
	}
	wdt_enable(WDTO_1S);
}						



boolean CommLinkMainAndBot::callSetSpeed(	boolean pBotBalanceMode, int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, 
										int16_t &pTiltX_fp9, int16_t& pTiltY_fp9,int16_t& pPosX, int16_t& pPosY, 
										int16_t &pAccelX,int16_t &pAccelY,
										void (*pSpareTimeLoop)(void) ) {
	Datagram setSpeedRequest;
	Datagram setSpeedResponse;
				
	linkToBalBoard.sendSetSpeedRequest(pBotBalanceMode, pSpeedX, pSpeedY, pOmega, setSpeedRequest);		
	TimePassedBy timeout;
	while (!timeout.isDue_ms(50)) {
		pSpareTimeLoop(); // LED, Speech amplitude , Mood, but no speech
		if (Serial1.available()) {
			char inputChar = Serial1.read();
			
			if (linkToBalBoard.receive(inputChar,50230,setSpeedResponse)) {
				if (setSpeedResponse.getMessageType() == SET_SPEED_RES) {
					linkToBalBoard.parseSetSpeedResponse(setSpeedResponse,pTiltX_fp9, pTiltY_fp9, pPosX, pPosY, pAccelX, pAccelY);
					return true;
				}	
				else {
					setErrorLamp(F("set speed wrong response"));							
					return false;
				}							
			}
			else {
				setErrorLamp(F("speed timeout"));
				return false;
			}
		} // if Serial1.available
	} // while (!timeout)
	setErrorLamp(F("no response on set speed"));
	return false;
}	
