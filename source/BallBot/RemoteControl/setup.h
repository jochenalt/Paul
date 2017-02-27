/*
 * setup.h
 *
 * Created: 15.02.2013 13:31:30
 *  Author: JochenAlt
 */ 


#ifndef SETUP_H_
#define SETUP_H_

#include "Paul.h"
// this is the main board ( used in the communication interface that works on base of the same data types)
#undef  BOT_BOARD 
#define REMOTE_BOARD
#undef MAIN_BOARD

// bot controller board and xbee board use UART
#define REMOTECONTROL_BAUD_RATE 57600

#define XBEE_BAUD_RATE 57600
#define XBEE_CHARACTER_TIMEOUT_US ((1000000UL/XBEE_BAUD_RATE)*10*2) // timeout per character, double the time a characters takes

// battery checker, with a resistor divider of 10/(56+10), Vref=2,56V, 10bits are used
#define BATT_PIN PIN_A0

// battery checker, with a resistor divider of 10/(56+10), Vref=2,56V, 10bits are used
#define POWER_RELAY_PIN PIN_B0
#define POWER_SWITCH_PIN PIN_B1


// everyone likes a blinking LED!
#define LED_PIN PIN_A7

// PINS of joystick
#define JOYSTICK_X_PIN PIN_A4
#define JOYSTICK_Y_PIN PIN_A3
#define JOYSTICK_Z_PIN PIN_A2
#define JOYSTICK_BUTTON_PIN PIN_C7

// Pins of Display
#define EDIP_SBUF_PIN PIN_D6   // SBUF pin indicates data in the display's sendbuffer (e.g. touch events)
#define EDIP_SS_PIN	  PIN_D7   // SS for Display
#define EDIP_MOSI_PIN PIN_B5   // SPI connection
#define EDIP_MISO_PIN PIN_B6   // SPI connection
#define EDIP_CLK_PIN  PIN_B7   // SPI connection

float getCurrentVoltage();
void setErrorLamp(const __FlashStringHelper* pLine_P);

#endif /* SETUP_H_ */