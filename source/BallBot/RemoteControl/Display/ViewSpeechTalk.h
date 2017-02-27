/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEWSPEECHTALK_H_
#define VIEWSPEECHTALK_H_

#include "View.h"
#include "DisplayController.h"

#define TYPING_BUFFER_SIZE 64
#define SCROLLING_BUFFER_SIZE 128

class ViewSpeechTalk : public View {
	
	public:
		ViewSpeechTalk() {
			typingBuffer[0] = 0;
			scrollingArea[0] = 0;
			scrollingLines = 0;
		}			
		virtual void startup();
		int8_t searchCharInString(char c, char* str);
		void sendTalkRequestToPaul(char* pText);
		void addChar(char c);
		void displayTypingArea();
		void displayScrollingArea();
		virtual void loop() {}
		
		virtual void touchEvent(uint8_t pTouchId);			
	private:
		char typingBuffer[TYPING_BUFFER_SIZE];

		char scrollingArea[SCROLLING_BUFFER_SIZE];
		uint8_t scrollingLines;
		boolean sayIt ;
};


#endif /* VIEWSPEECHTALK_H_ */