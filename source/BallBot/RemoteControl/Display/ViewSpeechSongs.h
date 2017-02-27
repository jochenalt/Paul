/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEWSPEECHSONGS_H_
#define VIEWSPEECHSONGS_H_

#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"
#include "DisplayController.h"
#include "Paul.h"

class ViewSpeechSongs : public View {
	
	public:
		ViewSpeechSongs() {
			firstSongInList = (SongType)3; // whatever the first song is 		
		}			
	virtual void startup();
	void sendSongRequestToPaul(SongType pSongNo);
	void displaySongList();
	virtual void loop();		
	virtual void touchEvent(uint8_t pTouchId);					
	private:
		SongType firstSongInList;
};


#endif /* VIEWSPEECHSONGS_H_ */