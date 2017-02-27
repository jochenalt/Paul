/*
 * ViewOptionsPaul.cpp
 *
 * Created: 03.04.2013 23:02:53
 *  Author: JochenAlt
 */ 



#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "DisplayController.h"
#include "ViewOptionsRemote.h"
#include "RemoteMemory.h"

#define THEME_X 140
#define THEME_Y 90
#define THEME_WIDTH 50
#define THEME1_X THEME_X
#define THEME1_Y THEME_Y
#define THEME2_X (THEME_X + THEME_WIDTH)
#define THEME2_Y THEME_Y
#define THEME3_X (THEME_X + THEME_WIDTH + THEME_WIDTH)
#define THEME3_Y (THEME_Y)
#define THEME4_X (THEME_X)
#define THEME4_Y (THEME_Y + THEME_WIDTH)
#define THEME5_X (THEME_X + THEME_WIDTH)
#define THEME5_Y (THEME_Y + THEME_WIDTH)
#define THEME6_X (THEME_X + THEME_WIDTH + THEME_WIDTH)
#define THEME6_Y (THEME_Y + THEME_WIDTH)


void ViewOptionsRemote::startup() {
	display.waitUntilLCDReady();			
	lcd.terminalOn(false);
	lcd.clear();
	display.deleteAllTouchAreas();
	char* windowTitle[] = {"Remote","Paul", "Speech"};
	uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_OPTIONS_PAUL, TOUCH_AREA_SPEECH_POEMS};				
	display.waitUntilLCDReady();			
	drawTabbedWindowTitle("Options",windowTitle,touchAreaID,3);
	// display the volume 
	lcd.setTransparencyByCorner();
	drawTouchArrowUp(TOUCH_AREA1_UP,20,80);
	drawTouchArrowDown(TOUCH_AREA1_DOWN,20,180);
	lcd.displayFlashPicture(0,134,BRIGHTNESS_BITMAP_NO);
	
	// draw 1. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME1_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME1_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME1_X+1, THEME1_Y+1, THEME1_X+THEME_WIDTH-2, THEME1_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_1,THEME1_X, THEME1_Y, THEME1_X+THEME_WIDTH, THEME1_Y+THEME_WIDTH);
	// draw 2. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME2_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME2_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME2_X+1, THEME2_Y+1, THEME2_X+THEME_WIDTH-2, THEME2_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_2,THEME2_X, THEME2_Y, THEME2_X+THEME_WIDTH, THEME2_Y+THEME_WIDTH);
	// draw 3. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME3_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME3_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME3_X+1, THEME3_Y+1, THEME3_X+THEME_WIDTH-2, THEME3_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_3,THEME3_X, THEME3_Y, THEME3_X+THEME_WIDTH, THEME3_Y+THEME_WIDTH);
	// draw 4. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME4_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME4_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME4_X+1, THEME4_Y+1, THEME4_X+THEME_WIDTH-2, THEME4_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_4,THEME4_X, THEME4_Y, THEME4_X+THEME_WIDTH, THEME4_Y+THEME_WIDTH);
	// draw 5. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME5_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME5_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME5_X+1, THEME5_Y+1, THEME5_X+THEME_WIDTH-2, THEME5_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_5,THEME5_X, THEME5_Y, THEME5_X+THEME_WIDTH, THEME5_Y+THEME_WIDTH);
	// draw 6. theme
	lcd.assignColor(THEME_FIRST_COLOR_NO, THEME6_FIRST_RGB_COLOR);
	lcd.assignColor(THEME_SECOND_COLOR_NO, THEME6_SECOND_RGB_COLOR);
	lcd.drawRectf(THEME6_X+1, THEME6_Y+1, THEME6_X+THEME_WIDTH-2, THEME6_Y+THEME_WIDTH-2, THEME_FIRST_COLOR_NO);
	display.addTouchArea(TOUCH_AREA_THEME_6,THEME6_X, THEME6_Y, THEME6_X+THEME_WIDTH, THEME6_Y+THEME_WIDTH);
		
	displayState();
	anyChange = false;	
}
		
void ViewOptionsRemote::displayState() {
	display.waitUntilLCDReady();
		
	// draw brightness slider
	lcd.setLineColor(WHITE_NO, BLACK_NO);
	lcd.drawRect(40,122,48,175);
	lcd.drawRectf(40+1,122+1,48-1, 122+1+ (175-122)*(100-memory.persistentMem.displayBrightness)/100, BLACK_NO);
	lcd.drawRectf(40+1,122+1 + (175-122)*(100-memory.persistentMem.displayBrightness)/100,48-1,175-1, ACCENT_COLOR1_NO);
	
		
	// switch off button
	lcd.setTextFont(FONT_DEFAULT);
	lcd.setTextColor(ACCENT_COLOR1_NO, BLACK_NO);
	lcd.drawText(THEME_X, THEME_Y+THEME_WIDTH*2+lcd.getFontHeight(FONT_LARGE)+2, 'L', F("switch off remote"));	
}

void ViewOptionsRemote::loop() {
	if (anyChange) {
		memory.delayedSave(10000);
		anyChange = false;
	}
}
		
void ViewOptionsRemote::touchEvent(uint8_t pTouchId) {
	switch (pTouchId) {
		case TOUCH_AREA_BACK:
			display.activateView(ViewMainNo);
			break;
		case TOUCH_AREA_OPTIONS_PAUL:
			display.activateView(ViewOptionsPaulNo);
			break; 	
		case TOUCH_AREA_OPTIONS_CONFIG:
			display.activateView(ViewOptionsConfigNo);
			break;
		case TOUCH_AREA_SPEECH_POEMS:
			display.activateView(ViewSpeechPoemsNo);
			break;
		case TOUCH_AREA1_UP:
			if (memory.persistentMem.displayBrightness <100 ) {
				memory.persistentMem.displayBrightness = memory.persistentMem.displayBrightness + 10;
				lcd.setBrightness(memory.persistentMem.displayBrightness);
				anyChange = true;
			}					
			else {
				memory.persistentMem.displayBrightness = 100;
				lcd.errorBeep();
			}				
			displayState();		
			break;
		case TOUCH_AREA1_DOWN:
			if (memory.persistentMem.displayBrightness >30) {
				memory.persistentMem.displayBrightness = memory.persistentMem.displayBrightness-10;
				lcd.setBrightness(memory.persistentMem.displayBrightness);
				anyChange = true;
			}						
			else {
				lcd.errorBeep();
				memory.persistentMem.displayBrightness = 30;
			}						
			displayState();		
			break;
		case TOUCH_AREA_THEME_1:
			memory.persistentMem.displayTheme = 0;
			display.setTheme();
			anyChange = true;
			displayState();
			break;	
		case TOUCH_AREA_THEME_2:
			memory.persistentMem.displayTheme = 1;
			display.setTheme();
			displayState();
			anyChange = true;
			break;	
		case TOUCH_AREA_THEME_3:
			memory.persistentMem.displayTheme = 2;
			display.setTheme();
			displayState();
			anyChange = true;
			break;	
		case TOUCH_AREA_THEME_4:
			memory.persistentMem.displayTheme = 3;
			display.setTheme();
			displayState();
			anyChange = true;
			break;	
		case TOUCH_AREA_THEME_5:
			memory.persistentMem.displayTheme = 4;
			display.setTheme();
			displayState();
			anyChange = true;
			break;	
		case TOUCH_AREA_THEME_6:
			memory.persistentMem.displayTheme = 5;
			display.setTheme();
			displayState();
			anyChange = true;
			break;	
			
		default:
			Serial.print(F("unknown touch area "));
			Serial.println(pTouchId);
			break;
	}
}
