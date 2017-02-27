/*
 * utilities.h
 *
 * Created: 18.02.2013 14:36:02
 *  Author: JochenAlt
 */ 


#ifndef TIMEPASSEDBY_H_
#define TIMEPASSEDBY_H_

// small helper class to measure the passed time since an event and to check 
// whether something that is supposed to run at a specific time (due_ms/due_us)
// use:
//     TimePassedBy timer(MS);						// initialize timer that is supposed to execute something periodically
//     int16_t passed_ms;		
//     while (true) {								// to be used in a loop
//			<do what you like>
//			if (timer.due_ms(200, passed_ms))		// check whether 200ms have been passed and 
//				<do something that has to be called every 200ms>
//		}
// 
class TimePassedBy {
	public:
	// initialize this timer to work 
	TimePassedBy () {
		mLastCall_ms = millis();
	} 
	// true, if at least <ms> milliseconds passed since last invocation that returned true.
	// returns the actual passed time in addition
	bool isDue_ms(uint16_t ms, uint16_t &passed_ms) {
		uint32_t now = millis();
		passed_ms = now-mLastCall_ms;
		if (passed_ms>=ms) {
			mLastCall_ms = now;
			return true;
		}
		return false;
	}
	void setDueTime (uint16_t due_ms) {
		mLastCall_ms = millis()-due_ms;
	}
	
	bool isDue_ms(uint16_t ms) {
		uint16_t passed_ms;
		return isDue_ms(ms, passed_ms);
	}

	uint16_t mLastCall_ms;	// last due time in milliseconds
	uint32_t now;			// current time
};


/*
 * utilities.cpp
 *
 * Created: 18.02.2013 14:35:48
 *  Author: JochenAlt
 */ 




#endif /* TIMEPASSEDBY */