/*
 * ViewSplash.h
 *
 * Created: 29.03.2013 10:29:39
 *  Author: JochenAlt
 */ 


#ifndef VIEWSPLASH_H_
#define VIEWSPLASH_H_

#include "View.h"
#include "TimePassedBy.h"


class ViewSplash: public View  {
	
	public:
		virtual void startup();
		boolean isFinished();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);
	private:
		TimePassedBy timer;
		int16_t x;
		int16_t y;
		float alpha;
};



#endif /* VIEWSPLASH_H_ */