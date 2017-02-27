/*
 * ViewOptionsPaul.cpp
 *
 * Created: 03.04.2013 23:02:53
 *  Author: JochenAlt
 */ 



#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "DisplayController.h"
#include "ViewOptionsPaul.h"
#include "RemoteMemory.h"

		void ViewOptionsPaul::startup() {
			display.waitUntilLCDReady();			
			lcd.terminalOn(false);
			lcd.clear();
			display.deleteAllTouchAreas();
			char* windowTitle[] = {"Paul","Config", "Remote"};
			uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_OPTIONS_CONFIG, TOUCH_AREA_OPTIONS_REMOTE};				
			drawTabbedWindowTitle("Options",windowTitle,touchAreaID,3);

			// display the volume 
			lcd.setTransparencyByCorner();
			drawTouchArrowUp(TOUCH_AREA1_UP,20,80);
			drawTouchArrowDown(TOUCH_AREA1_DOWN,20,180);
			lcd.displayFlashPicture(0,134,LOUDSPEAKER_BITMAP_NO);

			// display talking speed
			lcd.displayFlashPicture(80,137,TALKING_SPEED_BITMAP_NO);
			drawTouchArrowUp(TOUCH_AREA2_UP,100,80);
			drawTouchArrowDown(TOUCH_AREA2_DOWN,100,180);
			
			display.waitUntilLCDReady();
			display.addTouchArea(TOUCH_AREA_LANGUAGE,180-1,100-1,LCD_WIDTH-1,100+30);
			display.addTouchArea(TOUCH_AREA_VOICE,180-1,160-1,LCD_WIDTH-1,160+30);
			
			display.waitUntilLCDReady();
			
			displayState();
			changeSentToPaul = true;
			speechRateChange = false;
			volumeChange = false;
			numberOn = false;
		}
		
		void ViewOptionsPaul::displayLanguageVoice() {
			if (numberOn == false) {
				lcd.deleteRect(170,100,LCD_WIDTH-1,LCD_HEIGHT-1);
				lcd.setTextFont(FONT_LARGE);			
				lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BLACK_NO);

				lcd.drawText(180,100,'L',getLangDescription(memory.persistentMem.language));
				lcd.drawText(180,160,'L',getVoiceDescription(memory.persistentMem.voice));

				lcd.setTextFont(FONT_DEFAULT);
				lcd.setTextColor(PASSIVE_TEXT_COLOR_NO, BLACK_NO);
				lcd.drawText(180,125,'L',"Select its accent");
				lcd.drawText(180,185,'L',"Select its voice");	
			}
		}
		
		void ViewOptionsPaul::displayState() {			
			displayLanguageVoice();

			display.waitUntilLCDReady();
			
			// draw volume slider
			lcd.setLineColor(WHITE_NO, BLACK_NO);
			lcd.drawRect(40,122,48,175);
			lcd.drawRectf(40+1,122+1,48-1, 122+1+ (175-122)*(256-memory.persistentMem.volume)/256, BLACK_NO);
			lcd.drawRectf(40+1,122+1 + (175-122)*(256-memory.persistentMem.volume)/256,48-1,175-1, ACCENT_COLOR1_NO);
			
			// draw speed slider
			lcd.setLineColor(WHITE_NO, BLACK_NO);
			lcd.drawRect(120,122,128,175);
			lcd.drawRectf(120+1,122+1,128-1, 122+1+ (175-122)*(256-memory.persistentMem.speechRate)/256, BLACK_NO);
			lcd.drawRectf(120+1,122+1 + (175-122)*(256-memory.persistentMem.speechRate)/256,128-1,175-1, ACCENT_COLOR2_NO);
		}

		void ViewOptionsPaul::loop() {
			// send changed to Paul once a second
			if (display.linkIsOn) {
				if (changeSentToPaul) {
					if (sendToPaulTimer.isDue_ms(1000)) {
						display.linkToPaulPtr->callSendOption(memory.persistentMem.volume, memory.persistentMem.voice, memory.persistentMem.language, memory.persistentMem.speechRate);
						memory.delayedSave(10000);
						changeSentToPaul = false;
					}					
				}
			}
			
			if (showNumberTimer.isDue_ms(3000)) {
				lcd.deleteRect(180,100,LCD_WIDTH-1,LCD_HEIGHT-1);
				numberOn = false;
				displayLanguageVoice();
			}				
		}
		
		void ViewOptionsPaul::displaySliderChange() {
			char buffer[5];		
			showNumberTimer.setDueTime(0);
			numberOn = true;
			display.waitUntilLCDReady();			
			lcd.deleteRect(180,100,LCD_WIDTH-1,LCD_HEIGHT-1);
			lcd.setTextColor(PASSIVE_TEXT_COLOR_NO,BLACK_NO);
			lcd.setTextFont(FONT_DEFAULT);
						
			if (volumeChange) {
				uint16_t shownVolume = ((uint16_t)memory.persistentMem.volume) *100/256;
				sprintf_P(buffer,PSTR("%d  "),shownVolume);
				lcd.drawText(180,170,'L',F("current volume"));
			}
			if (speechRateChange) {
				uint16_t shownSpeed = ((uint16_t)memory.persistentMem.speechRate) *100/256;
				sprintf_P(buffer,PSTR("%d  "),shownSpeed);
				lcd.drawText(180,170,'L',F("current speech speed"));
			}
						
			if (volumeChange || speechRateChange) {		
				lcd.setTextFont(FONT_GIANT);
				lcd.drawText(180,120,'L',buffer);
			}		
			volumeChange = false;
			speechRateChange = false;
		}
		
		void ViewOptionsPaul::touchEvent(uint8_t pTouchId) {
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
				case TOUCH_AREA_OPTIONS_REMOTE:
					display.activateView(ViewOptionsRemoteNo);
					break;
				case TOUCH_AREA1_UP:
					if (memory.persistentMem.volume <=255-5 ) {
						memory.persistentMem.volume = memory.persistentMem.volume + 5;
						changeSentToPaul = true;
						volumeChange = true;
						displaySliderChange();
						displayState();
					}					
					else {
						memory.persistentMem.volume = 255;
						volumeChange = true;
						displaySliderChange();
						displayState();
						lcd.errorBeep();
					}						
					break;
				case TOUCH_AREA1_DOWN:
					if (memory.persistentMem.volume >5 ) {
						memory.persistentMem.volume = memory.persistentMem.volume -5;
						changeSentToPaul = true;
						volumeChange = true;
						displaySliderChange();
						displayState();
					}						
					else {
						lcd.errorBeep();
						volumeChange = true;
						memory.persistentMem.volume = 0;
						displaySliderChange();
						displayState();
					}						
					break;
				case TOUCH_AREA2_UP:
					if (memory.persistentMem.speechRate <=255-5 ) {
						memory.persistentMem.speechRate = memory.persistentMem.speechRate + 5;
						changeSentToPaul = true;
						speechRateChange = true;
						displaySliderChange();
						displayState();
					}					
					else {
						memory.persistentMem.speechRate = 256;
						speechRateChange = true;
						displaySliderChange();
						displayState();
						lcd.errorBeep();
					}						
					break;
				case TOUCH_AREA2_DOWN:
					if (memory.persistentMem.speechRate >50 ) {
						memory.persistentMem.speechRate = memory.persistentMem.speechRate -5;
						speechRateChange = true;
						changeSentToPaul = true;
						displaySliderChange();
						displayState();
					}						
					else {
						lcd.errorBeep();
						memory.persistentMem.speechRate = 50;
						speechRateChange = true;
						displaySliderChange();
						displayState();
					}						
					break;

				case TOUCH_AREA_LANGUAGE:
					memory.persistentMem.language = (LanguageType)((int)memory.persistentMem.language + 1);
					if (memory.persistentMem.language == NUMBER_OF_LANGUAGES)
						memory.persistentMem.language = (LanguageType)0;
					changeSentToPaul = true;
					displayState();
					break;
				case TOUCH_AREA_VOICE:
					memory.persistentMem.voice = (VoiceType)((int)memory.persistentMem.voice + 1);

					if (memory.persistentMem.voice == NUMBER_OF_VOICES)
						memory.persistentMem.voice = (VoiceType)0;
					changeSentToPaul = true;
					displayState();
					break;
				default:
					Serial.print(F("unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}
