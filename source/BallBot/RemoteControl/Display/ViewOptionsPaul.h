/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEW_OPTIONS_PAUL_H_
#define VIEW_OPTIONS_PAUL_H_

#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"


#include "DisplayController.h"

class ViewOptionsPaul : public View {
	
	public:
		ViewOptionsPaul() {
		}			
		virtual void startup();
		void displayState();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);	
		void displaySliderChange();
		void displayLanguageVoice();
	private:
		boolean changeSentToPaul;
		TimePassedBy sendToPaulTimer;
		boolean anyChange;
		TimePassedBy showNumberTimer;
		boolean speechRateChange;
		boolean volumeChange;
		boolean numberOn;
};


#endif /* VIEW_OPTIONS_PAUL_H_ */