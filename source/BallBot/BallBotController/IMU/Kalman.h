/*
 * Kalman.h
 *
 * Created: 28.12.2012 13:27:49
 * Author: JochenAlt
 * 
 * Kalman filter implementation as found in the net from 
 * Adapted Kalman filter from https://github.com/TKJElectronics/KalmanFilter/blob/master
 * Explanation in http://blog.tkjelectronics.dk/2012/09/a-practical-approach-to-kalman-filter-and-how-to-implement-it/
 * converted in fixed point arithmetics, so one computation takes 0.2 ms, which is pretty fast.
 * fixed point is done in a way that it gives maximum precision for angles <30°
 */ 


#ifndef KALMAN_H_
#define KALMAN_H_

#include "Arduino.h"
#include "../setup.h"
#include "../Fixedpoint.h"

class Kalman {

public:
	void init() {
		angle_fp10 = 0;
	    rate_fp7 = 0;
        bias_fp7 = 0;	// Reset bias
	    // reset error covariance matrix
		// Since we assume that the bias is 0 and we know the starting angle (use setAngle), the 
		// error covariance matrix is set like so  
		// see: http://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical 
        P00_fp24 = 0;	
        P01_fp24 = 0;
        P10_fp24 = 0;
        P11_fp22 = 0;
		
		Q_angle_fp24=FLOAT2FP16(0.001,24);
		Q_bias_fp16 = FLOAT2FP16(0.002,16); // 
		R_measure_fp20 = FLOAT2FP16(0.03,20);
	}		
    Kalman() {
		init();
    };
	
    // return the kalman filtered gyro value of the last computation. Compensates the gyro drift.
    int16_fp7_t getGyro_fp7() {
		return rate_fp7;
	}		

    // The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
    int16_fp10_t getAngle_fp10(int16_fp10_t newAngle_fp10, int16_fp6_t newRate_fp7, int16_fp20_t dt_fp20) {
        rate_fp7 = newRate_fp7 - bias_fp7;
        angle_fp10 += mul16s_rsh(dt_fp20,rate_fp7,17);
	
        // Update estimation error covariance - Project the error covariance ahead
		int16_fp24_t dt_mul_P11_fp24 = mul16s_rsh(dt_fp20,P11_fp22,18);
        P00_fp24 += mul16s_rsh(dt_fp20, dt_mul_P11_fp24 - P01_fp24- P10_fp24 + Q_angle_fp24,20);
        P01_fp24 -= dt_mul_P11_fp24;
        P10_fp24 -= dt_mul_P11_fp24;
        P11_fp22 += mul16s_rsh(Q_bias_fp16,dt_fp20,14);
        
        // Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
        // Calculate Kalman gain - Compute the Kalman gain
	    // Estimate error - 1x1 matrix
		int16_fp9_t S_fp9 =  (1L<<29) / ( (P00_fp24>>4) + R_measure_fp20);

		// Kalman gain - This is a 2x1 matrix			
		int16_fp18_t K0_fp18 =  mul16s_rsh(P00_fp24,S_fp9,15);
        int16_fp18_t K1_fp18 =  mul16s_rsh(P10_fp24,S_fp9,15);
				
		// check correct use of fixed point
		FPCHECK16(K0_fp18,"K0");
		FPCHECK16(K1_fp18,"K1");
        
        // Calculate angle and bias - Update estimate with measurement zk (newAngle)
		// Angle difference - 1x1 matrix
        int16_fp10_t y_fp10 = newAngle_fp10 - angle_fp10;

        angle_fp10	+= mul16s_rsh(K0_fp18, y_fp10,18);
        bias_fp7	+= mul16s_rsh(K1_fp18, y_fp10,21);
        
        // Calculate estimation error covariance - Update the error covariance
        P00_fp24 -= mul16s_rsh(K0_fp18,P00_fp24,18);
        P01_fp24 -= mul16s_rsh(K0_fp18,P01_fp24,18);
        P10_fp24 -= mul16s_rsh(K1_fp18,P00_fp24,18);
        P11_fp22 -= mul16s_rsh(K1_fp18,P01_fp24,18);

		// check that fixed point is used correctly		
		FPCHECK16(dt_mul_P11_fp24,"dTxP11");
		FPCHECK16(bias_fp7,"bias");

		FPCHECK16(P00_fp24,"P00");
		FPCHECK16(P01_fp24,"P01");
		FPCHECK16(P10_fp24,"P10");
		FPCHECK16(P11_fp22,"P11");

		/*+
		static int i = 0;
		if (i++ == 200) {
			i = 0;
			Serial.print(" P=(");
			Serial.print(FP2FLOAT(P00_fp24,24),8,1);Serial.print(",");
			Serial.print(FP2FLOAT(P01_fp24,24),8,1);Serial.print(",");
			Serial.print(FP2FLOAT(P10_fp24,24),8,1);Serial.print(",");
			Serial.print(FP2FLOAT(P11_fp22,22),8,1);Serial.print(")");

			Serial.print(" K=");
			Serial.print(FP2FLOAT(K0_fp18,18),8,1);Serial.print(",");
			Serial.print(FP2FLOAT(K1_fp18,18),8,1);Serial.print(")");
			Serial.print(" S=");
			Serial.print(FP2FLOAT(S_fp9,9),8,3);
			Serial.print(" bias=");
			Serial.print(FP2FLOAT(bias_fp7,7),8,3)^;
			Serial.println();  
		}
		*/
        return angle_fp10;
    };
public:
    
    /* Kalman filter variables */
    static int16_fp16_t Q_angle_fp24;			// Process noise variance for the accelerometer
    static int16_fp16_t Q_bias_fp16;			// Process noise variance for the gyro bias
    static int16_fp16_t R_measure_fp20;			// Measurement noise variance - this is actually the variance of the measurement noise
	    
    int16_fp10_t angle_fp10;					// The angle calculated by the Kalman filter - part of the 2x1 state matrix
    int16_fp7_t bias_fp7;						// The gyro bias calculated by the Kalman filter - part of the 2x1 state matrix
    int16_fp7_t rate_fp7;						// Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate
    
    int16_fp22_t P00_fp24;						// Error covariance matrix - This is a 2x2 matrix
	int16_fp22_t P01_fp24;				
	int16_fp22_t P10_fp24;
	int16_fp22_t P11_fp22;		
};


#endif /* KALMAN_H_ */