/*
 * Motors.h
 *
 * Created: 08.01.2013 17:40:41
 * Author: JochenAlt
 * 
 * Class to control the Pololu DC motor with 1:29 gearbox via Pololu motor driver VNH2SP30
 * This provides simple interace to get encoder position, set the (uncontrolled) torque and direction
 */ 


#ifndef MOTORS_H_
#define MOTORS_H_



class Motor {
	private:
		int8_t lastDirection;		// direction the motor is currently running, -1,0,+1
		uint8_t wheelNumber;		// my own number
		volatile int16_t* count;	// rotation counter right motor
	public:
		// set my number and my encoder position
		void set(uint8_t pNumber, volatile int16_t* pCount) {
			lastDirection = 0;
			wheelNumber = pNumber;
			count = pCount;			
		}
		
		// define torque of a motor torque = -255..255
		void drive(int16_t pTorque);
		
		// add the encoder ticks that occured since the last invocation of getEncoderCount
		void getEncoderCount(int16_t& pEncoder);
		
		// assign all pins, set PWM frequency to prescaler of 8
		// needs to be called before everything else
		static void setup();
		
		// stop everything and (re)-initialize encoder positions
		static void init();
};

extern Motor motor[WHEELS];

#endif /* MOTORS_H_ */