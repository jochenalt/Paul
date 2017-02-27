/*
 * ViewMain.cpp
 *
 * Created: 03.04.2013 23:07:03
 *  Author: JochenAlt
 */ 



#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"
#include "ViewMain.h"


#include "DisplayController.h"

void ViewMain::startup() {
	display.waitUntilLCDReady();
	lcd.clear();
	display.deleteAllTouchAreas();
	char* windowTitle[] = {"Paul","Options","Speech"};
	uint8_t touchAreaID[] = {NO_TOUCH_AREA,TOUCH_AREA_OPTIONS,TOUCH_AREA_SPEECH};				
	drawTabbedWindowTitle(NULL,windowTitle,touchAreaID,3);
}

		void ViewMain::setVoltage(float pPaulVoltage, float pRemoteVoltage) {
			paulVoltage = pPaulVoltage;
			remoteVoltage = pRemoteVoltage;
		}
		void ViewMain::setKinematic(	int16_t pPosX, int16_t pPosY, 
							float  pTiltX, float pTiltY,
							int16_t pOmega,
							int16_t pSpeedX, int16_t pSpeedY, boolean pIsMobbed) { 
			posX = pPosX;
			posY = pPosY;
			tiltX = pTiltX;
			tiltY = pTiltY;
			omega = pOmega;
			speedX = pSpeedX;
			speedY = pSpeedY;
			if (pIsMobbed)
				isMobbed = true; // it reset when displayed;
		}

// compute the number of the bitmap representing the 2D angle of Paul
void ViewMain::displayLoop() {
	if (updateTimer.isDue_ms(1000/REMOTECONTROL_FREQUENCY) && display.newValuesSet) {
		display.newValuesSet = false;
		float x = constrain(tiltX,-10.0,10.0);
		float y = constrain(tiltY,-10.0,10.0);;

		#define CENTRE_X 60
		#define CENTRE_Y 160
	
		// compute degree and length of tilt vector
		float degreeRad= atan2(y,x) ;	
		float tiltLength = sqrt(x*x + y*y);
		uint8_t tiltStrength = constrain(tiltLength*5.0,0,25);

		float cosdegree = cos(degreeRad);
		float sindegree = sin(degreeRad);
		int16_t pXpx = -tiltStrength*cosdegree;
		int16_t pYpx = tiltStrength*sindegree ;
		// rotation matrix for lines between body and head
		float sina = sin(degreeRad+PI/4);
		float cosa = cos(degreeRad+PI/4);
		// draw lines to head
		int16_t l1ax = CENTRE_X - cosa*10.0 + sina*10.0;
		int16_t l1ay = CENTRE_Y + sina*10.0 + cosa*10.0;
		int16_t l1bx = CENTRE_X + cosa*10.0 - sina*10.0;
		int16_t l1by = CENTRE_Y - sina*10.0 - cosa*10.0;
		int16_t l2ax = l1ax + pXpx;
		int16_t l2ay = l1ay + pYpx;
		int16_t l2bx = l1bx + pXpx;
		int16_t l2by = l1by + pYpx;
	
		// draw background (bitmap is 107x107)
		display.setThemeBitmapPage();
		lcd.setLineColor(MAINVIEW_PAUL_LINE_COLOR, TRANSPARENT_COLOR);
		lcd.setTextFont(FONT_DEFAULT);
		lcd.setTextColor(ACTIVE_TEXT_COLOR_NO, BLACK_NO);
		lcd.setNoTransparency();
		display.waitUntilLCDReady(); // the following flashs a bit and needs to be as fast as possible

		lcd.displayFlashPicture(CENTRE_X-53,CENTRE_Y-53,MAINVIEW_PAUL_BACKGROUND_BITMAP_NO);			
		lcd.drawLine(l1ax,l1ay,l2ax,l2ay);
		lcd.drawLine(l1bx,l1by,l2bx,l2by);
		// draw head (bitmap is 30x30)
		lcd.setTransparencyByCorner();
		lcd.displayFlashPicture(CENTRE_X+pXpx-15,CENTRE_Y+pYpx-15,MAINVIEW_PAUL_HEAD_BITMAP_NO);
		// draw z axis
		lcd.setLineColor(WHITE_NO, TRANSPARENT_COLOR);
		lcd.drawLine(CENTRE_X+pXpx,CENTRE_Y+pYpx, CENTRE_X+pXpx*2,CENTRE_Y+pYpx*2);
				
		if (textLen == 0) {
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_SMALL);
			lcd.drawText(180,100,'L',"Pos");
			lcd.drawText(180,114,'L',"Tilt");
			lcd.drawText(180,128,'L',"w");
				
			// draw text
			char buffer[32];
			sprintf_P(buffer,PSTR(" = (%d mm,%d mm)  "),posX,posY);
			lcd.drawText(205,100,'L',buffer);
			uint16_t length = tiltLength*2;
			int16_t degreeDegree = degreeRad/PI*180.0+90;
			while (degreeDegree <-180)
				degreeDegree += 360;
			while (degreeDegree >180)
				degreeDegree -= 360;
			sprintf_P(buffer,PSTR("= (%3d °,%3d cm)  "),degreeDegree,length);
			lcd.drawText(205,114,'L',buffer);
			sprintf_P(buffer,PSTR("= %d °/s  "),-omega);
			lcd.drawText(205,128,'L',buffer);
			lcd.setTextFont(FONT_LARGE);
			lcd.setTextColor(ACCENT_COLOR1_NO,BLACK_NO);
			if (lowBatBlinkTimer.isDue_ms(300))
				lowBatBlink = !lowBatBlink;
	
			if ((paulVoltage <  LOW_BAT_3S) && lowBatBlink)
				lcd.setTextColor(WHITE_NO,BLACK_NO);
			sprintf_P(buffer,PSTR("Paul has %.1f V"),paulVoltage);		
			lcd.drawText(180,204,'L',buffer);

			lcd.setTextColor(ACCENT_COLOR1_NO,BLACK_NO);
			if ((remoteVoltage <  LOW_BAT_2S) && lowBatBlink) 
				lcd.setTextColor(WHITE_NO,BLACK_NO);

			sprintf_P(buffer,PSTR("Remote has %.1f V"),remoteVoltage);
			lcd.drawText(180,220,'L',buffer);
			
			// draw speed
			lcd.setTextFont(FONT_DEFAULT);
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BLACK_NO);
			sprintf_P(buffer,PSTR("  %d mm/s  "),speedY);
			lcd.drawText(CENTRE_X,CENTRE_Y-70,'C',buffer);
			sprintf_P(buffer,PSTR("  %d mm/s  "),-speedX);
			lcd.drawText(CENTRE_X+84,CENTRE_Y+8,'C',buffer);
		};

		// display text
		if (display.isSpokenTextAvail() && (textLen != display.getSpokenTextLen())) {
			if (textLen == 0)
				deleteTerminalArea();
			int16_t x,y;
			lcd.setTextFont(FONT_DEFAULT);
			lcd.setTextColor(ACCENT_COLOR1_NO,BLACK_NO);
			lcd.writeSentenceInArea(180,70,LCD_WIDTH-1,LCD_HEIGHT-1,display.getSpokenText(),x,y);
			textLen = display.getSpokenTextLen();
			speechTimer.setDueTime(0);
		}						
			
		if ((textLen > 0) && speechTimer.isDue_ms(3000+textLen*40)) {
			deleteTerminalArea();
			display.spokenTextDone();
			textLen = 0;
		}				
	}
}				
		
void ViewMain::deleteTerminalArea() {
	// delete terminal remains
	lcd.deleteRect(180,40,LCD_WIDTH-1,LCD_HEIGHT-1);
}



void ViewMain::loop() {
	displayLoop();
}
		
void ViewMain::touchEvent(uint8_t pTouchId) {
	switch (pTouchId) {
		case TOUCH_AREA_SPEECH:
			display.activateView(ViewSpeechPoemsNo);
			break;
		case TOUCH_AREA_OPTIONS:
			display.activateView(ViewOptionsPaulNo);
			break;
		default:
			Serial.print(F("unknown touch area "));
			Serial.println(pTouchId);
			break;
	}
}			

