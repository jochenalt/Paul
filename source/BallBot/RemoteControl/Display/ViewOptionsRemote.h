/*
 * ViewSpeech.h
 *
 * Created: 26.03.2013 11:42:21
 *  Author: JochenAlt
 */ 


#ifndef VIEW_OPTIONS_REMOTE_H_
#define VIEW_OPTIONS_REMOTE_H_

#include "eDIP-TFT.h"
#include "eDIP-TFT-flash.h"
#include "FixedPoint.h"
#include "View.h"


#include "DisplayController.h"

class ViewOptionsRemote : public View {
	
	public:
		ViewOptionsRemote() {
		}			
		virtual void startup();
		void displayState();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);		
	private:
		boolean anyChange;
};


#endif /* VIEW_OPTIONS_REMOTE_H_ */