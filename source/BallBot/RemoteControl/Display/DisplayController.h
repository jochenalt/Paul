/*
 * DisplayController.h
 *
 * Created: 20.03.2013 18:06:39
 *  Author: JochenAlt
 */ 


#ifndef DISPLAYCONTROLLER_H_
#define DISPLAYCONTROLLER_H_


#include "Arduino.h"
#include "eDIP-TFT.h"
#include "TimePassedBy.h"
#include "View.h"
#include "Paul.h"
#include "CommLinkToPaul.h"


enum ViewType {ViewSplashNo, ViewWaitForPaulNo, ViewPaulAwakeNo, ViewMainNo,
			  ViewSpeechPoemsNo, ViewSpeechSongsNo, ViewSpeechTalkNo,
			  ViewOptionsPaulNo, ViewOptionsConfigNo, ViewOptionsRemoteNo};


#define TOUCH_AREA_NO	13 // paul config dialog has most touch areas
#define SPOKEN_TEXT_SIZE 128
struct TouchArea {
	uint16_t x1;
	uint16_t x2;
	uint8_t y1;
	uint8_t y2;
	uint8_t id;
};

class DisplayController {
	friend class View;
	friend class ViewMain;
	friend class ViewWaitForPaul;
	friend class ViewPaulAwakes;
	friend class ViewSpeechPoems;
	friend class ViewSpeechTalk;
	friend class ViewSpeechSongs;
	friend class ViewOptionsPaul;
	friend class ViewOptionsConfig;
	friend class ViewSplash;
	friend class ViewOptionsRemote;
	

	public:
		// setup the displa controller with communication link to paul and 
		// function that is invoked whenever the display has some spare time
		void setup(CommLinkToPaul* pLinkToPaul,void (*pSpareTimeLoop)(void));
		
		// call whenever possible. Used to update any new data in the screen		
		void loop();
		
		// new voltage data
		void setVoltage(float pPaulVoltage, float pRemoteVoltage);
		
		// new data of Pauls position and speed
		void setKinematic(	boolean pIsBalancing,int16_t pPosX, int16_t pPosY, 
							float pTiltX, float pTiltY,
							int16_t pOmega,
							int16_t pSpeedX, int16_t pSpeedY,
							boolean pIsMobbed);
		// new data on Paul's text
		void addToSpokenText(char pSpokenText[]);

		// any key pressed has to be added here in order to 
		// update the display (depending on the current view)
		void setKey(char c);
		
		// returns true, if the view is on that proxies to paul
		boolean viewProxyToPaul();
		
		// set state of communication link to paul here
		void linkToPaul(boolean pLinkIsOn);
		
		// any data from Paul has to be added here (depending on the view it might be displayed
		void sendToTerminal(char c);

		// return active View 
		ViewType getState();
		void activateView(ViewType state);

		
		// return length of text passed with setSpokentext
		uint16_t getSpokenTextLen();

		// true, if something has been passed with setSpokenText
		boolean isSpokenTextAvail();
		
		// true, if text has been worked through and can be deleted in buffer
		void spokenTextDone();

		// return spokenText
		char* getSpokenText();

		// user did something
		boolean userActivityHappened(uint16_t pSince);
		void powerOff();
	private:
		void setTheme();
		void setThemeBitmapPage();
		void setDefaultBitmapPage();

		void defineAccentColor(uint8_t pRed,uint8_t pBlue,uint8_t pGreen);
		void addTouchArea(uint8_t pTouchId, uint16_t pX1, uint16_t pY1, uint16_t pX2, uint16_t pY2);
		void callTouchArea(uint8_t pTouchId);

		void deleteAllTouchAreas();
		// returns the id of the touch event defined by the parameters delivery by the display
		uint8_t identifyTouchEvent(uint16_t pTouchX, uint16_t pTouchY);
		
		void waitUntilLCDReady() {
			lcd.waitUntilReceiveBufferEmpty(spareTimeLoop);
		}

		ViewType state; 
		TimePassedBy stateTimer;
		boolean linkIsOn;
		boolean isBalancing;
		CommLinkToPaul* linkToPaulPtr;
		
		TouchArea touchArea[TOUCH_AREA_NO]; 
		uint8_t touchAreas;
		boolean newValuesSet;
		void (*spareTimeLoop)(void);
		char spokenText[SPOKEN_TEXT_SIZE];
		uint8_t spokenTextLen;
		boolean spokenTextAvail;
		uint32_t lastUserActivity;

};

extern DisplayController display;

#endif /* DISPLAYCONTROLLER_H_ */