/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 



#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"
#include "DisplayController.h"
#include "Paul.h"
#include "ViewSpeechSongs.h"
#include "RemoteMemory.h"

void ViewSpeechSongs::startup() {
			display.waitUntilLCDReady();
			lcd.clear();
			display.deleteAllTouchAreas();
			char* windowTitle[] = {"Song","Talk","Poetry"};
			uint8_t touchAreaID[] = {TOUCH_AREA_BACK,TOUCH_AREA_SPEECH_TALK,TOUCH_AREA_SPEECH_POEMS};				
			drawTabbedWindowTitle("Speech",windowTitle,touchAreaID,3);
#define LISTBOX_SONG_X	120		
#define LISTBOX_SONG1_Y	90		
#define LISTBOX_SONG2_Y	165		

			// display the list of songs
			lcd.setTransparencyByCorner();
			drawTouchArrowUp(TOUCH_AREA1_UP,20,70);
			drawTouchArrowDown(TOUCH_AREA1_DOWN,20,170);
			
			display.addTouchArea(TOUCH_AREA_SONGS1,LISTBOX_SONG_X-1,LISTBOX_SONG1_Y-1,LCD_WIDTH-1,LISTBOX_SONG2_Y-1);
			display.addTouchArea(TOUCH_AREA_SONGS2,LISTBOX_SONG_X-1,LISTBOX_SONG2_Y-1,LCD_WIDTH-1,LCD_HEIGHT-1);
						
			displaySongList();
		}

	void ViewSpeechSongs::sendSongRequestToPaul(SongType pSongNo) {
		display.linkToPaulPtr->callSpeechRequest(memory.persistentMem.voice, NoPoem ,pSongNo,NULL);
	}		

	void ViewSpeechSongs::displaySongList() {
			int x,y;
			lcd.deleteRect(LISTBOX_SONG_X,LISTBOX_SONG1_Y,LCD_WIDTH-1,LCD_HEIGHT-1);
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_LARGE);
			lcd.drawText(LISTBOX_SONG_X,LISTBOX_SONG1_Y,'L',getSongName(firstSongInList));
			lcd.setTextColor(ACCENT_COLOR1_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_DEFAULT);
			lcd.writeSentenceInArea(LISTBOX_SONG_X,LISTBOX_SONG1_Y + lcd.getFontHeight(FONT_LARGE) +5,
									LCD_WIDTH-1,LISTBOX_SONG1_Y+lcd.getFontHeight(FONT_LARGE) +8 + lcd.getFontHeight(FONT_DEFAULT)*2,
									getSongDescription(firstSongInList),x,y);

			uint8_t song2No = (int)firstSongInList + 1;
			if (song2No > NUMBER_OF_SONGS) {
				song2No = 1;
			} 
			
			lcd.setTextColor(ACTIVE_TEXT_COLOR_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_LARGE);
			lcd.drawText(LISTBOX_SONG_X,LISTBOX_SONG2_Y,'L',getSongName((SongType)song2No));
			lcd.setTextColor(ACCENT_COLOR1_NO,BACKGROUND_COLOR_NO);
			lcd.setTextFont(FONT_DEFAULT);
			lcd.writeSentenceInArea(LISTBOX_SONG_X, LISTBOX_SONG2_Y + lcd.getFontHeight(FONT_LARGE) +5,
									LCD_WIDTH-1,LISTBOX_SONG2_Y+lcd.getFontHeight(FONT_LARGE) +8 + lcd.getFontHeight(FONT_DEFAULT)*2,
									getSongDescription((SongType)song2No),x,y);
		}
		void ViewSpeechSongs::loop() {
		}
		
		void ViewSpeechSongs::touchEvent(uint8_t pTouchId) {
			switch (pTouchId) {
				case TOUCH_AREA_BACK:
					display.activateView(ViewMainNo);
					break;
				case TOUCH_AREA_SPEECH_POEMS:
					display.activateView(ViewSpeechPoemsNo);
					break;
				case TOUCH_AREA_SPEECH_TALK:
					display.activateView(ViewSpeechTalkNo);
					break;
				case TOUCH_AREA1_UP: {
					uint8_t songNo = (int)firstSongInList -1;
					if (songNo == 0)
						firstSongInList = (SongType)NUMBER_OF_SONGS;
					else
						firstSongInList = (SongType)songNo;
					displaySongList();
					}					
					break;
				case TOUCH_AREA1_DOWN: {
					uint8_t songNo = (int)firstSongInList + 1;
					if (songNo >= NUMBER_OF_SONGS)
						firstSongInList = (SongType)1;
					else
						firstSongInList = (SongType)songNo;					
					displaySongList();
					}					
					break;
				case TOUCH_AREA_SONGS1:
					sendSongRequestToPaul(firstSongInList);
					break;
				case TOUCH_AREA_SONGS2: {
						uint8_t s = (int)firstSongInList + 1;
						if (s > NUMBER_OF_SONGS)
							s = (SongType)1;
						sendSongRequestToPaul((SongType)s);
					}					
					break;
				default:
					Serial.print(F("songs:unknown touch area "));
					Serial.println(pTouchId);
					break;
			}
		}					
