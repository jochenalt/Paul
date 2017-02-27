/*
 * MotorController.cpp
 *
 * Created: 09.12.2012 17:54:53
 * Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "FixedPoint.h"

#include <avr/wdt.h>
#include "avr/eeprom.h"
#include "Motor.h"
#include "MotorController.h"

#include "BallBotMemory.h"


// global variable to access MotorController
MotorController mCtrl;

// setup the motor controller
void MotorController::setup() {
	mCtrl.stop();
}

// factor that reduces PI terms with increasing speed. In my setting, it turned out, that especially P term has to be decreased 
// with increasing speed in order to reach a smooth speed regulation.
// empirically identified, maximum pTerm/iTerm at lowest Speed (maximum control), reduced to 4% (10/255) at max speed
// pSpeed is 0..256, returns 0..256 reduction factor
int16_t MotorController::termReductionBySpeed(int16_t pSpeed) {

	// that's the array how speed is going to be reduced in its interval from 0..255
	static uint8_t termCurve[17] = {240, 255, 245, 235, 225, 215, 205, 195, 185, 180, 175, 170, 165, 160, 155, 150,145};

	// interpolate the speed according to termCurve
	pSpeed = abs(pSpeed);
	uint8_t pos = pSpeed >> 4;
    uint8_t left = 	termCurve[pos++];
	uint8_t right = termCurve[pos];
	int16_t x = pSpeed- (pSpeed & 0xF0);
	return left + ((x*(right-left))>>4);
}

// adds the change of encoders since last invocation to pAngleChange_fp8 [°]
void MotorController::getWheelAngleChange(int16_fp8_t pAngleChange_fp8[WHEELS]) {
		
	for (uint8_t wheelNo=0;wheelNo<WHEELS;wheelNo++) {
		currEncoder[wheelNo] = 0;

		// current encoder value
		motor[wheelNo].getEncoderCount(mCtrl.currEncoder[wheelNo]);
		
		// with a sampling rate around 200Hz, and 928 pulses/revolution we need 15 cycles to get one encoder impulse 
		// at a speed of e.g. 20°/s. Therefore the output is quite notchy and results in a ugly sounding motor.
		// To solve that, we use a moving average of 32 samples, i.e. of the last 160ms
		int16_fp8_t sample_fp8 = movAvr[wheelNo].getAverage_fp8(mCtrl.currEncoder[wheelNo]); 

		// return the encoder positions converted into angular speed
		static int16_t encoder2degree_fp16 = (360L << 16)/ENCODER_PULES_PER_REV; // = 25423
		pAngleChange_fp8[wheelNo]	= mul16s_rsh(sample_fp8,encoder2degree_fp16,16);
	} // for all wheels
}

// stop all motors
void MotorController::stop() {
	currEncoder[0] = 0;
	currEncoder[1] = 0;
	currEncoder[2] = 0;
	lastTargetEncoder_fp8 = {0,0,0}; // target encoder position of last loop
	integratedError_fp8 = {0,0,0};   // diff between encoder as-is position and to-be position

	motor[0].drive(0);
	motor[1].drive(0);
	motor[2].drive(0);
}	
		
// set motor speed in °/s
// pPassedus is time in [us] since previous invocation of setWheelSpeed
// pFrequency is 1000/pPassedus (to increase performance reuse this, the division has been done earlier)
void MotorController::setWheelSpeed( int16_fp4_t pOmegaWheel_fp4[WHEELS], int16_t pPassedus, int16_t pFrequency) {
	
	// encoder values have been stored in gCurrEncoder[WHEELS] in previous call of getWheelAngleChange
	for (uint8_t wheelNo=0;wheelNo<WHEELS;wheelNo++) {
		pOmegaWheel_fp4[wheelNo] = constrain(pOmegaWheel_fp4[wheelNo],-(MAX_REVOLUTIONS<<4),+(MAX_REVOLUTIONS<<4));

		// for later computations, save encoder position 
		int32_t currWheelEncoder_fp8 = currEncoder[wheelNo]<<8;

		// something unusual in a PI controller: pTerm and dTerm have to be reduced with increasing speed, 
		// otherwise motor tends to overshot. Heuristic tries endded up in the definition of termReductionBySpeed
		static int16_t max_revolution_rezi_fp_18 = (1L<<18) / MAX_REVOLUTIONS; // == 218, looks strange, but avoids division during runtime
		int16_t speedfactor = mul16s_rsh(pOmegaWheel_fp4[wheelNo],max_revolution_rezi_fp_18,14); // 0..256
		int16_fp8_t reduction_fp6 = termReductionBySpeed(speedfactor) >> 2;

		// compute target encoder values and subtract from current encoder values (that's the PI error)
		// gearbox is 1:29, encoder delivers 32 pules per 360° at the motors axis, so 
		// we have 29*32=928 impulses per pulses. Passed parameter is °/s, so we have 32*29/360 impules per °
		// to get the target encoder position the angular speed is multiplied with the sample rate 
		static int16_t omega2encoder_fp30 = int16_t((float(ENCODER_PULES_PER_REV))/360.0/1000000.0 * (1L<<30)); // = 2767
		int32_fp8_t targetEncoder_fp8 = mul16s_rsh(pOmegaWheel_fp4[wheelNo],mul16s_rsh(pPassedus,omega2encoder_fp30,12),14);

		// compute error of encoder position = target - current
		int16_t error_fp4 = i32_rsh(targetEncoder_fp8 - currWheelEncoder_fp8, 4);

		// integrated error is error at start time of this function, so use previous target encoder position instead of current
		integratedError_fp8[wheelNo] += (currWheelEncoder_fp8 - lastTargetEncoder_fp8[wheelNo]);

		// compute PID term
		int16_t lKp_fp8 = mul16s_rsh(memory.persistentMem.motorPICtrlConfig.Kp_fp8,reduction_fp6,6);
		int16_fp4_t pTerm_fp4 = mul16s_rsh(lKp_fp8,error_fp4,4);
 
		// reduce Ki_fp8 with increasing speed 
		int16_t lKi_fp8 = mul16s_rsh(memory.persistentMem.motorPICtrlConfig.Ki_fp8,reduction_fp6,6);;
		int16_fp4_t iTerm_fp4 = mul16s_rsh(lKi_fp8,i32_rsh(integratedError_fp8[wheelNo],8),4);
				
		// compute torque in unit of encoder impulses
		int16_t encoder_pulses_fp4 = pTerm_fp4 - iTerm_fp4;
		
		// convert into motor torque from 0..255
		static int16_t encoder2torque_fp16 = int16_t((360.0*255.0 * (1L<<16)) / (ENCODER_PULES_PER_REV * float(MAX_REVOLUTIONS))); // =3601
		int16_t encoder2torque_fp8 = mul16s_rsh(encoder2torque_fp16,pFrequency,8); // = 2250 bei 160Hz
		int16_t torque_fp0 = mul16s_rsh(encoder_pulses_fp4,encoder2torque_fp8,12);
		torque_fp0 = constrain(torque_fp0,-255,255);

		// store last target Encoder for next invocation
		lastTargetEncoder_fp8[wheelNo] = targetEncoder_fp8;

		motor[wheelNo].drive(torque_fp0);
	} // for all wheels	
}

//  print the help text of the menu to Serial.
void MotorController::printMenuHelp() {
	Serial.println(F("Motor controller - PI tuning "));
	Serial.println(F("123 - select wheel"));
	Serial.println(F("q/w - inc/dec/0 pTerm"));
	Serial.println(F("a/s - inc/dec/0 iTerm"));
	Serial.println(F("+/- - inc/dec/ speed"));
	Serial.println(F("l   - play speed programme"));
	Serial.println(F("h   - this page"));
	Serial.println(F("0   - return to main menu"));
}

// menu to calibrate and test the motor controller. Works on Serial.
// BTW, the typical hint you find in the net to tune a PID controller is:
//
//	For manual tuning, set all three constants to zero, then turn up Kp until oscillation occurs.
//	Then turn up Kd until oscillation disappears. Adjust Kd until the system is critically damped, i.e. there’s no overshoot.
//	Then increase Ki until the steady-state error goes to zero in a reasonable time.
void MotorController::menu() {
	
	// ok, this method does not deserve a beauty award, but it's for testing only...
	printMenuHelp();
	int16_t passedus;
	int16_t frequency;
	int16_fp4_t omegaWheel_fp4[3];
	int16_t actualAngle_fp8[3];
	actualAngle_fp8[0] = 0;
	actualAngle_fp8[1] = 0;
	actualAngle_fp8[2] = 0;
	omegaWheel_fp4[0] = FLOAT2FP16(0,4);
	omegaWheel_fp4[1] = FLOAT2FP16(0,4);
	omegaWheel_fp4[2] = FLOAT2FP16(0,4);
	
	int32_t actualEncoderPosition_fp8 = 0;
	int32_t tobeEncoderPosition_fp8 = 0;
	int32_t lastWheelSpeed_fp4 = 0;
		
	int16_t speed_prg = 0;
	int8_t wheelno = 0;
	while (true)  {
		wdt_reset();
		delay_ms(6);
		static uint32_t lastCallus = microseconds();
		uint32_t us = microseconds();
		passedus = (us-lastCallus);
		lastCallus = us;
		frequency = 1000000L/passedus;
		getWheelAngleChange(actualAngle_fp8);
		
		actualEncoderPosition_fp8 += actualAngle_fp8[wheelno];
		tobeEncoderPosition_fp8 += ((lastWheelSpeed_fp4*passedus)/1000000L)<<4;
			
		static uint32_t us_duration = 0;

		if (speed_prg > 0) {
			if (us_duration >=	2000000L) {
				 switch (speed_prg) {
					case 1: omegaWheel_fp4[wheelno] = FP(100,4);break;
					case 2: omegaWheel_fp4[wheelno] = FP(200,4);break;
					case 3: omegaWheel_fp4[wheelno] = FP(300,4);break;
					case 4: omegaWheel_fp4[wheelno] = FP(700,4);break;
					case 5: omegaWheel_fp4[wheelno] = FP(1000,4);break;
					case 6: omegaWheel_fp4[wheelno] = FP(200,4);break;
					case 7: omegaWheel_fp4[wheelno] = FP(-300,4);break;
					case 8: omegaWheel_fp4[wheelno] = FP(-1000,4);break;
					case 9: omegaWheel_fp4[wheelno] = FP(-100,4);break;
					case 10: omegaWheel_fp4[wheelno] = FP(-300,4);break;
					case 11: omegaWheel_fp4[wheelno] = FP(0,4);break;
					case 12: omegaWheel_fp4[wheelno] = FP(50,4);break;
					case 13: omegaWheel_fp4[wheelno] = FP(0,4);break;

					case 14: speed_prg = 0;
					default:
						break;
				}
				if (speed_prg != 0)
					speed_prg++;
			} 
		} 
		if (us_duration >= 2000000L) {
			Serial.print(F(" vi="));Serial.print(float(tobeEncoderPosition_fp8)/(1<<8)*float(2000000.0f)/us_duration,2,8);
			Serial.print(F(" vm="));Serial.print(float(actualEncoderPosition_fp8)/(1<<8)*float(2000000.0f)/us_duration,2,8);
			Serial.print(F(" vn="));Serial.print(float(omegaWheel_fp4[wheelno])/(1<<4),2,8);
			Serial.print(F(" mc="));Serial.print(us_duration,DEC);
			Serial.print(F(" enc="));Serial.print(actualEncoderPosition_fp8>>8,DEC);
			Serial.print(F(" us="));Serial.print(passedus,DEC);

			Serial.println();			
			actualEncoderPosition_fp8 = 0;
			tobeEncoderPosition_fp8= 0;
			us_duration = 0;
		}
		us_duration += passedus;
		
		// set new motor speed and gather the current encoder position 
		setWheelSpeed(omegaWheel_fp4, passedus, frequency);
		lastWheelSpeed_fp4 = omegaWheel_fp4[wheelno];

		if (Serial.available()) {
			static char inputChar;
			inputChar = Serial.read();
			switch (inputChar) {
				case '1': 
					wheelno = 0;
					break;
				case '2': 
					wheelno = 1;
					break;
				case '3': 
					wheelno = 2;
					break;

				case 'h': 
					printMenuHelp();
					break;
				case 'q': memory.persistentMem.motorPICtrlConfig.Kp_fp8+=4;break;
				case 'w': memory.persistentMem.motorPICtrlConfig.Kp_fp8-=4;break;

				case 'a': memory.persistentMem.motorPICtrlConfig.Ki_fp8++;break;
				case 's': memory.persistentMem.motorPICtrlConfig.Ki_fp8--;break;

				case '+': omegaWheel_fp4[wheelno] += FP(10,4);break;
				case '-': omegaWheel_fp4[wheelno] -= FP(10,4);break;
				case 'l': 
					if (speed_prg!=0) 
						speed_prg = 0; 
					else {
						speed_prg = 1;
					}					
					break;
				case '0': return;
			
				default:
					break;
			} // switch

			Serial.print(F("wheel="));Serial.print(wheelno);
			Serial.print(F(" p="));Serial.print(FP2FLOAT(memory.persistentMem.motorPICtrlConfig.Kp_fp8,8),2,2);
			Serial.print(F(" i="));Serial.print(FP2FLOAT(memory.persistentMem.motorPICtrlConfig.Ki_fp8,8),2,2);
			Serial.print(F(" v="));Serial.println(omegaWheel_fp4[wheelno]>>4,DEC);
		} // if (Serial.available()) 
	} // while true		 
}

// print the PI terms to Serial
void MotorController::printCalibrationData() {	
	memory.persistentMem.motorPICtrlConfig.print();
}	

void MotorPIController::print() {
		Serial.print(F("Motor PI Controller   : PI=("));
		Serial.print(FP2FLOAT(memory.persistentMem.motorPICtrlConfig.Kp_fp8,8),3,1);Serial.print(",");
		Serial.print(FP2FLOAT(memory.persistentMem.motorPICtrlConfig.Ki_fp8,8),3,1);Serial.println(")");
}
