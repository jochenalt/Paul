/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEW_OPTIONS_CONFIG_H_
#define VIEW_OPTIONS_CONFIG_H_

#include "View.h"
#include "DisplayController.h"

class ViewOptionsConfig : public View {
	
	public:
		ViewOptionsConfig() {
		}			
		virtual void startup() {
			display.waitUntilLCDReady();
			lcd.clear();
			display.deleteAllTouchAreas();
			char* windowTitle[] = {"Config","Paul","Remote"};
			uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_OPTIONS_PAUL, TOUCH_AREA_OPTIONS_REMOTE};				
			drawTabbedWindowTitle("Options",windowTitle,touchAreaID,3);
			lcd.setTerminalColor(WHITE_NO, DARK_GREY);
			lcd.defineTerminal(false,0,75,LCD_WIDTH-1,LCD_WIDTH-1);
			lcd.terminalOn(true);
			lcd.clearTerminal();
		}
		
		void teardown() {
			lcd.terminalOn(false);
		}

		virtual void loop() {
		}
		
		void addKey(char c) {
			char str[] = { c,0 };
			lcd.sendTextToTerminal(str);
		}
		void addKey(char* str) {
			lcd.sendTextToTerminal(str);
		}

		virtual void touchEvent(uint8_t pTouchId) {
			switch (pTouchId) {
				case TOUCH_AREA_BACK:
					display.activateView(ViewMainNo);
					break;
				case TOUCH_AREA_OPTIONS_PAUL:
					display.activateView(ViewOptionsPaulNo);
					break;
				case TOUCH_AREA_OPTIONS_REMOTE:
					display.activateView(ViewOptionsRemoteNo);
					break;
				case TOUCH_AREA_OPTIONS_CONFIG:
					display.activateView(ViewOptionsConfigNo);
					break;
				default:
					Serial.print(F("unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}			
	private:
};



#endif /* VIEW_OPTIONS_CONFIG_H_ */