/*
 * BotController.cpp
 *
 * Created: 29.12.2012 22:03:12
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "Kinematics.h"
#include <avr/wdt.h>
#include "IMU/IMUController.h"
#include <avr/eeprom.h>
#include "FixedPoint.h"
#include "BotController.h"
#include "MotorController.h"
#include "BallBotMemory.h"

BotController botCtrl;

void BotController::setSpeed(int16_fp3_t pSpeedX_fp3, int16_fp3_t pSpeedY_fp3, int16_fp3_t pOmega_fp3) {
	botCtrl.toBeOmega_fp3 = pOmega_fp3;
	botCtrl.toBeSpeedX_fp3= pSpeedX_fp3;
	botCtrl.toBeSpeedY_fp3= pSpeedY_fp3;
}

void BotController::getSpeed(int16_fp3_t &pSpeedX_fp3, int16_fp3_t &pSpeedY_fp3, int16_fp3_t &pOmega_fp3) {
	pOmega_fp3 = botCtrl.toBeOmega_fp3;;
	pSpeedX_fp3 = botCtrl.toBeSpeedX_fp3;
	pSpeedY_fp3 = botCtrl.toBeSpeedY_fp3;
}

void BotController::reset() {
	memory.persistentMem.ctrlConfig.initDefaultValues();
	init();
}


// print the weights of the state feedback control to serial
void BotController::printCalibrationData() {	
	memory.persistentMem.ctrlConfig.print();
}

// print the to-be speed &omega of the bot (received from remote control)
void BotController::printToBeData() {
	Serial.print(F("to be                   v=("));		 
	Serial.print(botCtrl.toBeSpeedX_fp3>>3);Serial.print(",");
	Serial.print(botCtrl.toBeSpeedY_fp3>>3);Serial.print(F(") omega="));
	Serial.print(botCtrl.toBeOmega_fp3>>3);
	Serial.println();
}

void BotController::getFilteredAngle(int16_t &pTiltX_fp9, int16_t &pTiltY_fp9) {

	pTiltX_fp9 = tiltX_FP9;
	pTiltY_fp9 = tiltY_FP9;
}

void BotController::getFilteredAccel(int16_t &pAccelX, int16_t &pAccelY) {

	pAccelX = x.accel_fp0_filtered;
	pAccelY = y.accel_fp0_filtered;
}

void BotController::getAbsolutePosition(int16_t &pPosX, int16_t &pPosY) {
	pPosX = x.absBodyPos_fp8 >> 8;
	pPosY = y.absBodyPos_fp8 >> 8;

}	
// called when the control loop begins, waits until looptime is reached,
// detects the duration since last loop and resulting frequency
void BotController::waitForIMUData(	int16_t &pPassed_us, int16_t &pFrequency, 
									int16_fp9_t & pTiltX_FP9, int16_fp9_t &pTiltY_FP9, 
									int16_fp7_t &pAngularSpeedX_fp7,int16_fp7_t &pAngularSpeedY_fp7) {
	int16_fp9_t tiltX_fp9;
	int16_fp9_t tiltY_fp9;
	int16_fp7_t angularSpeedX_fp7,angularSpeedY_fp7;
	
	// fetch raw data from IMU via I2C
	imu.fetchRawData();
	
	// apply kalman filter
	imu.getFiltered(tiltX_fp9,tiltY_fp9,angularSpeedX_fp7,angularSpeedY_fp7);
	
	// compute duration and frequency
	static uint32_t lastCall_us = microseconds()-1000000L/IMU_SAMPLE_FREQ;		
	uint32_t now_us = microseconds();

	pPassed_us = (now_us-lastCall_us);
	lastCall_us = now_us;
	pFrequency = 1000000L/pPassed_us;

	pTiltX_FP9 = tiltX_fp9;
	pTiltY_FP9 = tiltY_fp9;	
	tiltX_FP9 = tiltX_fp9;
	tiltY_FP9 = tiltY_fp9;
	
	pAngularSpeedX_fp7 = angularSpeedX_fp7;
	pAngularSpeedY_fp7 = angularSpeedY_fp7;
}


// carry out state feedback control on base of 
// body position, body speed, body acceleration, head position, head position, head acceleration.
// To lean into curves, omega is used to compensate the centripedal force 
int16_fp3_t ControlPane::getNewSpeed(int16_t pFrequency, int16_fp19_t pLooptime_s_fp19, 
									 int16_t pActualSpeed_fp3, int16_fp3_t pToBeSpeed_fp3, 
									 int16_t pActualOmega_fp3, int16_fp3_t pToBeOmega_fp3, 
									 int16_fp3_t pTilt_fp9, int16_fp7_t pAngularSpeed_fp7) {
	// compute ABSOLUTE position where the bot really is
	absBasePos_fp8 += mul16s_rsh(pActualSpeed_fp3,pLooptime_s_fp19,14);

	// compute ABSOLUTE position of the body
	absBodyPos_fp8 = absBasePos_fp8 + mul16s_rsh(sin_FP9(pTilt_fp9),COG_HEIGHT,6); // sin_fp9 returns fp14;

	// compute difference since body position of last loop
	int16_t bodyPosDelta_fp8 = int16_t(absBodyPos_fp8 - lastAbsBodyPos_fp8);

	// compute ABSOLUTE position where we expect the bot to be 
	toBeAbsBasePos_fp8 += mul16s_rsh(pToBeSpeed_fp3,pLooptime_s_fp19,14);

	// compute to-be acceleration the robot is supposed to have
	int16_t toBeAbsAccel_fp0 = mul16s_rsh(pToBeSpeed_fp3-lastToBeSpeed_fp3,pFrequency,3);

	// compute the ABSOLUTE velocity the robot's body really has 
	int16_t bodyVelocity_fp3 = mul16s_rsh(bodyPosDelta_fp8,pFrequency,5);

	// compute the ABSOLUTE acceleration the robot's body really has 
	bodyAccel_fp0  = (int16_t)mul16s_rsh(bodyVelocity_fp3-lastBodyVelocity_fp3, pFrequency,3);	

	// compute the ABSOLUTE acceleration the robot's base really has 
	baseAccel_fp0 = mul16s_rsh(pActualSpeed_fp3-lastBaseVelocity_fp3, pFrequency,3);

	// compute the delta of absolute as-is and to-be position of the base (positive means tobe > as-is)
	errorBasePosition_fp4 = i32_rsh(toBeAbsBasePos_fp8-absBasePos_fp8,4);
	
	// compute the delta of absolute as-is and to-be position of the body
	errorBodyPosition_fp4 = i32_rsh(toBeAbsBasePos_fp8-absBodyPos_fp8,4);
	
	// compute the delta of the as-is and to-be speed (positive means tobe > as-is)
	errorBaseVelocity_fp3 = pToBeSpeed_fp3-pActualSpeed_fp3;

	// compute the delta of the as-is and to-be body speed (positive means tobe > as-is)
	errorBodyVelocity_fp3 = pToBeSpeed_fp3-bodyVelocity_fp3;

	// compute the difference between the as-is and to-be body acceleration (positive means tobe > as-is)
	errorBodyAccel_fp0 = toBeAbsAccel_fp0-bodyAccel_fp0; 

	// compute the difference between the as-is and to-be body acceleration (positive means tobe > as-is)
	errorBaseAccel_fp0 = toBeAbsAccel_fp0-baseAccel_fp0; 
	
	// compute error against centripedal force, which is f=omega*v*m*c, where m*c is the weight
	int16_t errorCentripedal_fp0 = mul16s_rsh(pToBeOmega_fp3, pToBeSpeed_fp3,6);

	// now multiply all deltas in each state variable with the according weight
	static int16_t tilt2error_fp5 = FLOAT2FP16(1000.0/(180.0/PI),5); // factor to convert degree in radian and [m] in [mm], 544
	int16_t error_tilt_fp0			= (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.angleWeight_fp8,mul16s_rsh(pTilt_fp9,tilt2error_fp5,9),13);	
	int16_t error_angular_speed_fp0	= (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.angularSpeedWeight_fp8,mul16s_rsh(pAngularSpeed_fp7,tilt2error_fp5,9),11);

	int16_t error_base_position_fp0 = (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.positionWeight_fp10,errorBasePosition_fp4,14);
	int16_t error_base_velocity_fp0 = (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.velocityWeight_fp10,errorBaseVelocity_fp3,13);
	int16_t error_base_accel_fp0	= (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.accelWeight_fp7,errorBaseAccel_fp0,7);

	int16_t error_body_position_fp0 = (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.bodyPositionWeight_fp10,errorBodyPosition_fp4,14);
	int16_t error_body_velocity_fp0 = (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.bodyVelocityWeight_fp10,errorBodyVelocity_fp3,13);
	int16_t error_body_accel_fp0	= (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.bodyAccelWeight_fp7,errorBodyAccel_fp0,7);

	int16_t error_centripedal_fp0	= (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.omegaWeight_fp7,errorCentripedal_fp0,7);

	// limit position and velocity error to that of 3° tilt error
	static int16_t positionMax_fp0 = (int16_t)mul16s_rsh(memory.persistentMem.ctrlConfig.angleWeight_fp8,mul16s_rsh(FLOAT2FP16(1.0,9),tilt2error_fp5,9),13);	

	error_base_position_fp0 = constrain(error_base_position_fp0, -positionMax_fp0, positionMax_fp0);
	error_body_position_fp0 = constrain(error_body_position_fp0, -positionMax_fp0, positionMax_fp0);
	
	// sum up all weighted errors
	int16_t error_fp0 = -error_tilt_fp0 - error_angular_speed_fp0		
						-error_base_velocity_fp0 - error_base_position_fp0 - error_base_accel_fp0
						-error_body_velocity_fp0 - error_body_position_fp0 - error_body_accel_fp0
						-error_centripedal_fp0;

	// 10° represents maximum error, which gives an accell of 4000 mm/s2
	accel_fp0 = constrain(error_fp0,-FP(8000,0),FP(8000,0));
	
	// accelerate only if not yet on max speed
	if ((sign(speed_fp3) != sign(accel_fp0)) ||
		(abs(speed_fp3) < FP(MAX_SPEED,3)))
		speed_fp3 += mul16s_rsh(accel_fp0,pLooptime_s_fp19,16);
	
	lastAbsBodyPos_fp8 = absBodyPos_fp8;
	lastBodyVelocity_fp3 = bodyVelocity_fp3;
	lastBaseVelocity_fp3 = pActualSpeed_fp3;
	lastToBeSpeed_fp3 = pToBeSpeed_fp3;
	
	// filtered acceleration to be passed to main controller
	accel_fp0_filtered = (int16_t)i32_rsh(	mul16s(FLOAT2FP16(0.02,8),		 accel_fp0) + 
										    mul16s((1<<8)-FLOAT2FP16(0.02,8),accel_fp0_filtered),8);
	return speed_fp3;
}

// print current position/speed to Serial.
void ControlPane::print() {
	Serial.print(F(" pos="));		
	Serial.print(FP2FLOAT(errorBasePosition_fp4,4),1,4);		
	Serial.print(F(" v="));		
	Serial.print(FP2FLOAT(errorBaseVelocity_fp3,3),0,5);		
	Serial.print(F(" bpos="));		
	Serial.print(FP2FLOAT(errorBodyPosition_fp4,4),1,4);		
	Serial.print(F(" bv="));		
	Serial.print(FP2FLOAT(errorBodyVelocity_fp3,3),0,5);		

	Serial.print(F(" e="));
	Serial.print(FP2FLOAT(error_fp0,0),2,5);		
	Serial.print(F(" a="));		
	Serial.print(FP2FLOAT(accel_fp0,0),0,7);		
	Serial.print(F(" v="));		
	Serial.print(FP2FLOAT(speed_fp3,3),0,5);		
}

// loop of balancing 
void BotController::loop(boolean pBalanceOn) {		
	// start loop. Wait until 7ms passed since last call and call IMU 
	// every 1ms to update the kalman  filters (IMU works at 1KHz)
	// loop time is 6ms, 160Hz
	waitForIMUData(looptime_us, frequency_Hz,tiltX_FP9,tiltY_FP9,angularSpeedX_fp7,angularSpeedY_fp7);
	
	// if we do not balance, just measure the angles and quit
	if (!pBalanceOn)
		return;
	
	// compute dT since last loop in [s]
	int16_fp19_t looptime_s_fp19 = mul16s_rsh(looptime_us, (1UL<<30)/1000000UL,11); // loop time in [s], 1ms = 2096
		
	// fetch motor encoder values in [°] to compute real bot position 
	int16_fp8_t angleChange_fp8[WHEELS] = {0,0,0};
	mCtrl.getWheelAngleChange(angleChange_fp8);									// get change since last invocation

	int16_t actWheelSpeed_fp4[WHEELS] = {0,0,0};
	for (uint8_t w = 0;w<WHEELS;w++) 
		actWheelSpeed_fp4[w] = mul16s_rsh(angleChange_fp8[w],frequency_Hz,4);	// compute wheel speed out of delta-angle

	// apply inverse kinematics to get { speed (x,y), omega } out of wheel speed
	// tilt angles are not passed, since the tilt-corrected rotation matrix of the 
	// last forward kinematics operation is stored, transposed and used here. Kind of
	// an ugly side-effect, we don't get a beauty award for this.
	// If we would be paranoid, we should use the current tilt angles instead  
	int16_fp3_t actualSpeedX_fp3,actualSpeedY_fp3;
	int16_fp3_t actualOmega_fp3;
	kin.computeActualSpeed( actWheelSpeed_fp4, 
						tiltX_FP9,tiltY_FP9,		 
						actualSpeedX_fp3, actualSpeedY_fp3, actualOmega_fp3);
						
	// compute new velocity out of current angle, angular velocity, velocity, position
	x.speed_fp3 = x.getNewSpeed(frequency_Hz, looptime_s_fp19, actualSpeedX_fp3, toBeSpeedX_fp3, actualOmega_fp3, toBeOmega_fp3, tiltX_FP9, angularSpeedX_fp7);
	y.speed_fp3 = y.getNewSpeed(frequency_Hz, looptime_s_fp19, actualSpeedY_fp3, toBeSpeedY_fp3, actualOmega_fp3, toBeOmega_fp3, tiltY_FP9, angularSpeedY_fp7);
			
	int16_fp3_t newOmega_fp3 = toBeOmega_fp3;

	// apply kinematics to compute wheel speed out of x,y, omega
	int16_fp4_t newWheelSpeed_fp4[WHEELS];
	kin.computeWheelSpeed(  x.speed_fp3, y.speed_fp3, newOmega_fp3, 
							0 /* tiltX_FP8 */, 0 /*tiltY_FP8 */,
							newWheelSpeed_fp4);		
							
	// send new speed to motors 
	mCtrl.setWheelSpeed(newWheelSpeed_fp4, looptime_us,frequency_Hz);		
}

void BotController::printLoopData() {
	static uint8_t i = 0;
	if (i++ == IMU_SAMPLE_FREQ) {
			i =0;
	
		Serial.print(F(" f="));		
		Serial.print(frequency_Hz);		
		Serial.print(F("Hz"));		
		Serial.print(F(" tilt=("));		
		Serial.print(FP2FLOAT(tiltX_FP9,9),1,2);		
		Serial.print(",");		
		Serial.print(FP2FLOAT(tiltY_FP9,9),1,2);		
		Serial.print("/");		
		Serial.print(FP2FLOAT(angularSpeedX_fp7,7),0,4);		
		Serial.print(",");		
		Serial.print(FP2FLOAT(angularSpeedY_fp7,7),0,4);		
		Serial.print(")");		

		Serial.print(F(" X-Pane:"));	
		x.print();
		Serial.print(F(" Y-Pane:"));	
		y.print();
		Serial.println();
	}
}	

void BotController::printMenuHelp() {
	Serial.println(F("Bot Control Menu"));
	Serial.println(F("u/i - +/- of AngleWeight"));
	Serial.println(F("n/m - +/- of AngularSpeedWeight"));

	Serial.println(F("k/l - +/- of BasePositionWeight  K/L - Body"));
	Serial.println(F("o/p - +/- of Base VelocityWeight O/P - Body"));
	Serial.println(F("c/v - +/- of Accel Weight		   C/V - Body"));
	Serial.println(F("y/x - +/- of Omega Weight"));

	Serial.println(F("r   reset movement"));
	Serial.println(F("R   reset all weights to 0"));
	Serial.println(F("b   mobbing mode on/off"));

	Serial.println();
	Serial.println(F("f/g - +/- of Velocity in x"));
	Serial.println(F("t/z - +/- of Velocity in y"));
	Serial.println(F("q/w - +/- of omega in z"));

	Serial.println();
	Serial.println(F("0 - return to main menu"));	
}

bool BotController::dispatchCommand(char pChar, bool &pQuit) {

	pQuit = false;
	switch (pChar) {
		case 'h':
			printMenuHelp();
			break;
		case 'b': botCtrl.mobbingModeOn = !botCtrl.mobbingModeOn; break;
		case 'u': memory.persistentMem.ctrlConfig.angleWeight_fp8+=FLOAT2FP16(0.5,8);break;
		case 'i': memory.persistentMem.ctrlConfig.angleWeight_fp8-=FLOAT2FP16(0.5,8);break;
		case 'n': memory.persistentMem.ctrlConfig.angularSpeedWeight_fp8+=FLOAT2FP16(0.1,8);break;
		case 'm': memory.persistentMem.ctrlConfig.angularSpeedWeight_fp8-=FLOAT2FP16(0.1,8);break;
		
		case 'k': memory.persistentMem.ctrlConfig.positionWeight_fp10+=FLOAT2FP16(0.1,10);break;
		case 'l': memory.persistentMem.ctrlConfig.positionWeight_fp10-=FLOAT2FP16(0.1,10);break;
		case 'o': memory.persistentMem.ctrlConfig.velocityWeight_fp10+=FLOAT2FP16(0.1,10);break;
		case 'p': memory.persistentMem.ctrlConfig.velocityWeight_fp10-=FLOAT2FP16(0.1,10);break;
		case 'c': memory.persistentMem.ctrlConfig.accelWeight_fp7+=FLOAT2FP16(0.1,7);break;
		case 'v': memory.persistentMem.ctrlConfig.accelWeight_fp7-=FLOAT2FP16(0.1,7);break;
		case 'K': memory.persistentMem.ctrlConfig.bodyPositionWeight_fp10+=FLOAT2FP16(0.1,10);break;
		case 'L': memory.persistentMem.ctrlConfig.bodyPositionWeight_fp10-=FLOAT2FP16(0.1,10);break;
		case 'O': memory.persistentMem.ctrlConfig.bodyVelocityWeight_fp10+=FLOAT2FP16(0.1,10);break;
		case 'P': memory.persistentMem.ctrlConfig.bodyVelocityWeight_fp10-=FLOAT2FP16(0.1,10);break;
		case 'C': memory.persistentMem.ctrlConfig.bodyAccelWeight_fp7+=FLOAT2FP16(0.1,7);break;
		case 'V': memory.persistentMem.ctrlConfig.bodyAccelWeight_fp7-=FLOAT2FP16(0.1,7);break;
		case 'x': memory.persistentMem.ctrlConfig.omegaWeight_fp7+=FLOAT2FP16(0.05,7);break;
		case 'y': memory.persistentMem.ctrlConfig.omegaWeight_fp7-=FLOAT2FP16(0.05,7);break;

		case 'r': 
			botCtrl.init();
			break;
		case 'R': 
			botCtrl.init();
			memory.persistentMem.ctrlConfig.null();

			break;
		case 'd': 
			reset();
			break;
		case 'f': botCtrl.toBeSpeedX_fp3+=FLOAT2FP16(3,4);break;
		case 'g': botCtrl.toBeSpeedX_fp3-=FLOAT2FP16(3,4);break;
		case 't': botCtrl.toBeSpeedY_fp3+=FLOAT2FP16(3,4);break;
		case 'z': botCtrl.toBeSpeedY_fp3-=FLOAT2FP16(3,4);break;
		case 'q': botCtrl.toBeOmega_fp3+=FLOAT2FP16(3,4);break;
		case 'w': botCtrl.toBeOmega_fp3-=FLOAT2FP16(3,4);break;
		case '0':
			mCtrl.stop();
			pQuit = true;
			break;
		default:
			return false; // no command found
			break;
		}	
		
		printCalibrationData();	 
		printToBeData();
	return true; // command parsed
}