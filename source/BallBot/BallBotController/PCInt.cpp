/*
 * PCInt.cpp
 *
 * Created: 26.11.2012 16:32:13
 *
 * Stolen from Arduino library and slightly amended, used to get interrupts each time an encoder pulse happens
 */ 

#include "Arduino.h"
/*
 * an extension to the interrupt support for arduino.
 * add pin change interrupts to the external interrupts, giving a way
 * for users to have interrupts drive off of any pin.
 * Refer to avr-gcc header files, arduino source and atmega datasheet.
 */

/*
 * Theory: all IO pins on Atmega168 are covered by Pin Change Interrupts.
 * The PCINT corresponding to the pin must be enabled and masked, and
 * an ISR routine provided.  Since PCINTs are per port, not per pin, the ISR
 * must use some logic to actually implement a per-pin interrupt service.
 */

/* Pin to interrupt map:
 * D0-D7 = PCINT 16-23 = PCIR2 = PD = PCIE2 = pcmsk2
 * D8-D13 = PCINT 0-5 = PCIR0 = PB = PCIE0 = pcmsk0
 * A0-A5 (D14-D19) = PCINT 8-13 = PCIR1 = PC = PCIE1 = pcmsk1
 */

volatile uint8_t *port_to_pcmask[] = {
  &PCMSK0,
  &PCMSK1,
  &PCMSK2,
  &PCMSK3,
};

/*
 * attach an interrupt to a specific pin using pin change interrupts.
 */
 void PCattachInterrupt(uint8_t pin, int mode) {
 
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
 
  uint8_t slot;
  volatile uint8_t *pcmask;

  // map pin to PCIR register
  if (port == NOT_A_PORT) {
    return;
  }
  else {
    port -= 1;
    pcmask = port_to_pcmask[port];
  }

// -- Fix by Baziki. In the original sources it was a little bug, which cause analog ports to work incorrectly.
  if (port == 1) {
     slot = port * 8 + (pin - 14);
  }
  else {
     slot = port * 8 + (pin % 8);
  }
// --Fix end
  // set the mask
  *pcmask |= bit;
  // enable the interrupt
  PCICR |= 0x01 << port;
}

