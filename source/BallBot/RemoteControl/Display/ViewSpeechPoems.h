/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEWSPEECHPOEMS_H_
#define VIEWSPEECHPOEMS_H_

#include "View.h"
#include "Paul.h"

class ViewSpeechPoems : public View {
	
	public:
		ViewSpeechPoems() {
			poem = (PoemType)1; // whatever the first poem is 		
		}			
		virtual void startup();
		void sendPoemRequestToPaul(PoemType pPoemNo);
		void displayPoemList();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);		
	private:
		PoemType poem;
};



#endif /* VIEWSPEECH_H_ */