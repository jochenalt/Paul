/*
 * ViewPaulAwakes.cpp
 *
 * Created: 03.04.2013 23:11:06
 *  Author: JochenAlt
 */ 


#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "View.h"
#include "ViewPaulAwakes.h"
#include "DisplayController.h"
#include "TimePassedBy.h"

#define PAUL_X 130
#define PAUL_Y 45
#define PAUL_EYES_X (PAUL_X + 11)
#define PAUL_EYES_Y (PAUL_Y + 30)


void ViewPaulAwakes::startup() {
	display.waitUntilLCDReady();
	lcd.clear();
	lcd.setNoTransparency();
	display.setThemeBitmapPage();
	lcd.displayFlashPicture(PAUL_X,PAUL_Y,PAUL_BITMAP_NO);
	display.deleteAllTouchAreas();
	lcd.setTransparencyByCorner();
	display.addTouchArea(TOUCH_AREA_SPLASH,0,0,LCD_WIDTH,LCD_HEIGHT);
	blinkStep = 0;
	textLen = 0;
}
		
void ViewPaulAwakes::paulSleeps() {
	lcd.setTransparencyByCorner();
	lcd.displayFlashPicture(PAUL_EYES_X,PAUL_EYES_Y,PAULS_EYES_FULL_BLINK_BITMAP_NO);
}

void ViewPaulAwakes::loop() {
	if (timer.isDue_ms(150)) {
		display.waitUntilLCDReady();
		display.setDefaultBitmapPage();
		uint8_t eyes_x = PAUL_EYES_X;
		switch (blinkStep) {
			case 0:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BITMAP_NO);
				break;
			case 1:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_HALF_BLINK_BITMAP_NO);
				break;
			case 2:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_FULL_BLINK_BITMAP_NO);
				break;
			case 3:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_HALF_BLINK_BITMAP_NO);
				break;
			case 6:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BITMAP_NO);
				break;
			case 7:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_HALF_BLINK_BITMAP_NO);
				break;
			case 8:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_FULL_BLINK_BITMAP_NO);
				break;
			case 9:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_HALF_BLINK_BITMAP_NO);
				break;
			case 10:
			case 11:
			case 12:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BITMAP_NO);
				break;
			case 13:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_LEFT_EYES_BITMAP_NO);
				break;
			case 14:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BITMAP_NO);
				break;
			case 15:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_RIGHT_EYES_BITMAP_NO);
				break;
			case 18:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BITMAP_NO);
				break;
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BUT_SAD_BITMAP_NO);
				break;
			default:
				lcd.displayFlashPicture(eyes_x,PAUL_EYES_Y,PAULS_EYES_BUT_SAD_BITMAP_NO);
				break;
			} // switch 
				
		// increase blink step
		if (blinkTimer.isDue_ms(150)) {
			blinkStep++;
			if (blinkStep > 26)
				blinkStep = 0;
		}
			
		// display text
		// text changed?
		if (display.isSpokenTextAvail() && (textLen != display.getSpokenTextLen())) {
			int16_t x,y;
			lcd.setTextFont(FONT_DEFAULT);
			lcd.drawRectf(220,20,LCD_WIDTH-1,LCD_HEIGHT,BLACK_NO);
			lcd.writeSentenceInArea(220,20,LCD_WIDTH-1,LCD_HEIGHT,display.getSpokenText(),x,y);
			textLen = display.getSpokenTextLen();
			speechTimer.setDueTime(0);
		}						
		
		// rmeove the text after a while			
		if ((textLen > 0) && speechTimer.isDue_ms(3000+textLen*40)) {
			lcd.drawRectf(220,20,LCD_WIDTH-1,LCD_HEIGHT,BLACK_NO);
			display.spokenTextDone();
			textLen = 0;
		}
	} // if timer
}			
		
		 void ViewPaulAwakes::touchEvent(uint8_t pTouchId) {
			switch (pTouchId) {
				case TOUCH_AREA_SPLASH:
					lcd.clear();
					display.activateView(ViewOptionsPaulNo);
					break;
				default:
					Serial.print(F("unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}			
