/*
 * LEDController.h
 *
 * Created: 15.02.2013 13:41:49
 * Author: JochenAlt
 */ 


#ifndef LEDCONTROLLER_H_
#define LEDCONTROLLER_H_

#include "Arduino.h"
#include "setup.h"
#include "TimePassedBy.h"

class LEDController {
	public:
		void init() {
			for (int i = 0;i<NUMBER_OF_LEDS;i++) {
				soundWave[i] = 0;
				eyes[i] = 0;
				values[i] = 0;
				tilt[i] = 0;
			}
			for (int i = 0;i<sizeof(soundWaveValue);i++)
				soundWaveValue[i] = 0;
			soundWaveBrightness = 48;	
			soundWaveSpeed = SOUND_WAVE_SPEED;		// sound wave speed in ° /s 	
			
			// configuration data of the eye blinking
			eyeToBeDistance = 90;					// distance of iris in degrees
			eyeToBeWidth = 0;						// Width of one eye in degrees
			eyeCurrentDistance = eyeToBeDistance;	// distance of iris in degrees
			eyeCurrentWidth = eyeToBeWidth;			// Width of one eye in degrees
			eyeCurrentPosition = 0;					// position of eyes in degrees
			eyeToBePosition = 0;					// position of eyes in degrees		
			eyeBlinkingTime = 0;					// current time of the eye in [ms]
			eyeBrightness = 0;						// brightness of eyes in 0..255		
			blinkNowTimes = 0;						// blink now zero times
			
			eyeBlinkingTime = EYE_BLINKING;			// blinking frequency in ms
			eyeBlinkingPhaseTime = 0;				// time [ms] within blinking phase
			eyeKillerLook = false;					// eyes without fading out at the boundaries
			tiltBrightness = 32;					// brightness of tilt effect 0..255
			currentTiltAngle = 0;					// angle of tilt
			tiltAngle = 0;
			tiltStrength = 0;
		}
				
		LEDController () {
			init();		
		}
		
		void setup();
		
		// set how eyes are supposed to look like
		uint16_t getEyeDistance() { return eyeToBeDistance;};
		uint16_t getEyeWidth() { return eyeToBeWidth;};
		uint16_t getEyebrightness() { return eyeBrightness;};
			
		void setEyes (	uint16_t pEyeDistance /* degrees */, 
						uint16_t pEyeWidth /* mm */,
						uint16_t pMiddlePosition /* degrees where the eyes are */, 
						uint8_t  pEyesBrightness /* brightness of eyes 0..255 */,
						bool  pKillerEyes /* eyes are not fading out but completely bright (like terminator eyes) */);
		void blinkNow(uint8_t pTimes); // blink now a couple of times
				
		// set soundWave
		void setSoundWaveBrightness(uint8_t pSoundWaveBrightness /* brightness 0..255 */);
		
		// set amplitude of sound that moves around the head
		void setSoundWaveAmplitude(uint8_t pAmplitude);		

		// brightness of tilt effect
		void setTiltBrightness(uint8_t pBrightness);	
	
		// set acceleration per dimension 
		void setTilt(int16_t pTiltAngle, int16_t pTiltStrengt /* 0..255 */);	
		// call as often as possible to create smooth patterns					
		void loop();	
		
		// set PWM value of one LED. LED = 0..31, pPwmValue=0..255
		void setLEDPwm(uint8_t pLEDNo, uint8_t pPWMValue);
		// set PWM value of all LEDs
		void setLEDAll(uint8_t pPWMValue);

		// print LED Controller menu help
		void printMenuHelp();
		void menu();

		void test();
	private:
		// initialize the LED board with passed I2C address
		void setupLEDBoard(int16_t pI2CAddress);

		// compensate the characteristics curve so, that brightness seems to be
		// linear with PWM value. returns a corrected PWM value
		uint8_t compensateCharacteristicCurve(uint8_t pPWMValue) ;

		// compute distance of two positions and the direction with the shortest distance
		int16_t distanceDegree(int16_t pDegree1,int16_t pDegree2, bool &pTurnPos);
		// norm a position to 0..360°
		int16_t normDegree(int16_t pDegree);
		// interpolate the soundWave amplitude value at a certain position in an 180° circle
		uint8_t getSoundWaveValuePerDegree(int16_t pDegree);


		uint8_t soundWave[NUMBER_OF_LEDS];
		uint8_t soundWaveValue[180/SOUND_WAVE_SPEED+1];
		uint8_t soundWaveBrightness;
		uint16_t soundWaveSpeed;			// sound wave speed in ° /s 	
		
		
		uint8_t eyes[NUMBER_OF_LEDS];
		uint16_t eyeCurrentDistance;		// distance of eyes in degrees
		uint16_t eyeCurrentWidth;			// Width of one eye in degrees
		uint16_t eyeToBeWidth;				// Width of one eye in degrees
		uint16_t eyeToBeDistance;			// Width of one eye in degrees
		
		int16_t eyeCurrentPosition;			// position of eyes in degrees
		int16_t eyeToBePosition;			// to-be position of eyes in degrees
		uint16_t eyeBlinkingTime;			// current time in the eye blinking cycle
		uint16_t eyeBlinkingPhaseTime;		// current time within a blinking cycle
		uint16_t eyeBrightness;				// brightness of eyes in 0..255		
		TimePassedBy eyeBlinkingTimer;		// timer that handles blinking c.
		uint8_t blinkNowTimes ;				// blink now times

		bool eyeKillerLook;					// eyes that do not fade out

		TimePassedBy eyeTimer;				// timer that handles loop etc.
		TimePassedBy soundWaveSampleTimer;
		uint8_t tilt[NUMBER_OF_LEDS];
		int16_t currentTiltAngle;
		int16_t tiltAngle;
		uint8_t tiltStrength;

		uint8_t tiltBrightness;

		uint8_t values[NUMBER_OF_LEDS];		// result PWM values that goes to the LED driver
		
		void sendLEDPWMValues();
		void computeSoundWave();
		void computeEyes();
		void computeTiltEffect();
};


extern LEDController ledCtrl;
#endif /* LEDCONTROLLER_H_ */