/*
 * maths.cpp
 *
 * Created: 05.12.2012 00:22:32
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "FixedPoint.h"
#include <avr/wdt.h> 

// sin values in fixed point in steps of 4 degrees, slightly above 90°
// last value is a bit faked to hit 90° exactly (presumably not really required, but nicer)
// error is below 0.1%
int16_fp14_t sin_FP14_base[] { 
	0,FLOAT2FP16(0.069756473744,14),FLOAT2FP16(0.139173100960,14),
	  FLOAT2FP16(0.207911690818,14),FLOAT2FP16(0.275637355817,14),FLOAT2FP16(0.342020143326,14),
	  FLOAT2FP16(0.406736643076,14),FLOAT2FP16(0.469471562786,14),FLOAT2FP16(0.529919264233,14),
	  FLOAT2FP16(0.587785252292,14),FLOAT2FP16(0.642787609687,14),FLOAT2FP16(0.694658370459,14),
	  FLOAT2FP16(0.743144825477,14),FLOAT2FP16(0.788010753607,14),FLOAT2FP16(0.829037572555,14),
	  FLOAT2FP16(0.866025403784,14),FLOAT2FP16(0.898794046299,14),FLOAT2FP16(0.927183854567,14),
	  FLOAT2FP16(0.951056516295,14),FLOAT2FP16(0.970295726276,14),FLOAT2FP16(0.984807753012,14), 
	  FLOAT2FP16(0.994521895368,14),FLOAT2FP16(0.999390827019,14),FLOAT2FP16(1.000610351563,14) // last value is faked to hit 90° exactly
};

// compute sinus with fixed point arithmetics by interpolation of a given value table 
// Works from 0°..90° only
int16_fp14_t sinFirstQuadrant_FP6(int16_fp6_t pAngleDeg_FP6) {
	uint8_t			a = pAngleDeg_FP6  >> 8;						// compute number of interpolation interval
	int16_fp14_t	l1_FP14 = sin_FP14_base[a++];					// store left interval value
	int16_fp14_t	l2_FP14 = sin_FP14_base[a];						// store right interval value
	int16_t			x_FP6= pAngleDeg_FP6 - (pAngleDeg_FP6 & 0xFF00);// position within that interval from 0..255, 
																	// so null everything besides 0..255 ( = 0xFF00)
	return l1_FP14 + mul16s_rsh(x_FP6 , l2_FP14-l1_FP14,8);			// interpolation
}

// returns sin of parameter in fixed point arithmentics	
// works from -360°..360°
int16_fp14_t sin_FP6(int16_fp6_t pAngleDeg_FP6) {
	int16_fp4_t lAngleDef_FP6	 = pAngleDeg_FP6;
	while (lAngleDef_FP6 > FP(360,6))
		lAngleDef_FP6 -= FP(360,6);
	while (lAngleDef_FP6 < 0)
		lAngleDef_FP6 += FP(360,6);

	if (lAngleDef_FP6 > FP(270,6))
		return -sinFirstQuadrant_FP6(FP(360,6)-lAngleDef_FP6);
	else if (lAngleDef_FP6 > FP(180,6))
		return -sinFirstQuadrant_FP6(lAngleDef_FP6-FP(180,6));
	else if (lAngleDef_FP6 > FP(90,6))
		return sinFirstQuadrant_FP6(FP(180,6)-lAngleDef_FP6);
	else
		return sinFirstQuadrant_FP6(lAngleDef_FP6);
}

int16_fp14_t cos_FP6(int16_fp6_t pAngleDeg_FP6) {
	return sin_FP6(pAngleDeg_FP6 + FP(90,6));
}

// version that works on base of fp8, which is maxiumum precision for 0..90°
// Works from 0°..90° only. A bit slower than the one above
int16_fp14_t sinFirstQuadrant_FP8(int16_fp8_t pAngleDeg_FP8) {
	uint8_t a = pAngleDeg_FP8 >> 10;						// compute number of interpolation interval
	int16_fp14_t l1_FP14 = sin_FP14_base[a++];				// compute left value
	int16_fp14_t l2_FP14 = sin_FP14_base[a];				// compute right value
	int16_t x_FP8 = pAngleDeg_FP8-(pAngleDeg_FP8 & 0xFC00); // position within that interval from 0..1023, 
															// so null everything besides 0..1023 ( = 0xFC00)
	return l1_FP14 + mul16s_rsh(x_FP8 , l2_FP14-l1_FP14,10); 
}

// version that works on base of fp9, which is maxiumum precision for 0..30°
// Works from 0°..30° only. A bit slower than the one above
int16_fp14_t sinFirstQuadrant_FP9(int16_fp9_t pAngleDeg_FP9) {
	uint8_t a = pAngleDeg_FP9 >> 11;						// compute number of interpolation interval
	int16_fp14_t l1_FP14 = sin_FP14_base[a++];				// compute left value
	int16_fp14_t l2_FP14 = sin_FP14_base[a];				// compute right value
	int16_t x_FP9 = pAngleDeg_FP9-(pAngleDeg_FP9 & 0xF800); // position within that interval from 0..2047, 
															// so null everything besides 0..2047 ( = 0xF800)
	return l1_FP14 + mul16s_rsh(x_FP9 , l2_FP14-l1_FP14,11); 
}
	
// returns sin of parameter in fixed point arithmentics
// works from -90°..+90°
int16_fp14_t sin_FP8(int16_fp8_t pAngleDeg_FP8) {
	ASSERT((pAngleDeg_FP8<=FP(120,8)) && (pAngleDeg_FP8>=-FP(120,8)),28434);

	int16_fp4_t lAngleDef_FP8	 = pAngleDeg_FP8;
	if (lAngleDef_FP8 >= FP(90,8))
		return sinFirstQuadrant_FP8(FP(90,8)-(lAngleDef_FP8-FP(90,8)));
	else if (lAngleDef_FP8 >= 0)
		return sinFirstQuadrant_FP8(lAngleDef_FP8);
	else if (lAngleDef_FP8 >= -FP(90,8))
		return -sinFirstQuadrant_FP8(-lAngleDef_FP8);
	else 
		return -sinFirstQuadrant_FP8(FP(90,8)+(lAngleDef_FP8 + FP(90,8)));
}

// returns sin of parameter in fixed point arithmentics
// works from -30°..+30°
int16_fp14_t sin_FP9(int16_fp9_t pAngleDeg_FP9) {
	ASSERT((pAngleDeg_FP9<=FP(30,8)) && (pAngleDeg_FP9>=-FP(30,8)),28434);

	if (pAngleDeg_FP9 >= 0)
		return sinFirstQuadrant_FP9(pAngleDeg_FP9);
	else
		return -sinFirstQuadrant_FP9(-pAngleDeg_FP9);
}

// returns cos in fp14,  works from -90°..90°
int16_fp14_t cos_FP8(int16_fp8_t pAngleDeg_FP8) {
	ASSERT((pAngleDeg_FP8<=FP(120,8)) && (pAngleDeg_FP8>=-FP(120,8)),28437);
	if (pAngleDeg_FP8 >= FP(90,8))
		return -sinFirstQuadrant_FP8(pAngleDeg_FP8-FP(90,8));
	else if (pAngleDeg_FP8 >= 0)
		return sinFirstQuadrant_FP8(FP(90,8)-pAngleDeg_FP8);
	else if (pAngleDeg_FP8 >= -FP(90,8))
		return sinFirstQuadrant_FP8(pAngleDeg_FP8+FP(90,8));
	else 
		return -sinFirstQuadrant_FP8(-pAngleDeg_FP8 -FP(90,8));
}

// returns a rough approximation of arctan in degree fp10 (error is 1% at 10°)
int16_fp10_t arctan2_fp10(int z, int xy) {    
	return  mul16s_rsh(i32_lsh(xy,10)/z, FLOAT2FP16(180.0/PI,6),6);
}



// inefficient but precise way to compute an inverse matrix on base of floats
// used during initialization only.
void computeInverseMatrix(matrix16_33_t& pM, uint8_t pMBase, matrix16_33_t& pI, uint8_t pIBase) {

	// compute inverse with determinant
	float det_denominator = 
					     (float(pM[0][0])*pM[1][1] * pM[2][2]) +
 			             (float(pM[0][1])*pM[1][2] * pM[2][0]) + 
			             (float(pM[0][2])*pM[1][0] * pM[2][1]) -
			             (float(pM[2][0])*pM[1][1] * pM[0][2]) -
			             (float(pM[2][1])*pM[1][2] * pM[0][0]) -
			             (float(pM[2][2])*pM[1][0] * pM[0][1]);

	// compute inverse matrix (as described in wikipedia)
	float detRezi = float(1L<<pMBase) * float(1L<<pIBase) / float(det_denominator);
	pI[0][0] = detRezi*((float(pM[1][1]) * pM[2][2] - float(pM[1][2]) * pM[2][1]));
	pI[0][1] = detRezi*((float(pM[0][2]) * pM[2][1] - float(pM[0][1]) * pM[2][2]));
	pI[0][2] = detRezi*((float(pM[0][1]) * pM[1][2] - float(pM[0][2]) * pM[1][1]));
	pI[1][0] = detRezi*((float(pM[1][2]) * pM[2][0] - float(pM[1][0]) * pM[2][2]));
	pI[1][1] = detRezi*((float(pM[0][0]) * pM[2][2] - float(pM[0][2]) * pM[2][0]));
	pI[1][2] = detRezi*((float(pM[0][2]) * pM[1][0] - float(pM[0][0]) * pM[1][2]));
	pI[1][2] = detRezi*((float(pM[0][2]) * pM[1][0] - float(pM[0][0]) * pM[1][2]));
	pI[2][0] = detRezi*((float(pM[1][0]) * pM[2][1] - float(pM[1][1]) * pM[2][0]));
	pI[2][1] = detRezi*((float(pM[0][1]) * pM[2][0] - float(pM[0][0]) * pM[2][1]));
	pI[2][2] = detRezi*((float(pM[0][0]) * pM[1][1] - float(pM[0][1]) * pM[1][0]));
}

/*
void testSin() {
	Serial.println(F("Test fixed point sin"));
	bool ok = true;
	for (int i = -30;i<30;i++) {
		wdt_reset();
		float fl=i;
		int16_fp8_t x_fp6 = FLOAT2FP16(fl,6);
		int16_fp8_t x_fp9 = FLOAT2FP16(fl,9);

		int16_fp6_t x_fp8 = 0;		
		float sin_fl = sin(fl/(180.0/PI));
		float cos_fl = cos(fl/(180.0/PI));

		int16_fp14_t s1_fp14 = sin_FP9 (x_fp9);
		// int16_fp14_t s1_fp14 = sin_FP8 (x_fp8);
		int16_fp14_t c1_fp14 = cos_FP6 (x_fp6);

		float e_sin1 = 0;
		float e_cos1 = 0;
		if (abs(sin_fl)>0.000001)
			e_sin1 = abs((FP2FLOAT(s1_fp14,14)-sin_fl)/(sin_fl));
		if (abs(cos_fl)>0.000001)
			e_cos1 = abs((FP2FLOAT(c1_fp14,14)-cos_fl)/(cos_fl));
		float e_sin2 = 0;
		float e_cos2 = 0;

		int16_fp14_t s2_fp14;
		int16_fp14_t c2_fp14;
	
		if (i>=-100 && i<=100) {
			x_fp8 = FLOAT2FP16(fl,8);
			s2_fp14 = sin_FP8 (x_fp8);
			c2_fp14 = cos_FP8 (x_fp8);
			if (abs(sin_fl)>0.000001)
				e_sin2 = abs((FP2FLOAT(s2_fp14,14)-sin_fl)/(sin_fl));
			if (abs(cos_fl)>0.000001)
				e_cos2 = abs((FP2FLOAT(c2_fp14,14)-cos_fl)/(cos_fl));
		}

		if (e_sin1>0.05 || e_cos1> 0.05 || e_sin2 > 0.01 || e_cos2 > 0.01)
		{
			
			Serial.print(F("error in sin at "));
			Serial.println(i,DEC);
			ok 	 = false;
		}	
	}
	if (ok) 
		Serial.println(F("OK"));
	else
		Serial.println(F("not OK"));
	
}
*/