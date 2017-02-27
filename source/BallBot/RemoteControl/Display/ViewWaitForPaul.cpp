/*
 * ViewWaitForPaul.cpp
 *
 * Created: 04.04.2013 12:43:09
 *  Author: JochenAlt
 */ 


#include "DisplayController.h"
#include "ViewWaitForPaul.h"
#include "eDIP-TFT-flash.h"
#include "eDIP-TFT.h"

void ViewWaitForPaul::startup() {
	display.waitUntilLCDReady();
	lcd.clear();
	alpha = PI/2.0;
	lcd.setNoTransparency();
	lcd.drawRectf(1,220,120,239,BLACK_NO);
		
	display.deleteAllTouchAreas();
	display.addTouchArea(TOUCH_AREA_WAIT_FOR_PAUL,0,0,LCD_WIDTH,LCD_HEIGHT);
}
		

void ViewWaitForPaul::loop() {
	if (timer.isDue_ms(30)) {
		int8_t d = cos(alpha)*50; 
		uint8_t x = 130 + d;
		display.setThemeBitmapPage();
		if (d < -30)
			lcd.displayFlashPicture(x,50,PAUL_RIGHT_TILT_BITMAP_NO);
		else 
			if (d > 30)
					lcd.displayFlashPicture(x,50,PAUL_LEFT_TILT_BITMAP_NO);
			else
				if (d< -15)
					lcd.displayFlashPicture(x,50,PAUL_HALF_RIGHT_TILT_BITMAP_NO);
				else 
					if (d > 15)
						lcd.displayFlashPicture(x,50,PAUL_HALF_LEFT_TILT_BITMAP_NO);
					else
						lcd.displayFlashPicture(x,50,PAUL_BITMAP_NO);
		alpha += 0.04;
	}						
}			
		
void ViewWaitForPaul::touchEvent(uint8_t pTouchId) {
	switch (pTouchId) {
		case TOUCH_AREA_WAIT_FOR_PAUL:
			lcd.clear();
			display.activateView(ViewOptionsPaulNo);
			break;
		default:
			Serial.print(F("unknown touch area "));
			Serial.println(pTouchId);
			break;
	}
}			
