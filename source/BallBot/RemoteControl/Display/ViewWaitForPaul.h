/*
 * ViewWaitForPaul.h
 *
 * Created: 26.03.2013 16:02:45
 *  Author: JochenAlt
 */ 


#ifndef VIEWWAITFORPAUL_H_
#define VIEWWAITFORPAUL_H_

#include "View.h"
#include "TimePassedBy.h"

class ViewWaitForPaul: public View  {	
	public:
		virtual void startup();
		virtual void loop();
		virtual void touchEvent(uint8_t pTouchId);		
	private:
		TimePassedBy timer;
		float alpha;

};

#endif /* VIEWWAITFORPAUL_H_ */