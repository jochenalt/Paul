/*
 * Kinematics.h
 *
 * Created: 05.12.2012 01:12:11
 * Author: JochenAlt
 *
 * Class that provides the kinematics, i.e. it computes the wheelspeed out of the x,y,omega speed and vice versa
 * - it knows the construction of the robot and all the data like ball radius, encoder pulses, wheel radius
 * - setup computes the several rotation matrixes and prepares them so that they can be computet efficiently during 
 *   main loop
 * - quite efficient, due to the preparation of all matrixes and fixed point arithmetix. One loop 
 *   kinematix + inverse kinematics takes less than 1ms (24Mhz Atmega)
 *
 * The computations are explained in the attached excel kinematics.xls
 */ 


#ifndef KINEMATIX_H_
#define KINEMATIX_H_

#include "Arduino.h"
#include "FixedPoint.h"



class Kinematix {
	public:
		Kinematix () {
		}

		// setup construction matrix, i.e. that precomputed matrix defining the construction specifics
		void setupKinematics();
		
		// forward kinematix: speed x/y/omega -> wheel speed
		void computeWheelSpeed( int16_t pVx, int16_t pVy, int16_t pOmegaZ, 
						int16_fp8_t pTiltX_FP8, int16_fp8_t pTiltY_FP8,
						int16_fp4_t pWheel1_speed_fp4[WHEELS]);

		// inverse kinematix: wheel speed -> speed xy/omega
		void computeActualSpeed(int16_fp4_t pWheel_FP4[WHEELS],
						int16_fp8_t pTiltX_FP8, int16_fp8_t pTiltY_FP8,
						int16_t& pVx, int16_t& pVy, int16_t& pOmega);

		// menu that provides couple of test functions  on base of Serial.
		void menu();
		// print the menu help page to Serial
		void printMenuHelp();

		// test kinematics, to be used in debugger
		void testKinematics();
		// test inverse kinematics, to be used in debugger
		void testInverseKinematics();
		// check performance
		void testPerformanceKinematics();
		
		// TRM is tiltRotationMatrix, the rotation matrix that is able to compensate the position where
		// the ball touches the ground. This points moves if the robot tilts.
		// Function to test this in the debugger.
		void testTRM();

	private:
		// compute the tilt rotation matrix, used in kinematics and inverse kinematics
		void computeTiltRotationMatrix(int16_fp8_t pTiltX_FP8, int16_fp8_t pTiltY_FP8);
		// pre-compute kinematics so that during the loop just a couple of multiplications are required.
		void setupConstructionMatrix();
		
		matrix16_33_t cm_fp19;	    // construction matrix, computed once during startup
		matrix16_33_t icm_fp08;	    // inverse construction matrix, computed once during startup
		matrix16_33_t trm_fp14;     // tilt rotation matrix to correct speed/omega depending on the tilt angle, computed in each loop
		matrix16_33_t kmm_fp11;	    // kinematics matrix, computed in each loop

		// true, if the direction of one coordinate is supposed to be reversed
		static bool reverseDirection[3]; // = { speed x, speed y, omega }
};

extern Kinematix kin;


#endif /* KINEMATIX_H_  */