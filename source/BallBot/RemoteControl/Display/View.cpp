/*
 * View.cpp
 *
 * Created: 28.03.2013 13:46:19
 *  Author: JochenAlt
 */ 

#include "DisplayController.h"
#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"


void View::drawTabbedWindowTitle(char pTopTitle[], char* pWindowTitle[], uint8_t pTouchAreaId[], uint8_t pTitles) {
	uint16_t x = 0;
	uint8_t y = 0;
	uint16_t width;
	uint8_t height;

	if (pTopTitle != NULL) {
		lcd.setTextFont(FONT_SMALL);
		lcd.setTextColor(PASSIVE_TEXT_COLOR_NO, BLACK_NO);
		height = lcd.getCurrentFontHeight();

		lcd.drawText(x,y,'L',pTopTitle);
		x = 0;
		y = height;
	}
			
	uint8_t titleNo = 0;
	boolean nextTitle = true;
	while (nextTitle) {
		if (titleNo == 0) {
			lcd.setTextFont(FONT_GIANT);
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
		}
		else {
			lcd.setTextFont(FONT_HUGE);
			lcd.setTextColor(PASSIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
		}				
		char* title = pWindowTitle[titleNo];
		width = lcd.getStringWidth(title);
		height = lcd.getCurrentFontHeight();
		// draw the title			
		lcd.drawText(x,y,'L',title);
		// define touch areas of tabbed paging
		if (pTouchAreaId[titleNo] != NO_TOUCH_AREA)
			display.addTouchArea(pTouchAreaId[titleNo],x,y,x+width, y+height);
		x += width;
		x += lcd.getStringWidth(" ");				
		if (x>=LCD_WIDTH) 
			nextTitle = false;

		if (titleNo == 0)
			y += lcd.getFontHeight(FONT_GIANT) - lcd.getFontHeight(FONT_HUGE);
		
		// next title or quit the loop
		titleNo++;
		if (titleNo>=pTitles)
			nextTitle = false;; 
	} 
}

void View::drawTouchArrowUp(int pTouchId, int x, int y) {
	lcd.displayFlashPicture(x,y,ARROW_UP_BITMAP_NO);
	display.addTouchArea(pTouchId,x,y,x+50,y+40);
}
void View::drawTouchArrowDown(int pTouchId, int x, int y) {
	lcd.displayFlashPicture(x,y,ARROW_DOWN_BITMAP_NO);
	display.addTouchArea(pTouchId,x,y,x+50,y+40);	
}
