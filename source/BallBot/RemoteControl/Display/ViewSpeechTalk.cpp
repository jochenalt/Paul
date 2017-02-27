/*
 * ViewSpeechTalk.cpp
 *
 * Created: 03.04.2013 22:57:40
 *  Author: JochenAlt
 */ 


#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"
#include "DisplayController.h"
#include "ViewSpeechTalk.h"
#include "RemoteMemory.h"

#define TEXTSCROLLAREA_X1 60 
#define TEXTSCROLLAREA_Y1 70
#define TEXTSCROLLAREA_X2 LCD_WIDTH-1 
#define TEXTSCROLLAREA_Y2 170

#define TEXTTYPING_AREA_X1 20 
#define TEXTTYPING_AREA_Y1 190
#define TEXTTYPING_AREA_X2 LCD_WIDTH-20
#define TEXTTYPING_AREA_Y2 LCD_HEIGHT-1
			
		void ViewSpeechTalk::startup() {
			display.waitUntilLCDReady();
			lcd.clear();
			display.deleteAllTouchAreas();
			char* windowTitle[] = {"Talk","Poetry","Song"};
			uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_SPEECH_POEMS,TOUCH_AREA_SPEECH_SONGS};				
			drawTabbedWindowTitle("Speech",windowTitle,touchAreaID,3);
			
			// display text 
			sayIt = false; // dont say the following line
			scrollingArea[0] = 0;
			strcpy_P(typingBuffer,PSTR("Tell me what you want me to say."));
			addChar(13);
			sayIt = true; // say everything typed in from now
			
 			displayScrollingArea();
			displayTypingArea();
			lcd.setLineColor(PASSIVE_TEXT_COLOR_NO,BLACK_NO);
			lcd.drawLine(TEXTTYPING_AREA_X1,TEXTTYPING_AREA_Y1-3,200,TEXTTYPING_AREA_Y1-3);
		}


		int8_t ViewSpeechTalk::searchCharInString(char c, char* str) {
			char* pos = strchr(str,c);
			if (pos != NULL) 
				return pos-str;
			return -1;
		}
				
		void ViewSpeechTalk::sendTalkRequestToPaul(char* pText) {
			if (sayIt)
				display.linkToPaulPtr->callSpeechRequest(memory.persistentMem.voice,NoPoem, NoSong,pText);
		}		
		
		void ViewSpeechTalk::addChar(char c) {

			// esc deletes the complete typing buffer
			if (c == 27) {
				typingBuffer[0] = 0;
				return;
			}
			Serial.print(c);
			// <CR>, evict one line from the scrolling area, if we already have three lines
			if ((c == char(13)) || (c == char(10))) {
				Serial.println();
				Serial.print(F("line>"));

				if (scrollingLines >= 4) {
					int8_t pos = searchCharInString(10,scrollingArea);
					if (pos >= 0) {
						int i = pos+1;
						int len = strlen(scrollingArea);
						for (; i<SCROLLING_BUFFER_SIZE;i++) {
							char c = scrollingArea[i];
							scrollingArea[i-pos-1] = c;
						}							
						scrollingArea[len-pos] = 0;	
						scrollingLines--;
					}	
				}
				// add typing area to scrolling area and add a line feed
				uint8_t scrollAreaLen = strlen(scrollingArea);
				uint8_t typingAreaLen = strlen(typingBuffer);
				for (int i = 0;i<typingAreaLen;i++)
					scrollingArea[scrollAreaLen++] = typingBuffer[i];
				scrollingArea[scrollAreaLen++] = char(10);
				scrollingArea[scrollAreaLen] = 0;
				displayScrollingArea();
				
				// send request to Paul
				sendTalkRequestToPaul(typingBuffer);
				
				// delete typing buffer
				typingBuffer[0] = 0;
				scrollingLines++;
			}
			else if ((c == 8)) { // backspace
				Serial.print(' ');
				Serial.print(c);
				uint8_t typingAreaLen = strlen(typingBuffer);
				if (typingAreaLen == 0)
					lcd.errorBeep();
				else
					typingBuffer[typingAreaLen-1] = 0; 
			} else if ((c>=32) && (c<=255))	{
				uint8_t typingAreaLen = strlen(typingBuffer);
				if (typingAreaLen >= TYPING_BUFFER_SIZE)
					lcd.errorBeep();
				else {
					typingBuffer[typingAreaLen] = c;	
					typingBuffer[typingAreaLen+1] = 0;	

				}
			}			
			displayTypingArea();
		}

		void ViewSpeechTalk::displayTypingArea() {
			lcd.setTextFont(FONT_GRANT);
			lcd.setTextColor(PASSIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.deleteRect(TEXTTYPING_AREA_X1,TEXTTYPING_AREA_Y1,TEXTTYPING_AREA_X2,TEXTTYPING_AREA_Y2);
			int x,y;
			lcd.writeSentenceInArea(TEXTTYPING_AREA_X1,TEXTTYPING_AREA_Y1,TEXTTYPING_AREA_X2,TEXTTYPING_AREA_Y2,typingBuffer,x,y);
		}			

		void ViewSpeechTalk::displayScrollingArea() {
			lcd.deleteRect(TEXTSCROLLAREA_X1,TEXTSCROLLAREA_Y1,TEXTSCROLLAREA_X2,TEXTSCROLLAREA_Y2);			
			lcd.setTextFont(FONT_DEFAULT);
			lcd.setTextColor(ACCENT_COLOR1_NO,BACKGROUND_COLOR_NO);
			int x,y;
			lcd.writeSentenceInArea(TEXTSCROLLAREA_X1,TEXTSCROLLAREA_Y1,TEXTSCROLLAREA_X2,TEXTSCROLLAREA_Y2,scrollingArea,x,y);
		}		
				
		void ViewSpeechTalk::touchEvent(uint8_t pTouchId) {
			switch (pTouchId) {
				case TOUCH_AREA_BACK:
					display.activateView(ViewMainNo);
					break;
				case TOUCH_AREA_SPEECH_SONGS:
					display.activateView(ViewSpeechSongsNo);
					break;
				case TOUCH_AREA_SPEECH_POEMS:
					display.activateView(ViewSpeechPoemsNo);
					break;
				default:
					Serial.print(F("unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}			
