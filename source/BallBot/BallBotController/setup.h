/*
 * setup.h
 *
 * Created: 16.11.2012 23:20:00
 * Author: JochenAlt
 *
 * basic numbers, definitions and a couple of helper classes
 */ 


#ifndef SETUP_H_
#define SETUP_H_

#include "FixedPoint.h"
#include "TimePassedBy.h"

// this is the bot board ( used in the communication interface that works on base of the same data types)
#define BOT_BOARD 
#undef MAIN_BOARD

#define BALL_RADIUS float(110.0) // mm
#define WHEEL_RADIUS float(24.0) // mm
#define WHEEL_ANGLE  float(40.0) // angle between platform and wheel axis [°]
#define COG_HEIGHT 500			 // height of centre of gravity to ground [mm]

#define IMU_SAMPLE_FREQ 200									// should be something that fits into 1Kh /(1 + <int>) = IMU_SAMPLE_RATE 
#define IMU_SAMPLE_LOOPTIME_US (1000000UL/IMU_SAMPLE_FREQ)	// sample loop time in microseconds

#define   INA_APIN_1     PIN_A2  // INA 1 motor 
#define   INB_APIN_1     PIN_A3  // INB 1 motor 
#define   PWM_APIN_1     PIN_D7	 // PWM 1 motor TIMER2A

#define   INA_APIN_2     PIN_A4  // INA 2 motor 
#define   INB_APIN_2     PIN_A5  // INB 2 motor 
#define   PWM_APIN_2     PIN_D6	 // PWM 2 motor TIMER2B

#define   INA_APIN_3     PIN_A6  // INA 3 motor 
#define   INB_APIN_3     PIN_A7  // INB 3 motor 
#define   PWM_APIN_3     PIN_B3	 // PWM 3 motor TIMER0A

#define	  ENCODE_A_APIN_1  PIN_A1 // encoder 1 A pin, PCINT0   
#define	  ENCODE_B_APIN_1  PIN_A0 // encoder 1 B pin, PCINT1   
#define	  ENCODE_A_APIN_2  PIN_B0 // encoder 2 A pin, PCINT8   
#define	  ENCODE_B_APIN_2  PIN_B1 // encoder 2 B pin, PCINT9   
#define	  ENCODE_A_APIN_3  PIN_C6 // encoder 3 A pin, PCINT22   
#define	  ENCODE_B_APIN_3  PIN_C7 // encoder 3 B pin, PCINT23   

#define GET_PWM_PIN(m) ((m == 0)?PWM_APIN_1:(m == 1)?PWM_APIN_2:PWM_APIN_3)
#define GET_INA_PIN(m) ((m == 0)?INA_APIN_1:(m == 1)?INA_APIN_2:INA_APIN_3)
#define GET_INB_PIN(m) ((m == 0)?INB_APIN_1:(m == 1)?INB_APIN_2:INB_APIN_3)
#define GET_ENCA_PIN(m) ((m == 0)?ENCODE_A_APIN_1:(m == 1)?ENCODE_A_APIN_2:ENCODE_A_APIN_3)
#define GET_ENCB_PIN(m) ((m == 0)?ENCODE_B_APIN_1:(m == 1)?ENCODE_B_APIN_2:ENCODE_B_APIN_3)

// A2 needs to be connected via voltage divider of 1:6 to VDD (before the 7805) which is 3 LiPo = 11,1V
// #define VOLTAGE_PIN PIN_A2
#define LED_PIN_BLUE PIN_B4
#define IMU_RESET_PIN PIN_D3

// description of the motors used
#define WHEELS 3						// we have three wheels
#define ENCODER_PULES_PER_REV (29*32)	// encoder have 32 pulses per revolution, gearbox is 1:29
#define MAX_REVOLUTIONS (1800)			// motor property: ° per second at max voltage
#define MOTOR_BACKLASH 1				// gearbox + wheel backlash [degrees] is corrected with each change in direction
#define MAX_SPEED 1800					// maximum speed per in x/y direction [mm/s]
#define MAX_WHEEL_SPEED  (FP32(2000,4)) // max speed per wheel is 2000 revolutions/s ( due to fixed-point-fp4)
#define ACCEL_DEAD_ZONE 30  // [mm/s2]
#define ACCEL_MIN 120		// [mm/s2]


#define PWM_PRESCALER 8					// relevant for PWM frequency and all timers to set PWM frequency 
#define TIMER_SHIFT 3					// due to the setting of the PWM frequency, all Arduino timers have to be shifted by 3

// baud rate for comunication with main board
#define INITIAL_BAUD_RATE 57600 

// reset the entire micro controller (including the IMU)
void resetMicroController();

// encoders pulses are grabbed by use of PCint
// this functions sets the interrupts accordingly
// but no function is attached, this is done 
// more efficient directly in the port-wise interrupt function
void PCattachInterrupt(uint8_t pin, int mode);

// timer's behavior, needs to be redefined to changed standard PWM prescaler
void delay_ms(uint16_t pMS);
uint32_t milliseconds();
uint32_t microseconds();
void setErrorLamp(const __FlashStringHelper* pLine_P);


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
/* 
class TimePassedBy {
	public:
	enum ModeType {US, MS};
	// initialize this timer to work on [ms] or on [us]
	TimePassedBy (ModeType pMode = MS) {
		mMode = pMode;
		mLastCall_ms = milliseconds();
		mLastCall_us = microseconds();
	} 
	// true, if at least <ms> milliseconds passed since last invocation that returned true.
	// returns the actual passed time in addition
	bool due_ms(uint16_t ms, uint16_t &passed_ms) {
		uint32_t now = milliseconds();
		passed_ms = now-mLastCall_ms;
		if (passed_ms>=ms) {
			mLastCall_ms = now;
			return true;
		}
		return false;
	}
	// same but with microseconds
	bool due_us(uint32_t us,uint16_t &passed_us) {
		uint32_t now = microseconds();
		passed_us = (uint32_t)(now-mLastCall_us);
		if (passed_us >=us) {
			mLastCall_us = now;
			return true;
		}
		return false;
	}
	
	uint32_t mLastCall_us;	// last due time in microseconds
	uint16_t mLastCall_ms;	// last due time in milliseconds
	uint32_t now;			// current time
	ModeType mMode;			// [ms] or [us]
};

*/
// Class that implements a pretty pattern blinker without use of delay(). 
// Has to run in a loop.
// use:
//      // define blink pattern, 1=on, 0=off
//		static uint8_t BotIsBalancing[3] = { 0b11001000,0b00001100,0b10000000}; // nice pattern. Each digit takes 50ms
//																				// define blink pattern array as long 
//																				// as you like
//		PatternBlinker blinker;													// initiate pattern blinker
//		blinker.set(LED_PIN_BLUE,BotIsBalancing,sizeof(BotIsBalancing));		// assign pattern
//		while (true) {
//			blinker.loop();														// switch on off LED when necessary
//			<do something else>
//		}
// 
class PatternBlinker {
	public:
		PatternBlinker() {
			mPin = -1;
			mPattern = NULL;
		}
		
		// switch blinker off		
		void off() {
			mPattern = NULL;
			if (mPin != -1)
				digitalWrite(mPin,HIGH);
		}		
		// set the blink pattern on the passed pin
		void set(uint8_t pPin, uint8_t* pPattern, uint8_t pPatternLength) {
			mPattern = pPattern;
			mPin = pPin;
			mPatternLen = pPatternLength;
			mSeq = 0;
		}
		void loop() {
			uint16_t passed_ms;
			if ((mPattern != NULL) && (timer.isDue_ms(50,passed_ms))) {
				uint8_t pos,bitpos;
				pos = mSeq / 8;
				bitpos = 7- (mSeq & 0x07);
				if ((mPattern[pos] & _BV(bitpos)) > 0) {
					digitalWrite(mPin,LOW);
				}					
				else {
					digitalWrite(mPin,HIGH);
				}

				mSeq++;
				if (mSeq >= (mPatternLen)*8)
					mSeq = 0;
			}
		}
		uint8_t mSeq;			// current position within the blink pattern
		int8_t mPin;			// pin to be used
		uint8_t* mPattern;		// blink pattern, passed in set()
		uint8_t  mPatternLen;	// length of the pattern  = sizeof(*mPattern)
		TimePassedBy timer;		// timer for checking passed time
};

// complementary filter that does not loose any tick. It keeps track of all
// numbers accepted and delivered and tries to keep the difference small. 
// Not really the way how control theory defines that, frequency is hard to identify by this
// but sometimes it is important to deliver all values (e.g. for encoder pulses)
class ComplementaryFilter {
	public:
		ComplementaryFilter () {
			init(0.1,IMU_SAMPLE_FREQ);
		}		
		// pass the weight of the most recent value
		// (1-ratio) is the weight of the filtered value
		void init(float ratio, int16_t pFrequency) {
			init(FLOAT2FP16(ratio/(ratio + 1.0/pFrequency),8));
		};
		void init(int16_t pFilterRatio_fp8) {
			inputOutputDiff = 0;
			compFilter = 0;
			compFilterRatio_fp8 = pFilterRatio_fp8;
		};

		// get a filtered value
		int16_fp8_t get(int16_t pValue) {
			int16_t value = (inputOutputDiff)+(pValue);
			compFilter = 
					(int16_t)i32_rsh(	mul16s(compFilterRatio_fp8,value) + 
										mul16s((1<<8)-compFilterRatio_fp8,compFilter),8),
			inputOutputDiff += (pValue-compFilter);
			return compFilter;
		};
	private:
		// difference of ticks accepted and delivered	
		int16_t inputOutputDiff;
		// complementary value
		int16_t compFilter;
		// weight of the latest value over the last complementary value
		int16_t compFilterRatio_fp8;
};

// a moving average is used to accumulate encoder pulses in a way that one loop gives a result that makes sense 
// (instead of notchy jumps due to the high sample rate and the encoder impulses of just 928/revolution)
// Implementation as a ringbuffer that keeps track of the sum of all contained elements in order to - in the long run -
// don't loose any encoder pulses.
#define SAMPLE_SIZE_EXP  5					// log2 of size of ring buffer
#define SAMPLE_SIZE (1<<SAMPLE_SIZE_EXP)	// size of ring buffer
class MovingAverage {
	public:
		MovingAverage() {
			sampleSum = 0;
			for (uint8_t i = 0;i<SAMPLE_SIZE;i++)
				sample[i] = 0;
			putPos = 0;		
		};
		
		// get moving average of the current (passed) value plus the last 31 values
		int16_fp8_t getAverage_fp8(int8_t pValue) {
			sampleSum += pValue;
			sampleSum -= sample[putPos];
			sample[putPos++] = pValue;
			if (putPos == SAMPLE_SIZE)
				putPos = 0;	
			return (sampleSum << (8-SAMPLE_SIZE_EXP));
		};
		int16_t sampleSum;
		int8_t sample[SAMPLE_SIZE];
		uint8_t putPos;	
};




#endif /* SETUP_H_ */