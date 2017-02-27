/*
 * ViewSpeechPoems.cpp
 *
 * Created: 03.04.2013 23:18:08
 *  Author: JochenAlt
 */ 

#include "eDIP-TFT.h"
#include "ViewSpeechPoems.h"
#include "DisplayController.h"
#include "eDip-TFT.h"
#include "eDip-TFT-flash.h"
#include "RemoteMemory.h"
			
		void ViewSpeechPoems::startup() {
			display.waitUntilLCDReady();
			lcd.clear();
			display.deleteAllTouchAreas();
			char* windowTitle[] = {"Poetry","Talk","Song"};
			uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_SPEECH_TALK,TOUCH_AREA_SPEECH_SONGS};				
			drawTabbedWindowTitle("Speech",windowTitle,touchAreaID,3);

			
#define LISTBOX_POEMS_X	120		
#define LISTBOX_POEM1_Y	90		
#define LISTBOX_POEM2_Y	165		

			// display the list of poems
			lcd.setTransparencyByCorner();
		
			drawTouchArrowUp(TOUCH_AREA1_UP,20,70);
			drawTouchArrowDown(TOUCH_AREA1_DOWN,20,170);
			
			display.addTouchArea(TOUCH_AREA_POEMS1,LISTBOX_POEMS_X-1,LISTBOX_POEM1_Y-1,LCD_WIDTH-1,LISTBOX_POEM2_Y-1);
			display.addTouchArea(TOUCH_AREA_POEMS2,LISTBOX_POEMS_X-1,LISTBOX_POEM2_Y-1,LCD_WIDTH-1,LCD_HEIGHT-1);
			
			displayPoemList();
		}

		void ViewSpeechPoems::sendPoemRequestToPaul(PoemType pPoemNo) {
			if (display.linkIsOn) {
				display.linkToPaulPtr->callSpeechRequest(memory.persistentMem.voice, pPoemNo, NoSong,NULL);
			}			
		}		

		void ViewSpeechPoems::displayPoemList() {
			int x,y;
			lcd.deleteRect(LISTBOX_POEMS_X,LISTBOX_POEM1_Y,LCD_WIDTH-1,LCD_HEIGHT-1);
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_LARGE);
			lcd.drawText(LISTBOX_POEMS_X,LISTBOX_POEM1_Y,'L',getPoemName(poem));
			lcd.setTextColor(ACCENT_COLOR1_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_DEFAULT);
			lcd.writeSentenceInArea(LISTBOX_POEMS_X,LISTBOX_POEM1_Y + lcd.getFontHeight(FONT_LARGE) +5,
									LCD_WIDTH-1,LISTBOX_POEM1_Y+lcd.getFontHeight(FONT_LARGE) +8 + lcd.getFontHeight(FONT_DEFAULT)*2,
									getPoemDescription(poem),x,y);

			uint8_t poem2No = (int)poem + 1;
			if (poem2No > NUMBER_OF_POEMS) {
				poem2No = 1;
			} 
			
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_LARGE);
			lcd.drawText(LISTBOX_POEMS_X,LISTBOX_POEM2_Y,'L',getPoemName((PoemType)poem2No));
			lcd.setTextColor(ACCENT_COLOR1_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_DEFAULT);
			lcd.writeSentenceInArea(LISTBOX_POEMS_X, LISTBOX_POEM2_Y + lcd.getFontHeight(FONT_LARGE) +5,
									LCD_WIDTH-1,LISTBOX_POEM2_Y+lcd.getFontHeight(FONT_LARGE) +8 + lcd.getFontHeight(FONT_DEFAULT)*2,
									getPoemDescription((PoemType)poem2No),x,y);
			
		}
		void ViewSpeechPoems::loop() {
		}
		
		void ViewSpeechPoems::touchEvent(uint8_t pTouchId) {
			switch (pTouchId) {
				case TOUCH_AREA_BACK:
					display.activateView(ViewMainNo);
					break;
				case TOUCH_AREA_SPEECH_SONGS:
					display.activateView(ViewSpeechSongsNo);
					break;
				case TOUCH_AREA_SPEECH_TALK:
					display.activateView(ViewSpeechTalkNo);
					break;
				case TOUCH_AREA1_UP:
					poem = (PoemType)(poem - 1);
					if (poem == 0)
						poem = (PoemType)NUMBER_OF_POEMS;
					displayPoemList();
					break;
				case TOUCH_AREA1_DOWN: {
						uint8_t poemNo = (int)poem + 1;
						if (poemNo > NUMBER_OF_POEMS) {
							poem = (PoemType)1;
						} else
							poem = (PoemType)poemNo;						
						displayPoemList();	
					}								
					break;
				case TOUCH_AREA_POEMS1:
					sendPoemRequestToPaul(poem);
					break;
				case TOUCH_AREA_POEMS2: {
						uint8_t p = (int)poem + 1;
						if (p > NUMBER_OF_POEMS)
							p = (PoemType)1;

						sendPoemRequestToPaul((PoemType )p);
					}					
					break;
				default:
					Serial.print(F("poems:unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}	