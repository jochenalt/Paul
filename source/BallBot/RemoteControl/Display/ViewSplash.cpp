/*
 * ViewSplash.cpp
 *
 * Created: 04.04.2013 12:39:13
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "ViewSplash.h"
#include "DisplayController.h"
#include "TimePassedBy.h"

void ViewSplash::startup() {
	// display Splash
	display.waitUntilLCDReady();
	lcd.clear();
	lcd.setTextFont(FONT_DEFAULT);
	lcd.drawText(1,220,'L',"It's just me, Paul.");	
	lcd.setNoTransparency();
	x = 130+160;
	y = 50+80;
	display.setThemeBitmapPage();
	lcd.displayFlashPicture(x,y,PAUL_BITMAP_NO);
	display.deleteAllTouchAreas();
	display.addTouchArea(TOUCH_AREA_SPLASH,0,0,LCD_WIDTH,LCD_HEIGHT);
}
		
boolean ViewSplash::isFinished() {
	return ((abs(130-x)<3) && (abs(50-y)<3));
}

void ViewSplash::loop() {
	if (timer.isDue_ms(20)) {
		x--;
		x--;
		y--;
		display.setThemeBitmapPage();		
		lcd.displayFlashPicture(x,y,PAUL_BITMAP_NO);
		display.waitUntilLCDReady();

	}
}			

void ViewSplash::touchEvent(uint8_t pTouchId) {
	switch (pTouchId) {
		case TOUCH_AREA_SPLASH:
			lcd.clear();
			display.activateView(ViewWaitForPaulNo);
			break;
		default:
			Serial.print(F("unknown touch area "));
			Serial.println(pTouchId);
			break;
	}
}			
