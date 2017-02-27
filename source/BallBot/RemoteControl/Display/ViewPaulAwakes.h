/*
 * ViewPaulAwakes.h
 *
 * Created: 23.03.2013 19:23:21
 *  Author: JochenAlt
 */ 


#ifndef VIEWPAULAWAKES_H_
#define VIEWPAULAWAKES_H_

#include "View.h"
#include "TimePassedBy.h"
#include "MsgType.h"

class ViewPaulAwakes : public View  {
	
	public:
		virtual void startup();
		void paulSleeps();
		virtual void loop();
		void setSpokenText(char *pText);
		virtual void touchEvent(uint8_t pTouchId);
	private:
		TimePassedBy blinkTimer;
		TimePassedBy timer;
		TimePassedBy speechTimer;
		uint8_t textLen;
		uint8_t blinkStep;
};


#endif /* VIEWPAULAWAKES_H_ */