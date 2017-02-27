/*
 * motors.cpp
 *
 * Created: 16.11.2012 23:18:35
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "FixedPoint.h"
#include "EEPROM.h"
#include <avr/wdt.h> 
#include "avr/eeprom.h"
#include "Motor.h"

// that's the three motors and their values
Motor motor[WHEELS];

// for performance reasons use explicit variables outside the Motor instance
volatile int16_t count_1 = 0; // encoder counter 
volatile int16_t count_2 = 0; // encoder counter 
volatile int16_t count_3 = 0; // encoder counter 

// initialize all motors and stop them
void Motor::init() {
	motor[0].drive(0);
	motor[1].drive(0);
	motor[2].drive(0);
	count_1 = 0;
	count_2 = 0;
	count_3 = 0;
}

// drive a motor with a given Torque between -255..255
void Motor::drive(int16_t pTorque)  {
	int16_t absTorque = abs(pTorque);
		
	// backlash compensation is done by correcting the encoder position 
	// by BACKLASH/2 each time a change in the direction happens. 
	// a bit heuristic, I know, but improves balancing bahaviour
	static int16_t backlashEncoderCompensation = ((ENCODER_PULES_PER_REV*MOTOR_BACKLASH)/360)/2;
	if (pTorque > 0) {
		if (lastDirection <= 0) {	
			// change direction to forward		
			digitalWrite(GET_INA_PIN(wheelNumber), LOW); 
			digitalWrite(GET_INB_PIN(wheelNumber), HIGH);		

			// remove half of the backlash as actual encoder position 
			// in order to let PI controller adds it 
			(*count) = (*count)-backlashEncoderCompensation;
		}			
	} else if (pTorque < 0) {
		if (lastDirection >= 0) {
			// change direction to backward
			digitalWrite(GET_INA_PIN(wheelNumber), HIGH); 
			digitalWrite(GET_INB_PIN(wheelNumber), LOW);	
				
			// add half of the backlash as actual encoder position 
			// in order to let PI controller remove it 
			(*count) = (*count)+backlashEncoderCompensation;
		}			
	}	
	
	// direction is set, set PWM value now
	if (absTorque > 255 )
		absTorque = 255;		
	analogWrite(GET_PWM_PIN(wheelNumber),absTorque);
	lastDirection = sign(pTorque);
};

// get the encoder count of that motor and add it to the parameter
void Motor::getEncoderCount(int16_t& pEncoder)
{
	pEncoder += (*count);
	(*count) = 0;
};	

void setPwmFrequency(int pin, int divisor);

SIGNAL(PCINT0_vect) {
	// pulse and direction, direct port reading to save cycles
	// check Encoder A on PINA0 for raising edge, and PINA1 for direction
	
	if (PINA & 0b01) {
		if (PINA & 0b10)
			count_1++;    
		else                      
			count_1--;  
	} else {
		if (PINA & 0b10)
			count_1--;     
		else                      
			count_1++;     
	}				
}
SIGNAL(PCINT1_vect) {
	// pulse and direction, direct port reading to save cycles
	// check Encoder A on PINB0 for raising edge, and PINB1 for direction
	if (PINB & 0b01) { 
		if (PINB & 0b10)
			count_2++;         
		else                      
			count_2--;      
	} else {
		if (PINB & 0b10)
			count_2--;
		else                      
			count_2++;  
	}	
}
SIGNAL(PCINT2_vect) {
	// pulse and direction, direct port reading to save cycles
	// check Encoder A on PINC6 for raising edge, and PINC7 for direction
	if (PINC & 0b01000000) { 
		if (PINC & 0b10000000)
			count_3++;           
		else                      
			count_3--;             
	} else {
		if (PINC & 0b10000000)
			count_3--;          
		else                      
			count_3++;        
	}	
}

// setup all motors, that is attach the pwm and encoder pins to it and set the pwm frqeuency
void Motor::setup() {
	// motor of wheel 1,
	pinMode(INA_APIN_1, OUTPUT); 
	pinMode(INB_APIN_1, OUTPUT); 
	pinMode(PWM_APIN_1, OUTPUT); 
	// motor of wheel 2, and ... 
	pinMode(INA_APIN_2, OUTPUT); 
	pinMode(INB_APIN_2, OUTPUT); 
	pinMode(PWM_APIN_2, OUTPUT); 
	// ... guess what
	pinMode(INA_APIN_3, OUTPUT); 
	pinMode(INB_APIN_3, OUTPUT); 
	pinMode(PWM_APIN_3, OUTPUT); 
  
	// setup PWM prescaler frequency to F_CPU/256/prescaler=8 = 9700 Hz
	// havent yet tried to set it higher, 9700 Hz is a bit annoying
	if (PWM_PRESCALER != 0) {  
		setPwmFrequency(PWM_APIN_1, PWM_PRESCALER);
		setPwmFrequency(PWM_APIN_2, PWM_PRESCALER);
		setPwmFrequency(PWM_APIN_3, PWM_PRESCALER);
	}

	// initialize encoder pins	 
	pinMode(ENCODE_A_APIN_1, INPUT); 
	pinMode(ENCODE_B_APIN_1, INPUT); 
	pinMode(ENCODE_A_APIN_2, INPUT); 
	pinMode(ENCODE_B_APIN_2, INPUT); 
	pinMode(ENCODE_A_APIN_3, INPUT); 
	pinMode(ENCODE_B_APIN_3, INPUT); 
  
	// turn on pullup resistor
	digitalWrite(ENCODE_A_APIN_1, HIGH);		
	digitalWrite(ENCODE_B_APIN_1, HIGH);		
	digitalWrite(ENCODE_A_APIN_2, HIGH);		
	digitalWrite(ENCODE_B_APIN_2, HIGH);        
	digitalWrite(ENCODE_A_APIN_3, HIGH);        
	digitalWrite(ENCODE_B_APIN_3, HIGH);		

	// attach all encoder A-pins to individual PCINT interrupt and mask 
	// them accordingly, so that each change in a A-pin has its own interrupt
	// we waste the encoder's double precision of having 29*64 interrupts, 29*32 interupts yielded by 
	// watching any change in encoder-a pin gives 924 pulses per revolution, i.e. a 
	// prevision of 0,3°. That should be sufficient.
	PCattachInterrupt(ENCODE_A_APIN_1, CHANGE);
	PCattachInterrupt(ENCODE_A_APIN_2, CHANGE);
	PCattachInterrupt(ENCODE_A_APIN_3, CHANGE);
	
	count_1 = 0;
	count_2 = 0;
	count_3 = 0;
	
	// initialize the motor structure
	motor[0].set(0,&count_1);
	motor[1].set(1,&count_2);
	motor[2].set(2,&count_3);
}


/**
 * Divides a given PWM pin frequency by a divisor.
 *
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 *
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 *
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 *
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 4 || pin == 5 || pin == 6 || pin == 7 || pin == 11 || pin == 12) {
    switch(divisor) {
      case    1: mode = 0b001; break;
      case    8: mode = 0b010; break;
      case   32: mode = 0b101; break;
      case   64: mode = 0b100; break;
      case  128: mode = 0b101; break;
      case  256: mode = 0b110; break;
      case 1024: mode = 0b111; break;
      default: return;
    }
    if((pin == 4) || (pin == 5)) {
      TCCR1B = (TCCR1B & 0b11111000) | mode;
    } 
    if((pin == 6) || (pin == 7)) {
      TCCR2B = (TCCR2B & 0b11111000) | mode;
    } 
	if ((pin == 11) || (pin == 11)) {
	  TCCR0B = (TCCR0B & 0b11111000) | mode;
	}		
  } 
}