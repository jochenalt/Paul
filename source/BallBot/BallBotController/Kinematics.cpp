/*
 * Kinematics.cpp
 *
 * Created: 30.11.2012 16:23:25
 * Author: JochenAlt
 */ 


#include "Arduino.h"
#include "setup.h"
#include "FixedPoint.h"
#include "MotorController.h"
#include "Kinematics.h"
#include <avr/wdt.h> 

Kinematix kin;


// true, if the direction of one coordinate is supposed to be reversed
// used to align the IMUs coordinate system with the motor platforms coord system
bool Kinematix::reverseDirection[3] = { true, true, false}; // x, y, omega
	
// return the construction matrix (CM) and compute its inverse matrix used for inverse kinematics
// for conveniency use floats, since this matrix is computed during startup only
void Kinematix::setupConstructionMatrix() {
		float a = -1.0/WHEEL_RADIUS;
		float cos_phi = cos(WHEEL_ANGLE/180.0*PI);
		float sin_phi = sin(WHEEL_ANGLE/180.0*PI);
		

		MATRIX33_SET_LINE(cm_fp19, 0, 
			FLOAT2FP16(0,19),FLOAT2FP16(a*cos_phi,19), FLOAT2FP16(-a*sin_phi,19));
		MATRIX33_SET_LINE(cm_fp19, 1, 
			FLOAT2FP16(a*sqrt(3)/-2*cos_phi,19), FLOAT2FP16(a*cos_phi/-2, 19), cm_fp19[0][2]);
		MATRIX33_SET_LINE(cm_fp19, 2, 
			-cm_fp19[1][0], cm_fp19[1][1], cm_fp19[1][2]);

		// now compute the inverse matrix in fp8 on base of its determinant
		float det_denominator = 
					     (float(cm_fp19[0][0])*cm_fp19[1][1] * cm_fp19[2][2]) +
 			             (float(cm_fp19[0][1])*cm_fp19[1][2] * cm_fp19[2][0]) + 
			             (float(cm_fp19[0][2])*cm_fp19[1][0] * cm_fp19[2][1]) -
			             (float(cm_fp19[2][0])*cm_fp19[1][1] * cm_fp19[0][2]) -
			             (float(cm_fp19[2][1])*cm_fp19[1][2] * cm_fp19[0][0]) -
			             (float(cm_fp19[2][2])*cm_fp19[1][0] * cm_fp19[0][1]);

		// compute inverse matrix in fp8
		// required in inverse kinematics
		float detRezi = float(1L<<19) * float(1L<<8) / float(det_denominator);
		MATRIX33_SET_LINE(icm_fp08, 0, 
			detRezi*((float(cm_fp19[1][1]) * cm_fp19[2][2] - float(cm_fp19[1][2]) * cm_fp19[2][1])),
			detRezi*((float(cm_fp19[0][2]) * cm_fp19[2][1] - float(cm_fp19[0][1]) * cm_fp19[2][2])),
			detRezi*((float(cm_fp19[0][1]) * cm_fp19[1][2] - float(cm_fp19[0][2]) * cm_fp19[1][1])));
		MATRIX33_SET_LINE(icm_fp08, 1, 
			detRezi*((float(cm_fp19[1][2]) * cm_fp19[2][0] - float(cm_fp19[1][0]) * cm_fp19[2][2])),
			detRezi*((float(cm_fp19[0][0]) * cm_fp19[2][2] - float(cm_fp19[0][2]) * cm_fp19[2][0])),
			detRezi*((float(cm_fp19[0][2]) * cm_fp19[1][0] - float(cm_fp19[0][0]) * cm_fp19[1][2])));
		MATRIX33_SET_LINE(icm_fp08, 2, 
			detRezi*((float(cm_fp19[1][0]) * cm_fp19[2][1] - float(cm_fp19[1][1]) * cm_fp19[2][0])),
			detRezi*((float(cm_fp19[0][1]) * cm_fp19[2][0] - float(cm_fp19[0][0]) * cm_fp19[2][1])),
			detRezi*((float(cm_fp19[0][0]) * cm_fp19[1][1] - float(cm_fp19[0][1]) * cm_fp19[1][0])));
}



// tiltRotationMatrix (TRM) is the rotation matrix that is able to compensate the position where
// the ball touches the ground. This points moves if the robot tilts, so when doing forward and inverse kinematics
// this angle needs to be taken into account when the wheel speed is computed out of x,y, omega
void Kinematix::computeTiltRotationMatrix(int16_fp8_t pTiltX_FP8, int16_fp8_t pTiltY_FP8) {
	
	// do it only if tilt angles have really changed
	// important, since forward and inverse kinematics per loop have the same angles.
	static bool alreadyComputed = false; // compute at least the first time, even if angles are all zero
	static int16_fp8_t lastTiltX_fp8, lastTiltY_fp8;
	
	if  (alreadyComputed && 
		(lastTiltX_fp8 == pTiltX_FP8) && 
		(lastTiltY_fp8 == pTiltY_FP8))
		return;
	
	alreadyComputed = true;
	lastTiltX_fp8 = pTiltX_FP8;		
	lastTiltY_fp8 = pTiltY_FP8;		
	
	// compute sin and cos, needed in rotation matrix
	int16_fp14_t sinX_fp14 = sin_FP8(pTiltY_FP8);
	int16_fp14_t cosX_fp14 = cos_FP8(pTiltY_FP8);
	int16_fp14_t sinY_fp14 = sin_FP8(pTiltX_FP8);
	int16_fp14_t cosY_fp14 = cos_FP8(pTiltX_FP8);

	// compute Tilt Rotation Matrix (TRM). All values are between -1..1, so use FP16
	// fixed point arithmetics. accuracy of TRM is better than 1%
	// computation is coming from kinematix.xls
	MATRIX33_SET_LINE(trm_fp14, 0, 
		cosY_fp14, 0, sinY_fp14);
	MATRIX33_SET_LINE(trm_fp14, 1, 
		mul16s_rsh(sinX_fp14, sinY_fp14,14), 
		cosX_fp14, 
		-mul16s_rsh(sinX_fp14, cosY_fp14,14));
	MATRIX33_SET_LINE(trm_fp14, 2, 
		-(mul16s_rsh(cosX_fp14,sinY_fp14,14)), 
		sinX_fp14, 
		mul16s_rsh(cosX_fp14,cosY_fp14,14));	
}

// compute speed of all motors depending from the speed in the IMU's coordinate system in (Vx, Vy, OmegaZ) 
// corrected by the tilt of the imu pTiltX, pTiltY 
void Kinematix::computeWheelSpeed( int16_fp3_t pVx_fp3, int16_fp3_t pVy_fp3, int16_fp3_t pOmegaZ_fp3, 
						int16_fp9_t pTiltX_FP9, int16_fp9_t pTiltY_FP9,
						int16_fp4_t pWheel_speed_fp4[WHEELS]) {
	// reverse the direction where necessary to align IMU coord system with wheel coord system
	int16_fp3_t lVx_fp3 = reverseDirection[0]?-pVx_fp3:pVx_fp3;
	int16_fp3_t lVy_fp3 = reverseDirection[1]?-pVy_fp3:pVy_fp3;
	int16_fp3_t lOmegaZ_fp3 = reverseDirection[2]?-pOmegaZ_fp3:pOmegaZ_fp3;
	
	// this matrix depends on the tilt angle and corrects the kinematics 
	// due to the slightly moved touch point of the ball
	kin.computeTiltRotationMatrix(pTiltX_FP9>>1,pTiltY_FP9>>1);

	// compute kinematics matrix
	// intermediate multiplications needed later on in fp17
	int16_fp17_t m01_11=  mul16s_rsh(cm_fp19[0][1],trm_fp14[1][1],16);	
	int16_fp17_t m01_21 = mul16s_rsh(cm_fp19[0][1],trm_fp14[2][1],16);
	int16_fp17_t m10_00 = mul16s_rsh(cm_fp19[1][0],trm_fp14[0][0],16);
	int16_fp17_t m10_10 = mul16s_rsh(cm_fp19[1][0],trm_fp14[1][0],16);
	int16_fp17_t m10_20 = mul16s_rsh(cm_fp19[1][0],trm_fp14[2][0],16);
	int16_fp17_t m11_11 = mul16s_rsh(cm_fp19[1][1],trm_fp14[1][1],16);
	int16_fp17_t m11_21 = mul16s_rsh(cm_fp19[1][1],trm_fp14[2][1],16);
	int16_fp17_t m02_02 = mul16s_rsh(cm_fp19[0][2],trm_fp14[0][2],16);
	int16_fp17_t m02_22 = mul16s_rsh(cm_fp19[0][2],trm_fp14[2][2],16);
	int16_fp17_t m02_12 = mul16s_rsh(cm_fp19[0][2],trm_fp14[1][2],16);


	// convert to radian
	int16_fp5_t lOmegaZRad_5fp =  -mul16s_rsh(lOmegaZ_fp3, FLOAT2FP32(PI/180.0,14) * (uint16_t(BALL_RADIUS)>>1),12-1) ;

	// compute wheel's speed in °/s
	int32_t wheelSpeed_fp4_0 = i32_rsh(((mul16s(m01_11 + m02_12,lVx_fp3)		  - mul16s(m02_02,lVy_fp3)          +(mul16s_rsh( m01_21 + m02_22,          lOmegaZRad_5fp,2))) * FLOAT2FP32 (180.0/PI,0)),16);
	int32_t wheelSpeed_fp4_1 = i32_rsh(((mul16s(m10_10 + m11_11 + m02_12,lVx_fp3) - mul16s(m10_00 + m02_02,lVy_fp3) +(mul16s_rsh( m10_20 + m11_21 + m02_22, lOmegaZRad_5fp,2))) * FLOAT2FP32 (180.0/PI,0)),16);
	int32_t wheelSpeed_fp4_2 = i32_rsh(((mul16s(-m10_10+ m11_11 + m02_12,lVx_fp3) + mul16s(m10_00 - m02_02,lVy_fp3) +(mul16s_rsh(-m10_20 + m11_21 + m02_22 ,lOmegaZRad_5fp,2))) * FLOAT2FP32 (180.0/PI,0)),16);

	// alarm: if one wheel is supposed to run faster than it can
	// reduce the speed of all wheels in proportion, so that the 
	// fastest wheel has max speed
	int32_t absWheelSpeed_fp4_0 = abs(wheelSpeed_fp4_0);
	int32_t absWheelSpeed_fp4_1 = abs(wheelSpeed_fp4_1);
	int32_t absWheelSpeed_fp4_2 = abs(wheelSpeed_fp4_2);

	// while one wheel's to-be speed exceeds max speed
	// compute reduction factor for fastest wheel and reduce 
	// speed of all wheels accordingly
	while ((absWheelSpeed_fp4_0 > MAX_WHEEL_SPEED) ||
		   (absWheelSpeed_fp4_1 > MAX_WHEEL_SPEED) ||
		   (absWheelSpeed_fp4_2 > MAX_WHEEL_SPEED)) {
		int16_t factor_fp8;
		if (absWheelSpeed_fp4_0 > MAX_WHEEL_SPEED) 
			factor_fp8 = (MAX_WHEEL_SPEED*256) / absWheelSpeed_fp4_0;
		else
		if (absWheelSpeed_fp4_1 > MAX_WHEEL_SPEED) 
			factor_fp8 = (MAX_WHEEL_SPEED*256) / absWheelSpeed_fp4_1;
		else
			factor_fp8 = (MAX_WHEEL_SPEED*256) / absWheelSpeed_fp4_2;
		wheelSpeed_fp4_0 = i32_rsh(wheelSpeed_fp4_0 * factor_fp8,8);
		wheelSpeed_fp4_1 = i32_rsh(wheelSpeed_fp4_1 * factor_fp8,8);
		wheelSpeed_fp4_2 = i32_rsh(wheelSpeed_fp4_2 * factor_fp8,8);
		
		absWheelSpeed_fp4_0 = abs(wheelSpeed_fp4_0);
		absWheelSpeed_fp4_1 = abs(wheelSpeed_fp4_1);
		absWheelSpeed_fp4_2 = abs(wheelSpeed_fp4_2);
	}

	pWheel_speed_fp4[0] = wheelSpeed_fp4_0;
	pWheel_speed_fp4[1] = wheelSpeed_fp4_1;
	pWheel_speed_fp4[2] = wheelSpeed_fp4_2;
}

// compute actual speed in the coord-system of the IMU out of the encoder's data depending on the given tilt
void Kinematix::computeActualSpeed( int16_fp4_t pWheel_FP4[WHEELS], 
									int16_fp9_t pTiltX_FP9, int16_fp9_t pTiltY_FP9,
									int16_fp3_t& pVx_FP3, int16_fp3_t& pVy_FP3, int16_fp3_t& pOmega_FP3) {
	// this matrix depends on the tilt angle and corrects the kinematics 
	// due to the moved touch point of the ball
	kin.computeTiltRotationMatrix(pTiltX_FP9 >> 1,pTiltY_FP9 >> 1);

	// intermediate multiplications needed later on in fp8
	int16_t m00_01=  mul16s_rsh(trm_fp14[0][0],icm_fp08[0][1],16);	
	int16_t m02_20=  mul16s_rsh(trm_fp14[0][2],icm_fp08[2][0],16);
	int16_t m10_01=  mul16s_rsh(trm_fp14[1][0],icm_fp08[0][1],16);
	int16_t m11_10=  mul16s_rsh(trm_fp14[1][1],icm_fp08[1][0],16);
	int16_t m11_11=  mul16s_rsh(trm_fp14[1][1],icm_fp08[1][1],16);
	int16_t m12_20=  mul16s_rsh(trm_fp14[1][2],icm_fp08[2][0],16);
	int16_t m20_01=  mul16s_rsh(trm_fp14[2][0],icm_fp08[0][1],16);
	int16_t m21_10=  mul16s_rsh(trm_fp14[2][1],icm_fp08[1][0],16);
	int16_t m21_11=  mul16s_rsh(trm_fp14[2][1],icm_fp08[1][1],16);
	int16_t m22_20=  mul16s_rsh(trm_fp14[2][2],icm_fp08[2][0],16);
							
	// compute inverse kinematics matrix					
	int16_fp3_t lVx_FP3    = 
		i32_rsh(
			i32_rsh( mul16s(m11_10+m12_20, pWheel_FP4[0])  +
						mul16s( m10_01 +m11_11 + m12_20, pWheel_FP4[1]) + 
						mul16s(-m10_01+m11_11+m12_20, pWheel_FP4[2]),4) * FLOAT2FP32 (PI/180.0,14),17);					
	int16_fp3_t lVy_FP3    = 
		i32_rsh(
			i32_rsh( mul16s(-m02_20, pWheel_FP4[0]) 
				       +mul16s(-m00_01 -m02_20, pWheel_FP4[1])		   
					   +mul16s( m00_01-m02_20, pWheel_FP4[2]),4) * FLOAT2FP32 (PI/180.0,14),17);
	int16_fp3_t lOmega_FP3 = 
		i32_rsh(
			i32_rsh( mul16s(-m21_10-m22_20, pWheel_FP4[0]) 
					   +mul16s(-m20_01 -m21_11 -m22_20, pWheel_FP4[1])  
					   +mul16s( m20_01-m21_11-m22_20, pWheel_FP4[2]),4) * FLOAT2FP32 (1.0/BALL_RADIUS,16),19);						

	// reverse the direction where necessary
	pVx_FP3 = reverseDirection[0]?-lVx_FP3:lVx_FP3;
	pVy_FP3 = reverseDirection[1]?-lVy_FP3:lVy_FP3;
	pOmega_FP3 = reverseDirection[2]?-lOmega_FP3:lOmega_FP3;

}


void Kinematix::setupKinematics() {
	// create construction matrix and its inverse
	setupConstructionMatrix();
}

void Kinematix::menu() {

	printMenuHelp();	
	int16_t omega_fp3 = 0;
	int16_t actual_omega_fp3;
	int16_t speed_x_fp3 = 0;
	int16_t speed_y_fp3 = 0;
	int16_fp8_t pTiltX_FP8 = 0;
	int16_fp8_t pTiltY_FP8 = 0;

	int16_t frequency;
	int16_fp4_t omegaWheel_fp4[3];
	int16_t actualAngle_fp8[3],	actualWheelSpeedAvr_fp4[3];
	int16_t actual_speed_x_fp3,actual_speed_y_fp3;

	actualAngle_fp8[0] = 0;
	actualAngle_fp8[1] = 0;
	actualAngle_fp8[2] = 0;
	actualWheelSpeedAvr_fp4[0] = 0;
	actualWheelSpeedAvr_fp4[1] = 0;
	actualWheelSpeedAvr_fp4[2] = 0;

	omegaWheel_fp4[0] = FLOAT2FP16(10,4);
	omegaWheel_fp4[1] = FLOAT2FP16(20,4);
	omegaWheel_fp4[2] = FLOAT2FP16(30,4);
	
	int32_t actualEncoderPosition_fp8[WHEELS] = {0,0,0};;
	int32_t tobeEncoderPosition_fp8 = 0;

	TimePassedBy timer ;
	while (true)  {
		wdt_reset();
		uint16_t passed_ms;
		while (!timer.isDue_ms(7,passed_ms)); // wait until 7ms are over
		frequency = 1000L/passed_ms;
		
		mCtrl.getWheelAngleChange(actualAngle_fp8);

		actualEncoderPosition_fp8[0] += actualAngle_fp8[0];
		actualEncoderPosition_fp8[1] += actualAngle_fp8[1];
		actualEncoderPosition_fp8[2] += actualAngle_fp8[2];

		// apply inverse kinematics, i.e. compute speed out of wheel position given by the encoders	
		int16_t actualWheelSpeed_fp4[WHEELS];
		actualWheelSpeed_fp4[0] = mul16s_rsh(actualAngle_fp8[0],frequency,4);
		actualWheelSpeed_fp4[1] = mul16s_rsh(actualAngle_fp8[1],frequency,4);
		actualWheelSpeed_fp4[2] = mul16s_rsh(actualAngle_fp8[2],frequency,4);
		
		computeActualSpeed( actualWheelSpeed_fp4, pTiltX_FP8, pTiltY_FP8,
							actual_speed_x_fp3, actual_speed_y_fp3, actual_omega_fp3);
						
		// compute speed per wheel out of the speed/omega
		computeWheelSpeed(   speed_x_fp3, speed_y_fp3, omega_fp3, 
							 pTiltX_FP8, pTiltY_FP8,
 							 omegaWheel_fp4);		

		static uint32_t ms_duration = 0;
		if (ms_duration >= 1000L) {
			Serial.print(" w_out=(");Serial.print(FP2FLOAT(actualWheelSpeed_fp4[0],4),1,4);Serial.print(",");
			Serial.print(FP2FLOAT(actualWheelSpeed_fp4[1],4),1,4);Serial.print(",");
			Serial.print(FP2FLOAT(actualWheelSpeed_fp4[2],4),1,4);Serial.print(")");

			Serial.print(" a=(");Serial.print(FP2FLOAT(pTiltX_FP8,8),1,4);Serial.print(",");
			Serial.print(FP2FLOAT(pTiltY_FP8,8),1,4);Serial.print(") ");

			Serial.print(" v_out=(");Serial.print(FP2FLOAT(actual_speed_x_fp3,3),1,4);Serial.print(",");
			Serial.print(FP2FLOAT(actual_speed_y_fp3,3),1,4);Serial.print(",");
			Serial.print(FP2FLOAT(actual_omega_fp3,3),1,4);Serial.print(")");

			Serial.print(" v_in=(");Serial.print(FP2FLOAT(speed_x_fp3,3),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(speed_y_fp3,3),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(omega_fp3,3),1,3);Serial.print(") ");
			Serial.print(" tilt=(");Serial.print(FP2FLOAT(pTiltX_FP8,8),1,2);
			Serial.print(",");Serial.print(FP2FLOAT(pTiltY_FP8,8),1,2);Serial.print(") ");

			Serial.print(" w1=(");Serial.print(FP2FLOAT(omegaWheel_fp4[0],4),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(omegaWheel_fp4[1] ,4),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(omegaWheel_fp4[2] ,4),1,4);Serial.print(")");

			Serial.print(" enc=(");Serial.print(actualAngle_fp8[0]);
			Serial.print(",");Serial.print(actualAngle_fp8[1]);
			Serial.print(",");Serial.print(actualAngle_fp8[2]);Serial.print(")");

			Serial.print(" °=(");Serial.print(FP2FLOAT(actualEncoderPosition_fp8[0],8),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(actualEncoderPosition_fp8[1],8),1,4);
			Serial.print(",");Serial.print(FP2FLOAT(actualEncoderPosition_fp8[2],8),1,4);Serial.print(")");
			
			Serial.print(" us=");Serial.print(passed_ms);
			Serial.print(" f=");Serial.print(frequency);
			
			Serial.println();		
			actualEncoderPosition_fp8[0] = 0;
			actualEncoderPosition_fp8[1] = 0;
			actualEncoderPosition_fp8[2] = 0;

			tobeEncoderPosition_fp8= 0;
			ms_duration = 0;
		}				

		ms_duration += passed_ms;
		
		// set new motor speed and gather the current encoder position 
		mCtrl.setWheelSpeed(omegaWheel_fp4, passed_ms*1000, frequency);
		if (Serial.available()) {
			static char inputChar;
			inputChar = Serial.read();
			switch (inputChar) {
				case 'h': 
					printMenuHelp();
					break;
				case 'q': speed_x_fp3 +=8;break;
				case 'w': speed_x_fp3 -=8;break;

				case 'a': speed_y_fp3 += 8;break;
				case 's': speed_y_fp3 -= 8;break;

				case 'y': omega_fp3 += 8;break;
				case 'x': omega_fp3 -= 8;break;

				case 'p': pTiltX_FP8 +=10;break;
				case 'ü': pTiltX_FP8 -=10;break;

				case 'ö': pTiltY_FP8 +=10;break;
				case 'ä': pTiltY_FP8 -=10;break;

				case '0': return;
			
				default:
					break;
			} // switch
		}					
	} // while true		 
}




void Kinematix::testKinematics() {
	
	setupConstructionMatrix();
	float cm00 = FP2FLOAT(cm_fp19[0][0],19);
	float cm01 = FP2FLOAT(cm_fp19[0][1],19);
	float cm02 = FP2FLOAT(cm_fp19[0][2],19);
	float cm10 = FP2FLOAT(cm_fp19[1][0],19);
	float cm11 = FP2FLOAT(cm_fp19[1][1],19);
	float cm12 = FP2FLOAT(cm_fp19[1][2],19);
	float cm20 = FP2FLOAT(cm_fp19[2][0],19);
	float cm21 = FP2FLOAT(cm_fp19[2][1],19);
	float cm22 = FP2FLOAT(cm_fp19[2][2],19);
	
	Serial.println(F("construction matrix"));
	Serial.print(cm00,4,3);
	Serial.print(cm01,4,3);
	Serial.print(cm02,4,3);Serial.println();
	Serial.print(cm10,4,3);
	Serial.print(cm11,4,3);
	Serial.print(cm12,4,3);Serial.println();
	Serial.print(cm20,4,3);
	Serial.print(cm21,4,3);
	Serial.print(cm22,4,3);Serial.println(" ");

		
	int16_fp3_t lVx_fp3,lVy_fp3,lOmega_fp3;
	lVx_fp3 = FP(0,3);
	lVy_fp3 = FP(0,3);

	lOmega_fp3=FP(35,3);
	Serial.print(F("Vx="));
	
	Serial.print(FP2FLOAT(lVx_fp3,3),2,4);
	Serial.print(F(" Vy="));
	Serial.print(FP2FLOAT(lVy_fp3,3),2,4);
	Serial.print(F(" Omega="));
	Serial.print(FP2FLOAT(lOmega_fp3,3),2,4);
	Serial.println();

	int16_fp8_t lTiltX_fp8,lTiltY_fp8;
	lTiltX_fp8 = FLOAT2FP16(0,8);
	lTiltY_fp8 = FLOAT2FP16(0,8);

	Serial.print(F("TiltX="));
	Serial.print(FP2FLOAT(lTiltX_fp8,8),3,2);
	Serial.print(F(" TiltY="));
	Serial.print(FP2FLOAT(lTiltY_fp8,8),3,2);
	Serial.println();
	
	int16_t pWheel_speed_fp4[WHEELS] = {0,0,0};
	 computeWheelSpeed( lVx_fp3, lVy_fp3, lOmega_fp3,
	 					lTiltX_fp8, lTiltY_fp8,
						pWheel_speed_fp4);
	float lWheel1 = FP2FLOAT(pWheel_speed_fp4[0],4);
	float lWheel2 = FP2FLOAT(pWheel_speed_fp4[1],4);
	float lWheel3 = FP2FLOAT(pWheel_speed_fp4[2],4);
	
	Serial.print(F("W1="));
	Serial.print(lWheel1,2,4);
	Serial.print(F(" W2="));
	Serial.print(lWheel2,2,4);
	Serial.print(F(" W3="));
	Serial.print(lWheel3,2,4);
	Serial.println();					
}	

void Kinematix::testInverseKinematics() {
	
	setupConstructionMatrix();
	float icm00 = FP2FLOAT(icm_fp08[0][0],8);
	float icm01 = FP2FLOAT(icm_fp08[0][1],8);
	float icm02 = FP2FLOAT(icm_fp08[0][2],8);
	float icm10 = FP2FLOAT(icm_fp08[1][0],8);
	float icm11 = FP2FLOAT(icm_fp08[1][1],8);
	float icm12 = FP2FLOAT(icm_fp08[1][2],8);
	float icm20 = FP2FLOAT(icm_fp08[2][0],8);
	float icm21 = FP2FLOAT(icm_fp08[2][1],8);
	float icm22 = FP2FLOAT(icm_fp08[2][2],8);

	Serial.println(F("inverse construction matrix"));
	Serial.print(icm00,4,3);
	Serial.print(icm01,4,3);
	Serial.print(icm02,4,3);Serial.println(" ");
	Serial.print(icm10,4,3);
	Serial.print(icm11,4,3);
	Serial.print(icm12,4,3);Serial.println(" ");
	Serial.print(icm20,4,3);
	Serial.print(icm21,4,3);
	Serial.print(icm22,4,3);Serial.println(" ");	

	// speed of wheels in °/s
	float lWheel1 = -758.9;
	float lWheel2 = 36.4;
	float lWheel3 = -133.7;
	
	Serial.print(F("W1="));
	Serial.print(lWheel1,2,4);
	Serial.print(F(" W2="));
	Serial.print(lWheel2,2,4);
	Serial.print(F(" W3="));
	Serial.print(lWheel3,2,4);
	Serial.println();
						
	int16_fp8_t lTiltX_FP8,lTiltY_FP8;
	lTiltX_FP8 = FLOAT2FP16(20,8);
	lTiltY_FP8 = FLOAT2FP16(-15,8);
	Serial.print(F("TiltX="));
	Serial.print(FP2FLOAT(lTiltX_FP8,8),3,2);
	Serial.print(F(" TiltY="));
	Serial.print(FP2FLOAT(lTiltY_FP8,8),3,2);Serial.println();
	
	// this matrix depends on the tilt angle and corrects the 
	computeTiltRotationMatrix(lTiltX_FP8,lTiltY_FP8);

	
	int16_t lVx_fp3,lVy_fp3,lOmega_fp3;
	
	int16_t wheel_fp4[WHEELS] = {0,0,0};
	wheel_fp4[0] = FLOAT2FP16(lWheel1,4);
	wheel_fp4[1] = FLOAT2FP16(lWheel2,4);
	wheel_fp4[2] = FLOAT2FP16(lWheel3,4);
	
	computeActualSpeed( wheel_fp4,
				lVx_fp3, lVy_fp3, lTiltX_FP8, lTiltY_FP8,lOmega_fp3);
				
				
	float lVx = FP2FLOAT(lVx_fp3,3);			
	float lVy = FP2FLOAT(lVy_fp3,3);			
	float lOmega = FP2FLOAT(lOmega_fp3,3);


	Serial.print(F("Vx="));
	Serial.print(lVx,2,4);
	Serial.print(F(" Vy="));
	Serial.print(lVy,2,4);
	Serial.print(F(" Omega="));
	Serial.print(lOmega,2,4);
	Serial.println();
}


void Kinematix::testPerformanceKinematics() {
	
	Serial.println(F("Kinematics performance"));
	unsigned long start =	milliseconds();
	wdt_enable(WDTO_4S);
	wdt_reset();	
	delay_ms(1000);
	unsigned long end =	milliseconds();
	Serial.print("End ms=");
	Serial.println(end-start);
	
	int i = 0;
	wdt_reset();	
	Serial.println(F("Start"));
	start =	milliseconds();
	for (i = 0;i<1000;i++) {
		int16_fp3_t lVx_fp3,lVy_fp3,lOmega_fp3;
		lVx_fp3 = FP(300,3);
		lVy_fp3 = FP(-100,3);
		lOmega_fp3=FP(35,3);

		int16_fp8_t lTiltX_fp8,lTiltY_fp8;
		lTiltX_fp8 = FLOAT2FP16(20,8);
		lTiltY_fp8 = FLOAT2FP16(-15,8);
		int16_t pWheel_speed_fp4[WHEELS] = {0,0,0};
		computeWheelSpeed( lVx_fp3, lVy_fp3, lOmega_fp3,
	 					lTiltX_fp8, lTiltY_fp8,
						pWheel_speed_fp4);
						
		int16_t omegaWheel_fp4[3];
		omegaWheel_fp4[0] = pWheel_speed_fp4[0];
		omegaWheel_fp4[1] = pWheel_speed_fp4[1];
		omegaWheel_fp4[2] = pWheel_speed_fp4[2];
		// setMotorSpeed(omegaWheel_fp4, encoderWheel_fp4,passedMS, passedSRez);	
			
		
		computeActualSpeed(pWheel_speed_fp4,0,0,
					lVx_fp3, lVy_fp3, lOmega_fp3);
	}
	end = milliseconds();
	Serial.println(F("Stop"));

	Serial.print((end-start),DEC);Serial.print("ms for ");Serial.print(i,DEC);Serial.print(" loops, ");Serial.print(float((end-start)) / float(i),2,3);Serial.println("ms");					
}

void Kinematix::testTRM() {
	float glb_error = 0;
	for (int j = 1;j<20;j=j+5) {
		float error  = 0;
		for (float i = 0.0;i<2*PI;i=i+0.1) {
			float x,y;
			x = sin(float(i))*j;
			y = cos(float(i)) *j;


			kin.computeTiltRotationMatrix(FLOAT2FP16(x,8),FLOAT2FP16(y,8));

			float sin_tilt_x = sin(DEG2RAD(x));
			float cos_tilt_x = cos(DEG2RAD(x));
			float sin_tilt_y = sin(DEG2RAD(y));
			float cos_tilt_y = cos(DEG2RAD(y));

			error += (abs(sin_tilt_x)==0)?0: abs ((FP2FLOAT((trm_fp14[0][0]),14) - sin_tilt_x) / sin_tilt_x);
			error += (sin_tilt_y==0)?0:abs ((FP2FLOAT((trm_fp14[0][2]),14) - sin_tilt_y) / sin_tilt_y);

			error += (sin_tilt_x*sin_tilt_y == 0)?0:abs ((FP2FLOAT((trm_fp14[1][0]),14) - sin_tilt_x*sin_tilt_y) /(sin_tilt_x*sin_tilt_y));
			error += (cos_tilt_x==0)?0:abs ((FP2FLOAT((trm_fp14[1][1]),14) - cos_tilt_x) / cos_tilt_x);
			error += (sin_tilt_x*cos_tilt_y==0)?0:abs ((FP2FLOAT((trm_fp14[1][2]),14) - (-sin_tilt_x*cos_tilt_y)) / (sin_tilt_x*cos_tilt_y));

			error += (cos_tilt_x*sin_tilt_y==0)?0:abs ((FP2FLOAT((trm_fp14[2][0]),14) + cos_tilt_x*sin_tilt_y) / (cos_tilt_x*sin_tilt_y));
			error += (sin_tilt_x==0)?0:abs ((FP2FLOAT((trm_fp14[2][1]),14) - sin_tilt_x) / sin_tilt_x);
			error += (cos_tilt_x*cos_tilt_y==0)?0:abs ((FP2FLOAT((trm_fp14[2][2]),14) - cos_tilt_x*cos_tilt_y) / (cos_tilt_x*cos_tilt_y));

			Serial.print(int(error),DEC);
		}	
		glb_error = error;
		error = 0;
	}	
}

void Kinematix::printMenuHelp() {
	Serial.println(F("Kinematics"));
	Serial.println(F("q/w - inc/dec speed in x"));
	Serial.println(F("a/s - inc/dec speed in y"));
	Serial.println(F("y/x - inc/dec angular speed in z"));
	Serial.println(F("p/ü - inc/dec tilt x"));
	Serial.println(F("ö/ä - inc/dec tilt y"));

	Serial.println(F("h   - help"));
	Serial.println(F("0   - exit"));
}
