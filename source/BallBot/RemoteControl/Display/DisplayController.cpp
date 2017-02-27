/*
 * DisplayController.cpp
 *
 * Created: 20.03.2013 18:05:46
 *  Author: JochenAlt
 */ 

#include "DisplayController.h"
#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"

#include "FixedPoint.h"
#include "setup.h"

#include "View.h"
#include "ViewSplash.h"
#include "ViewMain.h"
#include "ViewPaulAwakes.h"
#include "ViewWaitForPaul.h"
#include "ViewSpeechPoems.h"
#include "ViewSpeechTalk.h"
#include "ViewSpeechSongs.h"
#include "ViewOptionsConfig.h"
#include "ViewOptionsPaul.h"
#include "ViewOptionsRemote.h"

#include "CommLinkToPaul.h"
#include "RemoteMemory.h"

// instantiate all view
ViewPaulAwakes viewPaulAwakes;
ViewMain viewMain;
ViewWaitForPaul viewWaitForPaul;
ViewSpeechPoems viewSpeechPoems;
ViewSpeechTalk viewSpeechTalk;
ViewSpeechSongs viewSpeechSongs;
ViewOptionsPaul viewOptionsPaul;
ViewOptionsRemote viewOptionsRemote;
ViewOptionsConfig viewOptionsConfig;
ViewSplash viewSplash;

DisplayController display;

void DisplayController::defineAccentColor(uint8_t pRed,uint8_t pBlue,uint8_t pGreen) {
	lcd.assignColor(ACCENT_COLOR1_NO, pRed,pBlue,pGreen); // water blue
	lcd.assignColor(ACCENT_COLOR2_NO,constrain(pRed-64,0,255),constrain(pBlue-64,0,255),constrain(pGreen-64,0,255));	
	
}

void DisplayController::setTheme() {
	switch (memory.persistentMem.displayTheme) {
		case 0:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME1_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME1_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME1_MAINVIEW_LINE_RGB_COLOR); 
			break;
		case 1:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME2_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME2_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME2_MAINVIEW_LINE_RGB_COLOR); 
			break;
		case 2:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME3_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME3_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME3_MAINVIEW_LINE_RGB_COLOR); 
			break;
		case 3:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME4_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME4_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME4_MAINVIEW_LINE_RGB_COLOR); 
			break;
		case 4:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME5_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME5_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME5_MAINVIEW_LINE_RGB_COLOR); 
			break;
		case 5:
			// define palette
			lcd.assignColor(ACCENT_COLOR1_NO, THEME6_FIRST_RGB_COLOR); // water blue
			lcd.assignColor(ACCENT_COLOR2_NO,THEME6_SECOND_RGB_COLOR);	
			lcd.assignColor(MAINVIEW_PAUL_LINE_COLOR, THEME6_MAINVIEW_LINE_RGB_COLOR); 
			break;

		default:
			Serial.println(F("invalid theme "));
			memory.persistentMem.displayTheme = 0;
			break;
	}
}

void DisplayController::setThemeBitmapPage() {
	lcd.setPage(memory.persistentMem.displayTheme+1);
}

void DisplayController::setDefaultBitmapPage() {
	lcd.setPage(0);
}


void DisplayController::setup(CommLinkToPaul* pLinkToPaul,void (*pSpareTimeLoop)(void)) {
	lcd.setup(EDIP_SS_PIN,EDIP_SBUF_PIN);
	lcd.clear();
	lcd.setBrightness(constrain(memory.persistentMem.displayBrightness,30,100));
		
	isBalancing = false;
	linkIsOn = false;
	touchAreas = 0; 
	lastUserActivity = millis();
	spokenTextDone(); // reset spoken text buffer
	
	linkToPaulPtr = pLinkToPaul;
	newValuesSet = false;
	setTheme();	
	
	activateView(ViewSplashNo);
}

void DisplayController::setVoltage(float pPaulVoltage, float pRemoteVoltage) {
	viewMain.setVoltage(pPaulVoltage,pRemoteVoltage);
	newValuesSet = true;
}

void DisplayController::setKinematic(	boolean pIsBalancing,int16_t pPosX, int16_t pPosY, 
							float pTiltX, float pTiltY,
							int16_t pOmega,
							int16_t pSpeedX, int16_t pSpeedY,boolean  pIsMobbed) {
								

	viewMain.setKinematic(pPosX, pPosY, pTiltX, pTiltY,pOmega,pSpeedX, pSpeedY,pIsMobbed); 
	isBalancing = pIsBalancing;
	newValuesSet = true;
	lastUserActivity = millis();
}	

boolean DisplayController::userActivityHappened(uint16_t pSince) {
	return ((millis()-lastUserActivity) < (uint32_t(pSince)*1000UL));
}


ViewType DisplayController::getState() {
	return state;
}

boolean DisplayController::viewProxyToPaul() {
	return (state == ViewOptionsPaulNo);
}	
		
void DisplayController::setKey(char c) {
	// Serial.print(F("setkey "));
	// Serial.println((int)c);

	if ((state != ViewSpeechTalkNo) && (state != ViewOptionsConfigNo)) {
		activateView(ViewSpeechTalkNo);
	}
	if (state == ViewSpeechTalkNo)
		viewSpeechTalk.addChar(c);
		
	if (state == ViewOptionsConfigNo) {
		viewOptionsConfig.addKey(c);
	}		
}

void DisplayController::activateView(ViewType pNewState) {
	switch (state) {
		case ViewSplashNo:
			viewSplash.teardown();
			break;
		case ViewWaitForPaulNo:
			viewWaitForPaul.teardown();
			break;
		case ViewPaulAwakeNo:
			viewPaulAwakes.teardown();
			break;
		case ViewMainNo:
			viewMain.teardown();
			break;
		case ViewSpeechPoemsNo:
			viewSpeechPoems.teardown();
			break;
		case ViewSpeechSongsNo:
			viewSpeechSongs.teardown();
			break;
		case ViewSpeechTalkNo:
			viewSpeechTalk.teardown();
			break;
		case ViewOptionsConfigNo:
			viewOptionsConfig.teardown();
			break;
		case ViewOptionsPaulNo:
			viewOptionsPaul.teardown();
			break;
		case ViewOptionsRemoteNo:
			viewOptionsRemote.teardown();
			break;
		default:	
			Serial.print(F("activateView:invalid state"));
			Serial.println(pNewState);

			break;
	}

	switch (pNewState) {
		case ViewSplashNo:
			viewSplash.startup();
			break;
		case ViewWaitForPaulNo:
			if (state != ViewSplashNo)
				lcd.clear();
			viewWaitForPaul.startup();
			break;
		case ViewPaulAwakeNo:
			viewPaulAwakes.startup();
			break;
		case ViewMainNo:
			viewMain.startup();
			break;
		case ViewSpeechPoemsNo:
			viewSpeechPoems.startup();;
			break;
		case ViewSpeechSongsNo:
			viewSpeechSongs.startup();
			break;
		case ViewSpeechTalkNo:
			viewSpeechTalk.startup();
			break;
		case ViewOptionsConfigNo:
			viewOptionsConfig.startup();
			break;
		case ViewOptionsPaulNo:
			viewOptionsPaul.startup();
			break;
		case ViewOptionsRemoteNo:
			viewOptionsRemote.startup();
			break;

		default:	
			Serial.print(F("setUIState:invalid state"));
			Serial.println(pNewState);

			break;
	}
	
	state = pNewState;
}	

void DisplayController::callTouchArea(uint8_t pTouchId) {
	switch (state) {
		case ViewSplashNo:
			viewSplash.touchEvent(pTouchId);
			break;
		case ViewWaitForPaulNo:
			viewWaitForPaul.touchEvent(pTouchId);
			break;
		case ViewPaulAwakeNo:
			viewPaulAwakes.touchEvent(pTouchId);
			break;
		case ViewMainNo:
			viewMain.touchEvent(pTouchId);
			break;
		case ViewOptionsConfigNo:
			viewOptionsConfig.touchEvent(pTouchId);
			break;
		case ViewOptionsPaulNo:
			viewOptionsPaul.touchEvent(pTouchId);
			break;
		case ViewOptionsRemoteNo:
			viewOptionsRemote.touchEvent(pTouchId);
			break;
		case ViewSpeechSongsNo:
			viewSpeechSongs.touchEvent(pTouchId);
			break;
		case ViewSpeechTalkNo:
			viewSpeechTalk.touchEvent(pTouchId);
			break;
		case ViewSpeechPoemsNo:
			viewSpeechPoems.touchEvent(pTouchId);
			break;

		default:
			// dont care for a broken link, only in main dialogs
			break;
	}
}

void DisplayController::loop() {
	switch (state) {
		case ViewSplashNo:
			viewSplash.loop();
			if (viewSplash.isFinished())
				activateView(ViewWaitForPaulNo);				
			break;
		case ViewWaitForPaulNo:
			viewWaitForPaul.loop();
			if (linkIsOn)
				activateView(ViewPaulAwakeNo);
			break;
		case ViewPaulAwakeNo:
			viewPaulAwakes.loop();
			if (isBalancing)
				activateView(ViewMainNo);
			if (!linkIsOn) {
				lcd.clear();
				activateView(ViewWaitForPaulNo);				
			}				
			break;
		case ViewMainNo:
			viewMain.loop();
			if (!linkIsOn)
				activateView(ViewWaitForPaulNo);
			break;
		case ViewOptionsConfigNo:
			viewOptionsConfig.loop();
			break;
		case ViewOptionsPaulNo:
			viewOptionsPaul.loop();
			break;
		case ViewOptionsRemoteNo:
			viewOptionsRemote.loop();
			break;
		case ViewSpeechSongsNo:
			viewSpeechSongs.loop();
			break;
		case ViewSpeechTalkNo:
			viewSpeechTalk.loop();
			break;
		case ViewSpeechPoemsNo:
			viewSpeechPoems.loop();
			break;

		default:
			// dont care for a broken link, only in main dialogs
			break;
	}
	
	// check for events from touchscreen
	char sendBuffer[64];
	DisplayEDIP::TouchEventType pType;
	uint16_t touchedX, touchedY;
	uint8_t sendBufferLen;
	if (lcd.requestSendBuffer(sendBuffer, sizeof(sendBuffer),sendBufferLen)) {
		if (lcd.parseTouchEvent(sendBuffer,pType, touchedX, touchedY)) {
			if (pType == DisplayEDIP::DETOUCH_EVENT) {
				lastUserActivity = millis();
				uint8_t touchId = identifyTouchEvent(touchedX, touchedY);
				if (touchId != 0) {
					callTouchArea(touchId);
				}					
				else	{
					Serial.println(F("unregistered touch event at"));
					Serial.print(touchedX);
					Serial.print(",");
					Serial.println(touchedY);
					Serial.println(")");
				}
			}
		} else {
			Serial.println(F("unknown event"));
			/*
			for (int i = 0;i<sendBufferLen;i++) {
				if ((sendBuffer[i]>=32) && (sendBuffer[i]<127))
					Serial.print(sendBuffer[i]);
				else
					Serial.print((int)sendBuffer[i]);
					Serial.print(' ');
			}
			Serial.println();
			*/
		}
	}
}

void DisplayController::addTouchArea(uint8_t pTouchId, uint16_t pX1, uint16_t pY1, uint16_t pX2, uint16_t pY2) {
	if (touchAreas < TOUCH_AREA_NO) {
		// lcd.drawRect(pX1, pY1, pX2, pY2);
		lcd.defineTouchArea(pX1, pY1, pX2, pY2);
		touchArea[touchAreas].id = pTouchId;
		touchArea[touchAreas].x1 = pX1;
		touchArea[touchAreas].y1 = pY1;	
		touchArea[touchAreas].x2 = pX2;
		touchArea[touchAreas].y2 = pY2;	
		touchAreas = touchAreas + 1;
	} else {
		Serial.println(F("too few touch areas"));
	}
}

void DisplayController::deleteAllTouchAreas() {
	touchAreas = 0;
	lcd.removeTouchArea(0,0);
}
		
uint8_t DisplayController::identifyTouchEvent(uint16_t pTouchX, uint16_t pTouchY) {
	for (int i = 0;i<touchAreas;i++) {
		if ((touchArea[i].x1 <= pTouchX) && (pTouchX <= touchArea[i].x2) && 
			(touchArea[i].y1 <= pTouchY) && (pTouchY <= touchArea[i].y2)) {
			return touchArea[i].id;	
		}
	}
	Serial.println(F("no touch area"));
	return 0;		
}
		
	
void DisplayController::linkToPaul(boolean pLinkIsOn) {
	linkIsOn = pLinkIsOn;						
	
}

void DisplayController::sendToTerminal(char c) {
	if (state == ViewOptionsConfigNo)
		viewOptionsConfig.addKey(c);
}

boolean DisplayController::isSpokenTextAvail() {
	return spokenTextAvail;
}
uint16_t DisplayController::getSpokenTextLen() {
	return spokenTextLen;
}
void DisplayController::spokenTextDone() {
	spokenTextAvail = false;
	spokenText[0] = 0;
	spokenTextLen = 0;
}	
char* DisplayController::getSpokenText() {
	return spokenText;
}

void DisplayController::addToSpokenText(char pSpokenText[]) {
	if (pSpokenText != NULL && (strlen(pSpokenText)>0)) {
		uint8_t len = strlen(pSpokenText);
		
		// delete first word until spokentext is long enough to add pSpokenText
		while (len + spokenTextLen > SPOKEN_TEXT_SIZE) {
			char delimiters[] = {' ','!','.','.',',','-',':',';','?', char(13),char(10), char(0)};
			char* ptr = strtok(spokenText, delimiters);
			uint8_t tokenLen = SPOKEN_TEXT_SIZE - len; // in case we dont find a word, cut off the required length
			if (ptr != NULL) 
				tokenLen = strlen(ptr);
			
			strcpy(spokenText, spokenText + tokenLen);
			spokenText[spokenTextLen+1-tokenLen] = 0;
			spokenTextLen = spokenTextLen-tokenLen;
		}
		
		// from now on we have enough free space in spokenText
		strcpy(spokenText+spokenTextLen,pSpokenText);
		spokenTextLen = strlen(spokenText);
		spokenTextAvail = true;
	}
}

// display a nice shut down message. No need for an own view 
void DisplayController::powerOff() {
	lcd.clear();
	lcd.setTextColor(WHITE_NO,BLACK_NO);
	lcd.setTextFont(FONT_GIANT);
	lcd.drawText(60,100,'L',F("Power Off"));
	lcd.setTextFont(FONT_DEFAULT);
	lcd.setTextColor(ACCENT_COLOR1_NO, BLACK_NO);
	lcd.drawText(60,140,'L',F("Paul says thank you"));

}
