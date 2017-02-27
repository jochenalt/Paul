/*
 * setup.h
 *
 * Created: 15.02.2013 13:31:30
 *  Author: JochenAlt
 */ 


#ifndef SETUP_H_
#define SETUP_H_

#include "FixedPoint.h"

// this is the main board ( used in the communication interface that works on base of the same data types)
#undef  BOT_BOARD 
#undef  REMOTE_BOARD
#define MAIN_BOARD


// bot controller board and xbee board use UART
#define BOT_CONTROLLER_BOARD_BAUD_RATE 57600
// frequency main loop of bot controller
#define BOT_CONTROLLER_FREQUENCY 166 // [Hz]

#define XBEE_BAUD_RATE 57600
#define XBEE_CHARACTER_TIMEOUT_US ((1000000UL/XBEE_BAUD_RATE)*10*2) // timeout per character, double the time a characters takes

// everyone likes a blinking LED!
#define LED_PIN PIN_A1

// battery checker, with a resistor divider of 10/(56+10), Vref=2,56V, 10bits are used
#define BATT_PIN PIN_A0

// pin that is 3,7V when EMIC-2 is talking
#define EMIC2_TALKS_PIN PIN_B2

// red error LED
#define ERROR_LED PIN_D7
// blue talking LED
#define DONT_KNOW_LED PIN_D6
// green balancing LED
#define IS_BALANCING_LED PIN_C7

// switch for balance mode
#define BALANCING_SWITCH_PIN PIN_C6

// text 2 speech module EMIC-2
#define EMIC2_RX_PIN PIN_B0 
#define EMIC2_TX_PIN PIN_B1
#define EMIC2_SP_PLUS_PIN PIN_A2			// PIN SP+ of EMIC-2 module to ADC to compute the soundwave
#define EMIC2_BAUDRATE 9600					// fixed in EMIC-2 module
#define AMPLITUDE_MEASUREMENT_FREQUENCY 30  // amplitude of current speech is measured with this sample rate [Hz]

// LED driver
#define LED_DRIVER1_I2C_ADDRESS 0x60
#define LED_DRIVER2_I2C_ADDRESS 0x61
#define LED_DRIVER3_I2C_ADDRESS 0x62


#define NUMBER_OF_LEDS 48
#define SAMPLE_TIME 50				// sample time in [ms] when the LED Controller changes PWM values
#define EYE_SPEED 10				// speed [deg/SAMPLE_TIME] the eyes move 
#define EYE_DISTANCE_SPEED 3		// speed [deg/SAMPLE_TIME] the distance of the eyes move 
#define EYE_WIDTH_SPEED 1			// speed [deg/SAMPLE_TIME] the width of the eyes inc/dec
#define LED_ZERO_POSITION (120)		// compensate how the LEDs are built in, turn everyhting by 90°

#define EYE_BLINKING 3000			// time in [ms] the eyes are blinking
#define EYE_BLINKING_TIME 70		// time a blinking takes in ms
#define EYE_BLINKING_DARK_TIME 50	// time during a blinking while eyes are dark
#define EYE_BLINKING_RND 500		// maximum time the blinking frequency varies
#define EYE_BLINKING_PAUSE 800		// minimum time between two blinking			
#define EYE_DEFAULTBRIGHTNESS 220	
#define TILT_SPEED	10				// speed [°/SAMPLE_TIME] the tilt wave moves
#define TILT_WIDTH 90				// width of the tilt wave in [°] 

#define SOUND_WAVE_SPEED 5			// speed of soundwave in [°/SAMPLE_TIME]
#define SOUND_WAVE_FREQUENCY AMPLITUDE_MEASUREMENT_FREQUENCY	// frequency between two sound amplitude samples

float getCurrentVoltage();
void setErrorLamp(const __FlashStringHelper* pLine_P);


#endif /* SETUP_H_ */