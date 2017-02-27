/*
 * LEDController.cpp
 *
 * Created: 15.02.2013 13:38:45
 * Author: JochenAlt
 * 
 * Controller for LEDs 
 * - implements several beautiful and emotional blink patterns (soundwave, tilt, eyes)
 * - sends PWM commands to two 16-channel LED driver boards via I2C
 * 
 */

#include "Arduino.h"
#include "LEDController.h"
#include "Wire.h"
#include <avr/wdt.h>


LEDController ledCtrl;

// initialize the LED board with passed I2C address
void LEDController::setupLEDBoard(int16_t pI2CAddress) {
	Wire.begin();     //I2C-Start
	Wire.beginTransmission(pI2CAddress); // TLC59116 Slave Adresse ->C0 hex

	Wire.write(0x80);  // autoincrement ab Register 0h
	Wire.write(0x00);  // Register 00 /  Mode1  
	Wire.write(0x00);  // Register 01 /  Mode2 

	Wire.write(0x00);  // Register 02 /  PWM LED 1    // Default alle PWM auf 0
	Wire.write(0x00);  // Register 03 /  PWM LED 2    
	Wire.write(0x00);  // Register 04 /  PWM LED 3
	Wire.write(0x00);  // Register 05 /  PWM LED 4
	Wire.write(0x00);  // Register 06 /  PWM LED 5
	Wire.write(0x00);  // Register 07 /  PWM LED 6
	Wire.write(0x00);  // Register 08 /  PWM LED 7
	Wire.write(0x00);  // Register 09 /  PWM LED 8
	Wire.write(0x00);  // Register 0A /  PWM LED 9
	Wire.write(0x00);  // Register 0B /  PWM LED 10
	Wire.write(0x00);  // Register 0C /  PWM LED 11
	Wire.write(0x00);  // Register 0D /  PWM LED 12
	Wire.write(0x00);  // Register 0E /  PWM LED 13
	Wire.write(0x00);  // Register 0F /  PWM LED 14
	Wire.write(0x00);  // Register 10 /  PWM LED 15
	Wire.write(0x00);  // Register 11 /  PWM LED 16  // Default alle PWM auf 0

	Wire.write(0xFF);  // Register 12 /  Group duty cycle control
	Wire.write(0x00);  // Register 13 /  Group frequency
	Wire.write(0xAA);  // Register 14 /  LED output state 0  // Default alle LEDs auf PWM
	Wire.write(0xAA);  // Register 15 /  LED output state 1  // Default alle LEDs auf PWM
	Wire.write(0xAA);  // Register 16 /  LED output state 2  // Default alle LEDs auf PWM
	Wire.write(0xAA);  // Register 17 /  LED output state 3  // Default alle LEDs auf PWM
	Wire.write(0x00);  // Register 18 /  I2C bus subaddress 1
	Wire.write(0x00);  // Register 19 /  I2C bus subaddress 2
	Wire.write(0x00);  // Register 1A /  I2C bus subaddress 3
	Wire.write(0x00);  // Register 1B /  All Call I2C bus address
	Wire.write(0xFF);  // Register 1C /  IREF configuration  
	Wire.endTransmission();  // I2C-Stop	
}

// setup I2C line to LED driver
void LEDController::setup() {
	setupLEDBoard(LED_DRIVER1_I2C_ADDRESS);
	setupLEDBoard(LED_DRIVER2_I2C_ADDRESS);
	setupLEDBoard(LED_DRIVER3_I2C_ADDRESS);

	for (int i= 0;i<=255;i++) {
		uint8_t value = compensateCharacteristicCurve(i);
		setLEDAll(value);
	}	  
			  
	for (int i= 255;i>=0;i--) {
		uint8_t value = compensateCharacteristicCurve(i);
	    setLEDAll(value);
	}	  
}


// Diese Funktion setzt die Helligkeit für ein LED-Register 
// Voraussetzung ist, das im entsprechende Register 14 bis 17 die LED aktiviert ist
// Übergabeparameter: LED = Nummer der LED / PWM = Helligkeitswert 0 -255

void LEDController::setLEDPwm(uint8_t pLEDNo, uint8_t pPWMValue)
{
  if (pLEDNo<16) {
	Wire.begin();										// I2C-Start
	Wire.beginTransmission(LED_DRIVER1_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
	Wire.write(0x02 + pLEDNo);							// Register LED-Nr
	Wire.write(pPWMValue);
	Wire.endTransmission();								// I2C-Stop
  }
  else
  {
	if (pLEDNo<32) {
		Wire.begin();										// I2C-Start
		Wire.beginTransmission(LED_DRIVER2_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
		Wire.write(0x02 + pLEDNo-16);						// Register LED-Nr
		Wire.write(pPWMValue);
		Wire.endTransmission();								// I2C-Stop 
	} else {
		Wire.begin();										// I2C-Start
		Wire.beginTransmission(LED_DRIVER3_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
		Wire.write(0x02 + pLEDNo-32);						// Register LED-Nr
		Wire.write(pPWMValue);
		Wire.endTransmission();								// I2C-Stop 
	} 
			
  }	
}

// set all LEDs with same pwm value
void LEDController::setLEDAll(uint8_t pPWMValue)
{
  Wire.begin();										// I2C-Start
  Wire.beginTransmission(LED_DRIVER1_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
  Wire.write(0x82);									// Startregister 02h 
  for (int i=1 ; i < 17; i++){						// 16Bytes (Register 02h bis 11h) schreiben
    Wire.write(pPWMValue);
  }
  Wire.endTransmission();							// I2C-Stop
  Wire.begin();										// I2C-Start
  Wire.beginTransmission(LED_DRIVER2_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
  Wire.write(0x82);									// Startregister 02h 
  for (int i=1 ; i < 17; i++){						// 16Bytes (Register 02h bis 11h) schreiben
    Wire.write(pPWMValue);
  }
  Wire.endTransmission();							// I2C-Stop
  Wire.beginTransmission(LED_DRIVER3_I2C_ADDRESS);	// TLC59116 Slave Adresse ->C0 hex
  Wire.write(0x82);									// Startregister 02h 
  for (int i=1 ; i < 17; i++){						// 16Bytes (Register 02h bis 11h) schreiben
    Wire.write(pPWMValue);
  }
  Wire.endTransmission();							// I2C-Stop

}


// compensate the unlinearity of the LED by compensatig the characteristic curve
// the returned value is to be used instead of the passed PWM value resulting in
// an almost linear brightness behaviour
uint8_t LEDController::compensateCharacteristicCurve(uint8_t pPWMValue) {
	// that's the array how speed is going to be reduced in its interval from 0..255
	static uint8_t termCurve[17] = {0, 3, 7, 12, 18, 25, 33, 43, 55, 69, 85, 103, 123, 145, 189, 217,255};

	// interpolate the speed according to termCurve
	uint8_t pos = pPWMValue >> 4;
    uint8_t left = 	termCurve[pos++];
	uint8_t right = termCurve[pos];
	int16_t x = pPWMValue- (pPWMValue & 0xF0);
	return left + ((x*(right-left))>>4);
}

// set soundWave
void LEDController::setSoundWaveBrightness(uint8_t pSoundWaveBrightness /* brightness 0..255 */) {
	soundWaveBrightness = pSoundWaveBrightness;
}
		
// set amplitude of sound that moves around the head
void LEDController::setSoundWaveAmplitude(uint8_t pAmplitude) {
	if (soundWaveSampleTimer.isDue_ms(1000/SOUND_WAVE_FREQUENCY)) {
		for (int i = 180/SOUND_WAVE_SPEED;i>0;i--)
			soundWaveValue[i] = soundWaveValue[i-1];
		soundWaveValue[0] = pAmplitude;	
	}
}

// interpolate the soundWave amplitude value at a certain position in an 180° circle
uint8_t LEDController::getSoundWaveValuePerDegree(int16_t pDegree) {
	if ((pDegree>=0) && (pDegree<=180)) {
		uint8_t left = pDegree / SOUND_WAVE_SPEED;
		uint8_t right = left+1;
		// interpolate
		uint8_t dx = pDegree-left*SOUND_WAVE_SPEED;
		int16_t dy = int16_t(soundWaveValue[right])-soundWaveValue[left];
		uint8_t result = int16_t(soundWaveValue[left]) + (int16_t(dy) * int16_t(dx))/SOUND_WAVE_SPEED;
		return result;	
	}
	return 0;
}

// soundwave produced a wave of the currently measured amplitude 
// starting from the ears the going to the head's back 
void LEDController::computeSoundWave() {
	static uint8_t s = 0;
	s++;
	// sound wave starts from the ears to the back and the front
	for (int i = 0;i<NUMBER_OF_LEDS;i++) {
		bool turnPos1,turnPos2;
 		int16_t degree = (i * 360)/NUMBER_OF_LEDS - LED_ZERO_POSITION;
		 // distance of current LED to both ears
		int16_t distLeftEar = distanceDegree(degree,80,turnPos1);
		int16_t distRightEar = distanceDegree(degree,-80,turnPos2);
		
		int16_t value = 0;
		int16_t soundWaveValue;
		// the effect goes to the back of the head, which is 100° away 
		// from the ear's positio of +80° and -80°
		if (distLeftEar<=100 && !turnPos1) {
			soundWaveValue = getSoundWaveValuePerDegree(distLeftEar);
			value = (soundWaveValue*soundWaveBrightness)>> 4;
			value = (value *  ( (180-distLeftEar) )) >> 7;  // brightness goes down with distance
		}
		if (distRightEar<=100 && turnPos2) {
			soundWaveValue = getSoundWaveValuePerDegree(distRightEar);
			value = (soundWaveValue*soundWaveBrightness)>> 4;
			value = (value *  ( (180-distRightEar) )) >> 7; // brightness goes down with distance

		}
		soundWave[i] = 	value;
		/*
		if (s == 0) {
			Serial.print("i=");
			Serial.print(i);
			Serial.print("deg=");
			Serial.print(degree);
			Serial.print("dist=");
			Serial.print(dist);
			Serial.print("bright=");
			Serial.print(soundWaveBrightness);
			Serial.print("bright=");
			Serial.print(soundWaveBrightness);
			Serial.print("soundWaveV=");
			Serial.print(soundWaveValue);
			Serial.print("result=");
			Serial.println(soundWave[i]);
		}		
		*/
	 }	
}

// set acceleration per dimension 
void LEDController::setTilt(int16_t pTiltAngle, int16_t pTiltStrength) {
	tiltAngle  = pTiltAngle;
	tiltStrength = pTiltStrength;
}	
		
		
// brightness of tilt effect
void LEDController::setTiltBrightness(uint8_t pBrightness) {
	tiltBrightness = pBrightness;
}

// call as often as possible to create smooth patterns					
void LEDController::loop() {
	uint16_t passedTime;
	if (!eyeTimer.isDue_ms(SAMPLE_TIME,passedTime))
		return;
		
	computeSoundWave();
	computeEyes();
	computeTiltEffect();
		
	// add all values to compute final PWM values
	for (int i = 0;i<NUMBER_OF_LEDS;i++) {
		uint16_t value = 0;
		
		int16_t idx = i;
		if (eyes[idx] != 0)
			value = eyes[idx];
		else
			value = uint16_t(soundWave[idx]) + uint16_t(tilt[idx]);

		value = constrain(value,0,255);
		values[i] = compensateCharacteristicCurve(value);
	}
	
	// send final values to LED driver
	sendLEDPWMValues();
}	
	
			
void LEDController::sendLEDPWMValues() {
	for (int i = 0;i<NUMBER_OF_LEDS;i++)
		setLEDPwm(i,values[i]);
}

void LEDController::computeTiltEffect() {
	 // go to tilt Angle
	 tiltAngle= normDegree(tiltAngle);

	 // set values in tilt array
	int16_t bright = (tiltBrightness*tiltStrength) >>4;
	bright = constrain(bright,0,255);
	int16_t maxDist =  ((TILT_WIDTH/2)*tiltStrength) >> 4;

	 for (int i = 0;i<NUMBER_OF_LEDS;i++) {
		 bool turnPos;
 		int16_t degree = (i * 360)/NUMBER_OF_LEDS + LED_ZERO_POSITION;
		int16_t dist = distanceDegree(degree,tiltAngle,turnPos);
		if (dist< maxDist)
			tilt[i] = (int16_t(bright)*(maxDist - dist)) / maxDist;
		else
			tilt[i] = 0;
	 }		
}

// set how eyes are supposed to look like
void LEDController::setEyes (	uint16_t pEyeDistance /* degrees */, 
								uint16_t pEyeWidth /* mm */,
								uint16_t pMiddlePosition /* degrees where the eyes are */, 
								uint8_t  pEyesBrightness /* brightness of eyes 0..255 */,
								bool  pKillerEyes) {
	eyeToBeDistance = pEyeDistance;
	eyeToBeWidth	= pEyeWidth;
	eyeToBePosition = normDegree(pMiddlePosition);
	eyeBrightness = pEyesBrightness;
	eyeKillerLook = pKillerEyes;
}

// blink now a couple of times
void LEDController::blinkNow(uint8_t pTimes) {	
	blinkNowTimes = pTimes;
}


void LEDController::computeEyes() {
	// now move eyes to designated position for next loop
	bool pTurnPos;
	int16_t dist = distanceDegree(eyeCurrentPosition,eyeToBePosition, pTurnPos);
	dist = constrain(dist,-EYE_SPEED,EYE_SPEED);
	if (pTurnPos)
		eyeCurrentPosition += dist;
	else
		eyeCurrentPosition -= dist;

	// increase/descrease eye width
	eyeCurrentWidth = eyeToBeWidth;
	int16_t eyeWidthDist = abs(int16_t(eyeToBePosition)-int16_t(eyeCurrentWidth));
	int16_t eyeWidthSpeed = constrain(eyeWidthDist,-EYE_WIDTH_SPEED, EYE_WIDTH_SPEED);
	if (eyeCurrentWidth > eyeToBeWidth)
		eyeCurrentWidth -= eyeWidthSpeed;
	else
		eyeCurrentWidth += eyeWidthSpeed;

	// increase/descrease eye distance
	eyeCurrentDistance = eyeToBeDistance;
	int16_t eyeDistanceDist = abs(eyeToBeDistance-eyeCurrentDistance);
	int16_t eyeDistanceSpeed = constrain(eyeDistanceDist, -EYE_DISTANCE_SPEED,EYE_DISTANCE_SPEED);
	if (eyeCurrentDistance > eyeToBeDistance)
		eyeCurrentDistance -= eyeDistanceSpeed;
	else
		eyeCurrentDistance += eyeDistanceSpeed;


	// are we starting a blinking?
	// blinking starts after eyeblinkungtime or if blinkNow has been called
	// which sets blinkNowTimes
	uint16_t passed_ms;
	uint16_t eyeBlinkingFactor = 255;
	if (eyeBlinkingPhaseTime == 0) { // currently no blinking is in rogress
		if (eyeBlinkingTimer.isDue_ms(eyeBlinkingTime, passed_ms)) { // time for start blinkng
			eyeBlinkingPhaseTime = millis();// set the time the blinking has been started
		} else {
			if (blinkNowTimes > 0) { // individually blinking via blinkNow() has been initiated
				blinkNowTimes--;				
				eyeBlinkingPhaseTime = millis(); // set time the blinking starts
			}
		}
	}		
	
	// if we are in a blinking phase, get the time since it started are compute the brightness factor
	if (eyeBlinkingPhaseTime>0) {
		passed_ms = millis() - eyeBlinkingPhaseTime; // time since blinking started
		
		 // start blinking phase
		if (passed_ms<EYE_BLINKING_TIME) {
			// start of blinking reduce brightness
			eyeBlinkingFactor = (255*(EYE_BLINKING_TIME-passed_ms)) / EYE_BLINKING_TIME;
		}
		else {
			if (passed_ms<EYE_BLINKING_TIME + EYE_BLINKING_DARK_TIME) 
				// dark phase, eyes are dark
				eyeBlinkingFactor = 0;
			else {
				if (passed_ms<2*EYE_BLINKING_TIME + EYE_BLINKING_DARK_TIME)
					// end of blinking, increase brightness
					eyeBlinkingFactor = (255*(passed_ms-(EYE_BLINKING_TIME+EYE_BLINKING_DARK_TIME))) / (EYE_BLINKING_TIME + EYE_BLINKING_DARK_TIME);
				else {
					eyeBlinkingFactor = 255;
					if (passed_ms<2*EYE_BLINKING_TIME + EYE_BLINKING_DARK_TIME + EYE_BLINKING_PAUSE) {
						// non-blinking pause after blinking is over, compute next blinking time
						eyeBlinkingTime = EYE_BLINKING_TIME + EYE_BLINKING_DARK_TIME + EYE_BLINKING_PAUSE + random(0,EYE_BLINKING_RND);
						eyeBlinkingPhaseTime = 0; // ok, it's done
						// if (blinkNowTimes == 0)
						// 	eyePanicMode = false; // panic mode is switched off as well, is a one shot only
					}					
				}
			}		
		}				
	}
		
	// print the eyes to the value array
	int16_t leftEye = normDegree(eyeCurrentPosition-eyeCurrentDistance/2);
	int16_t rightEye = normDegree(eyeCurrentPosition+eyeCurrentDistance/2);

	int16_t eyeHalfWidth = eyeCurrentWidth/2;
	for (int i = 0;i<NUMBER_OF_LEDS;i++) {
		
		eyes[i] = 0;		

		int16_t degree = (i * 360)/NUMBER_OF_LEDS - LED_ZERO_POSITION;
		int16_t distLeftEye = distanceDegree(degree,leftEye,pTurnPos);
		int16_t distRightEye = distanceDegree(degree,rightEye,pTurnPos);

		if (eyeKillerLook) {
			if (distLeftEye<eyeHalfWidth)
				eyes[i] += eyeBrightness;
			if (distRightEye<eyeHalfWidth)
				eyes[i] += eyeBrightness;	
		}
		else {
			if (distLeftEye<eyeHalfWidth)
				eyes[i] += (eyeBrightness*(eyeHalfWidth-distLeftEye)) / (eyeHalfWidth);
			if (distRightEye<eyeHalfWidth)
				eyes[i] += (eyeBrightness*(eyeHalfWidth-distRightEye)) / (eyeHalfWidth);	
		}		

		
		// multiply by blinking factor
		eyes[i] = (eyeBlinkingFactor * eyes[i]) >> 8;
	}
}

int16_t LEDController::distanceDegree(int16_t pDegree1,int16_t pDegree2, bool &pTurnPos) {
	pDegree1 = normDegree(pDegree1);
	pDegree2 = normDegree(pDegree2);
	int16_t dist;
	if (pDegree2 >= pDegree1) {
		dist = pDegree2-pDegree1;
		pTurnPos = true;
		if (dist >= 180) {
			dist = 360-dist;
			pTurnPos = false;
		}
	} else {
		dist = pDegree1-pDegree2;
		pTurnPos = false;
		if (dist >= 180) {
			dist = 360-dist;
			pTurnPos = true;
		}
	}
	return dist;
}	

int16_t LEDController::normDegree(int16_t pDegree) {
	while (pDegree<0)
		pDegree += 360;
	while (pDegree>=360)
		pDegree -= 360;	
	return pDegree;
}


void LEDController::printMenuHelp() {
	Serial.println(F("LED menu"));
	Serial.println(F("q/w eye move"));
	Serial.println(F("a/s eye width"));
	Serial.println(F("y/x eye distance"));
	Serial.println(F("o/p tilt angle"));
	Serial.println(F("k/l tile strengt"));
	Serial.println(F("b/n/m big/small/0 sound peak"));

	Serial.println(F("h - this page"));	
	Serial.println(F("0 - main menu"));	

}

void LEDController::menu() {
	int16_t eyeDistance = 90;
	int16_t eyeWidth = 45;
	int16_t eyePosition  = 0; 
	int16_t angle = 0,tiltStrength = 0;
	while (true) {
		wdt_reset();
		if (Serial.available())
			if (Serial.available()) {
				char inputChar = Serial.read();
				switch (inputChar) {
					case '0':
						return;
						break;
					case 'h':
						printMenuHelp();
						break;
					case 'q':
						eyePosition --;
						break;
					case 'w':
						eyePosition ++;
						break;
					case 'a':
						eyeWidth --;
						break;
					case 's':
						eyeWidth++;
					case 'y':
						eyeDistance--;
						break;
					case 'x':
						eyeDistance++;
					case 'o':
						angle++;
						break;
					case 'p':
						angle--;
						break;
					case 'k':
						tiltStrength++;
						break;
					case 'l':
						tiltStrength--;
						break;
					case 'b':
						setSoundWaveAmplitude(255);
						break;
					case 'n':
						setSoundWaveAmplitude(127);
						break;
					case 'm':
						setSoundWaveAmplitude(0);
						break;

					
					default:					
					break;
				}
				setEyes(eyeDistance, eyeWidth, eyePosition, EYE_DEFAULTBRIGHTNESS, false);
				setTilt(angle, tiltStrength);
				setTiltBrightness(128);
				setSoundWaveBrightness(128);
				Serial.print(F("eye: dist="));
				Serial.print(eyeDistance);
				Serial.print(F(" width="));
				Serial.print(eyeWidth);
				Serial.print(F(" pos="));
				Serial.print(eyePosition);
				Serial.print(F(" tilt="));
				Serial.print(angle);
				Serial.print(F(" strength="));
				Serial.print(tiltStrength);

				Serial.println();
			}
		
		loop();
		ledCtrl.loop();
	}												
}	
	
	
void LEDController::test() {
	ledCtrl.setSoundWaveAmplitude(100);
	ledCtrl.setTilt(-4,2);
	delay(SAMPLE_TIME);
	ledCtrl.loop();
	ledCtrl.setSoundWaveAmplitude(10);
	ledCtrl.setTilt(1,2);

	delay(50);
	ledCtrl.loop();
	ledCtrl.setSoundWaveAmplitude(50);
	delay(50);
	ledCtrl.loop();
	ledCtrl.setSoundWaveAmplitude(5);
	delay(50);
	ledCtrl.loop();
	ledCtrl.setSoundWaveAmplitude(120);
	delay(50);
	ledCtrl.loop();
	ledCtrl.setSoundWaveAmplitude(30);
	delay(50);
	ledCtrl.loop();
	
}

	

	