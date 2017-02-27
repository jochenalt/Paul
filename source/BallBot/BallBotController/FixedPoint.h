/*
 * FixedPoint.h
 *
 * Created: 26.11.2012 17:58:07
 * Author: JochenAlt
 * 
 * fixed point arithmetics to gain some performance. Most computations (including trigonometrics)
 * are done on base of 16 bit integers with fixed point with the base 2 (to allow shifting).
 * All variables and types used have a postfix _fp<base>. I.e. a variable count_fp8 means that
 * count is a fixed point number that has to be divided by (1<<8) to get the real representation.
 * count_fp8 = 100 means the float 100/256 = 0,390. Maximum number is MAXINT/256 = 256, resolution is 1/256=0,00390
 */ 

#ifndef FIXEDPOINT_H_
#define FIXEDPOINT_H_

// return -1,0,+1 depending on the sign of the number
#define sign(xy) (((xy)>0)?1:((xy)<0)?-1:0)

// return a fixed point representation of a number <xy> to the base <base>
#define FP(xy,base) ((xy)<<(base))
// same in 32bit. circumvent the shifting bug in avr-gcc on negative 32bit integers 
#define FP32(xy,base) (((xy)>=0)?(xy)<<base:-((-xy)<<base))

// return the multiplier of a base
#define FP_BASE(base) (1<<base)
// convert a float <fl> to a 16bit fixed point integer to the <base>
#define FLOAT2FP16(fl,base) ((int16_t)((fl)*(1L<<(base))))
// convert a float <fl> to a 32bit fixed point integer to the <base>
#define FLOAT2FP32(fl,base) ((int32_t)((fl)*(1L<<(base))))

// convert a fixed point number to a float 
#define FP2FLOAT(xy,base) ( float(xy) /(1L<<base) )
// convert degrees to radian 
#define DEG2RAD(xy) ((xy)*(PI/180.0))
// and vice versa
#define RAD2DEG(xy) ((xy)*(180.0/PI))

// definitions of fixed point types
typedef int32_t int32_fp14_t;
typedef int32_t int32_fp11_t;
typedef int32_t int32_fp5_t;
typedef int32_t int32_fp8_t;

typedef int16_t int16_fp36_t;
typedef int16_t int16_fp35_t;
typedef int16_t int16_fp34_t;
typedef int16_t int16_fp33_t;
typedef int16_t int16_fp32_t;
typedef int16_t int16_fp31_t;
typedef int16_t int16_fp30_t;
typedef int16_t int16_fp29_t;
typedef int16_t int16_fp28_t;
typedef int16_t int16_fp27_t;
typedef int16_t int16_fp26_t;
typedef int16_t int16_fp25_t;
typedef int16_t int16_fp24_t;
typedef int16_t int16_fp23_t;
typedef int16_t int16_fp22_t;
typedef int16_t int16_fp21_t;
typedef int16_t int16_fp20_t;
typedef int16_t int16_fp19_t;
typedef int16_t int16_fp18_t;
typedef int16_t int16_fp17_t;
typedef int16_t int16_fp16_t;
typedef int16_t int16_fp15_t;
typedef int16_t int16_fp14_t;
typedef int16_t int16_fp13_t;
typedef int16_t int16_fp12_t;
typedef int16_t int16_fp11_t;
typedef int16_t int16_fp10_t;
typedef int16_t int16_fp9_t;
typedef int16_t int16_fp8_t;
typedef int16_t int16_fp7_t;
typedef int16_t int16_fp6_t;
typedef int16_t int16_fp5_t;
typedef int16_t int16_fp4_t;
typedef int16_t int16_fp3_t;
typedef int16_t int16_fp2_t;
typedef int16_t int16_fp1_t;

// same for 3x3 matrices
typedef int16_t matrix16_33_t[3][3];
typedef int32_t matrix32_33_t[3][3];

// convenience macro: set a complete line of a 3x3 matrix with a, b,c
#define MATRIX33_SET_LINE(m,i,a,b,c) m[(i)][0] = (a);m[(i)][1] = (b); m[(i)][2] =  (c)
// assertion that print error to Serial
#define ASSERT(cond,error_no) if (!(cond))  { Serial.print("#"); Serial.println(error_no,DEC); };
// check that fixed point number is not exceeding 30000 (close to 2^15, so these fixed points are close to potential overflow)
#define FPCHECK16(xy,msg) { if (abs(xy)>30000) { Serial.print(F(" FP:"));Serial.print(F(msg)); }}	
	
// assembler routine to multiply to signed 16bit number and returns a signed 32bit number	
inline int32_t mul16s(int16_t a, int16_t b)
{
  int32_t result;
  asm(
    "clr __tmp_reg__"   "\n\t"
    "muls %B1,%B2"   "\n\t"      
    "mov %D0,R1"   "\n\t"
    "mov %C0,R0"   "\n\t"
    "mul %A1,%A2"  "\n\t"    
    "mov %B0,R1"   "\n\t"
    "mov %A0,R0"   "\n\t"
    "mulsu %B1,%A2""\n\t"    
    "sbc %D0,__tmp_reg__"  "\n\t"
    "add %B0,R0"  "\n\t"
    "adc %C0,R1"  "\n\t"
    "adc %D0,__tmp_reg__"  "\n\t"
    "mulsu %B2,%A1" "\n\t"    
    "sbc %D0,__tmp_reg__"  "\n\t"
    "add %B0,R0"    "\n\t"
    "adc %C0,R1"  "\n\t"
    "adc %D0,__tmp_reg__"  "\n\t"
    "clr R1"   "\n\t"
    : "=&a" (result)
    :"a" (a), "a" (b));

  return result;
}

// assembler routine to multiply to unsignd 16bit numbers and returns an unsigned 32bit integer
inline uint32_t multi16(uint16_t zahl_a, uint16_t zahl_b)
{
	uint32_t result;
	asm(
		"mul %B1,%B2"			"\n\t"		//zahl_a_high x zahl_b_high
		"mov %C0,R0"			"\n\t"
		"mov %D0,R1"			"\n\t"
		"mul %A1,%A2"			"\n\t"		//zahl_a_low x zahl_b_low
		"mov %A0,R0"			"\n\t"
		"mov %B0,R1"			"\n\t"
		"mul %B1,%A2"			"\n\t"		//zahl_a_high x zahl_b_low
		"add %B0,R0"			"\n\t"
		"adc %C0,R1"			"\n\t"
		"clr __zero_reg__"		"\n\t"		//R1 für Übertrag löschen
		"adc %D0,__zero_reg__"	"\n\t"		//Übertrag hinzuaddieren
		"mul %A1,%B2"			"\n\t"		//zahl_a_low x zahl_b_high
		"add %B0,R0"			"\n\t"
		"adc %C0,R1"			"\n\t"
		"clr __zero_reg__"		"\n\t"		//R1 für Übertrag löschen
		"adc %D0,__zero_reg__"	"\n\t"		//Übertrag hinzuaddieren
		: "=&r" (result)
		: "r" (zahl_a), "r" (zahl_b));

	return result;
}


// does a left shift on 32 bit number. Fixes the bug in avr-gcc, that 
// ignores the sign for 32 bit numbers (which it doesnt for 16 bit)
static inline int32_t i32_lsh(int32_t x,uint8_t shift) {
	return (x>=0)?(x << shift):-((-x) << shift);
}

// does a right shift on 32 bit number. Fixes the bug in avr-gcc, that 
// ignores the sign for 32 bit numbers (which it doesnt for 16 bit)
static inline int32_t i32_rsh(int32_t x,uint8_t shift) {
	return (x>=0)?(x >> shift):-((-x) >> shift);
}

// multiplies two 16bit numbers and shifts the result to the right
inline int32_t mul16s_rsh(int16_t a, int16_t b,uint8_t shift) {
	int32_t result = mul16s(a,b);
	if (result < 0)
		return -((-result) >> shift);
	return result >> shift;
}
	
// multiplies two 16bit numbers and shifts the result to the left
inline int32_t mul16s_lsh(int16_t a, int16_t b,uint8_t shift) {
	int32_t result = mul16s(a,b);
	if (result < 0)
		return -((-result) << shift);
	return result << shift;
}

// some sin/cos functions in fixed point
int16_fp14_t sin_FP6(int16_fp6_t pAngleDeg_FP6);
int16_fp14_t cos_FP6(int16_fp6_t pAngleDeg_FP6);
int16_fp14_t sin_FP8(int16_fp8_t pAngleDeg_FP8);
int16_fp14_t cos_FP8(int16_fp8_t pAngleDeg_FP8);
int16_fp14_t sin_FP9(int16_fp9_t pAngleDeg_FP9);
int16_fp10_t arctan2_fp10(int z, int xy);

// compute the inverse of a 3x3 matrix
void computeInverseMatrix(matrix16_33_t& pM, uint8_t pMBase, matrix16_33_t& pI, uint8_t pBase);

#endif /* FIXEDPOINT_H_ */