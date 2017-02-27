/*
 * ViewMain.h
 *
 * Created: 23.03.2013 20:41:24
 *  Author: JochenAlt
 */ 


#ifndef VIEWMAIN_H_
#define VIEWMAIN_H_

#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"

#include "DisplayController.h"


class ViewMain : public View {
	
	public:
		ViewMain() {
			posX = 0;
			posY = 0; 
			tiltX = 0;
			tiltY = 0;
			omega = 0;
			speedX = 0;
			speedY = 0; 
			paulVoltage = 0.0;
			remoteVoltage = 0.0;
			isMobbed = false;
			textLen = 0;
		}			
		virtual void startup();
		void setVoltage(float pPaulVoltage, float pRemoteVoltage);
		void setKinematic(	int16_t pPosX, int16_t pPosY, 
							float pTiltX, float pTiltY,
							int16_t pOmega,
							int16_t pSpeedX, int16_t pSpeedY, 
							boolean pisMobbed);
		// compute the number of the bitmap representing the 2D angle of Paul
		void displayLoop();
		void setSpokenText(char* pText);
		void deleteTerminalArea();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);
	private:
		int16_t posX;
		int16_t posY; 
		float	tiltX;
		float	tiltY;
		int16_t omega;
		int16_t speedX;
		int16_t speedY; 
		float paulVoltage;
		float remoteVoltage;		
		
		TimePassedBy speechTimer;
		boolean isMobbed;
		uint8_t textLen;
		TimePassedBy updateTimer;
		TimePassedBy lowBatBlinkTimer;
		boolean lowBatBlink;

};

#endif /* VIEWMAIN_H_ */