// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//     2012-06-20 - improved FIFO overflow handling and simplified read process
//     2012-06-19 - completely rearranged DMP initialization code and simplification
//     2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//     2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//     2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                - add 3D math helper file to DMP6 example sketch
//                - add Euler output and Yaw/Pitch/Roll output formats
//     2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//     2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//     2012-05-30 - basic DMP initialization working

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "Arduino.h"
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include "Wire.h"

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;

#include "../setup.h"
#include "../fixedpoint.h"

/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
 * ========================================================================= */

/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:

   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */



// uncomment "OUTPUT_READABLE_QUATERNION" if you want to see the actual
// quaternion components in a [w, x, y, z] format (not best for parsing
// on a remote host such as Processing or something though)
//#define OUTPUT_READABLE_QUATERNION

// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles
// (in degrees) calculated from the quaternions coming from the FIFO.
// Note that Euler angles suffer from gimbal lock (for more info, see
// http://en.wikipedia.org/wiki/Gimbal_lock)
//#define OUTPUT_READABLE_EULER

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
//#define OUTPUT_READABLE_YAWPITCHROLL

// uncomment "OUTPUT_READABLE_REALACCEL" if you want to see acceleration
// components with gravity removed. This acceleration reference frame is
// not compensated for orientation, so +X is always +X according to the
// sensor, just without the effects of gravity. If you want acceleration
// compensated for orientation, us OUTPUT_READABLE_WORLDACCEL instead.
//#define OUTPUT_READABLE_REALACCEL

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
//#define OUTPUT_READABLE_WORLDACCEL

// uncomment "OUTPUT_TEAPOT" if you want output that matches the
// format used for the InvenSense teapot demo
//#define OUTPUT_TEAPOT


// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
char* sBuffer;			// buffer for sprintf, etc.
// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

bool sTrace = false;						// true if data is traced continuously
DataFormatType sFormat = NONE;		// data format to trace

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
	// Serial.print("I");
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setupIMU() {
	// Serial.println(F("Initializing MPU-6050"));
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize serial communication
    // (115200 chosen because it is required for Teapot Demo output, but it's
    // really up to you depending on your project)
    
    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
    // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.

    // initialize device
    // Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // verify connection
    // Serial.println(F("Testing device connections..."));
	if (!mpu.testConnection())
		Serial.println(F("MPU6050 connection failed"));

    // wait for ready
	// delay(100);
	
    // Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    // while (Serial.available() && Serial.read()); // empty buffer
    // while (!Serial.available());                 // wait for data
    // while (Serial.available() && Serial.read()); // empty buffer again
	
    // load and configure the DMP
    // Serial.println(F("Initializing DMP..."));
    Serial.print("dmp ");
	devStatus = mpu.dmpInitialize();
	

	// mpu.resetDMP();
	// mpu.resetSensors();
    
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        // Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        // Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
        // Serial.println(F("MPU-6050 IMU ready"));
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

	// default values with rate=4 DLPF=3 gives a sample per 9ms 
	// with higher rate and low lowpass configuration we get a sample per 7ms
	/*
	Serial.println("default Rate=");Serial.println(mpu.getRate());
	Serial.println("default DLPF==");Serial.println(mpu.getDLPFMode());

	mpu.setRate(3);
	mpu.setDLPFMode(4);

	Serial.println("set Rate=");Serial.println(mpu.getRate());
	Serial.println("set DLPF=");Serial.println(mpu.getDLPFMode());
	*/
}


float Q_angle  =  0.001; //0.001
float Q_gyro   =  0.003;  //0.003
float R_angle  =  0.3;  //0.03


void kalmanCalculate(float newAngleX, float newAngleY, float newRateX, float newRateY,int passedMS, float &pXKalman, float &pYKalman) {

	static float x_angle = 0;
	static float x_bias = 0;
	static float y_angle = 0;
	static float y_bias = 0;

	static float P_00x = 0, P_01x = 0, P_10x = 0, P_11x = 0;
	static float P_00y = 0, P_01y = 0, P_10y = 0, P_11y = 0;

	float S, K_0, K_1;
    float dt = float(passedMS)/1000.0;

    x_angle += dt * (newRateX - x_bias);
    P_00x +=  - dt * (P_10x + P_01x) + Q_angle * dt;
    P_01x +=  - dt * P_11x;
    P_10x +=  - dt * P_11x;
    P_11x +=  + Q_gyro * dt;

    float xy = newAngleX - x_angle;
    S = P_00x + R_angle;
    K_0 = P_00x / S;
    K_1 = P_10x / S;

    x_angle +=  K_0 * xy;
    x_bias  +=  K_1 * xy;
    P_00x -= K_0 * P_00x;
    P_01x -= K_0 * P_01x;
    P_10x -= K_1 * P_00x;
    P_11x -= K_1 * P_01x;

	pXKalman = x_angle;
	
	y_angle += dt * (newRateY - y_bias);
    P_00y +=  - dt * (P_10y + P_01y) + Q_angle * dt;
    P_01y +=  - dt * P_11y;
    P_10y +=  - dt * P_11y;
    P_11y +=  + Q_gyro * dt;

    float yy = newAngleY - y_angle;
    S = P_00y + R_angle;
    K_0 = P_00y / S;
    K_1 = P_10y / S;

    y_angle +=  K_0 * yy;
    y_bias  +=  K_1 * yy;
    P_00y -= K_0 * P_00y;
    P_01y -= K_0 * P_01y;
    P_10y -= K_1 * P_00y;
    P_11y -= K_1 * P_01y;

	pYKalman = y_angle;
  }
  
void getIMUValuesFilter(float pCompFactor, float &pXComp, float &pYComp, float &pXKaiman, float &pYKaiman, int16_t passedMS) {

	// try complimentary filter first 	
	int16_t ax_raw, ay_raw, az_raw;
	int16_t gx_delta_raw, gy_delta_raw, gz_delta_raw;	
	// try complimentary filter first 
	// acceleration has LSB of 8192 (->full range is 2g)
	// gyros have a 131 LSB/deg/s (-> full range is +/- 250 degrees/s)
	mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_delta_raw, &gy_delta_raw, &gz_delta_raw);
	float ax = ax_raw/(8192.0*2.0); // convert to 0..1g for 0..90°
	float ay = ay_raw/(8192.0*2.0); // convert to 0..1g for 0..90°
	float gx = gy_delta_raw*((PI/180.0)/131.0); // convert to [rad/s]
	float gy = gx_delta_raw*((PI/180.0)/131.0); // convert to [rad/s]
	
	static float angleX = 0;
	static float angleY = 0;
	angleX = pCompFactor * (angleX + gx*passedMS/1000.0) + (1-pCompFactor)*ax; 
	angleY = pCompFactor * (angleY + gy*passedMS/1000.0) + (1-pCompFactor)*ay; 
         
	pXComp = angleX * (180.0 / PI);  // very close to sin(x)
	pYComp = angleY * (180.0 / PI);  
	// try kaiman filter
	kalmanCalculate(ax, ay, -gx, -gy,passedMS, pXKaiman, pYKaiman );
	pXKaiman = pXKaiman * (180.0 / PI);  // very close to sin(x)
	pYKaiman = pYKaiman * (180.0 / PI);  

}

void getIMUValuesDMP(int16_fp8_t &pX_fp8, int16_fp8_t &pY_fp8) {
	// x and y values are pitch and roll in terms of MPU6050
	mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
	pX_fp8 = FLOAT2FP16(ypr[1] * (180/M_PI),8);
	pY_fp8 = FLOAT2FP16(ypr[2] * (180/M_PI),8);
	/*
	int16_t quat_fp14[4];
	int32_t gravity_fp28[4];
	int32_t ypr_fp14[3];
	mpu.dmpGetQuaternion(quat_fp14, fifoBuffer);
	// Serial.print("qw:");Serial.print(q.w,3,6);Serial.print("#");Serial.print(FP2FLOAT(quat_fp14[0],14),3,6);Serial.println();
	// Serial.print("qx:");Serial.print(q.x,3,6);Serial.print("#");Serial.print(FP2FLOAT(quat_fp14[1],14),3,6);Serial.println();
	// Serial.print("qy:");Serial.print(q.y,3,6);Serial.print("#");Serial.print(FP2FLOAT(quat_fp14[2],14),3,6);Serial.println();
	// Serial.print("qz:");Serial.print(q.z,3,6);Serial.print("#");Serial.print(FP2FLOAT(quat_fp14[3],14),3,6);Serial.println();

    mpu.dmpGetGravity_fp14(gravity_fp28, quat_fp14);
	// Serial.print("gx:");Serial.print(gravity.x,3,6);Serial.print("#");Serial.print(FP2FLOAT(gravity_fp28[0],28),3,6);Serial.println();
	// Serial.print("gy:");Serial.print(gravity.y,3,6);Serial.print("#");Serial.print(FP2FLOAT(gravity_fp28[1],28),3,6);Serial.println();
	// Serial.print("gz:");Serial.print(gravity.z,3,6);Serial.print("#");Serial.print(FP2FLOAT(gravity_fp28[2],28),3,6);Serial.println();

    mpu.dmpGetPitchRoll_fp14(ypr_fp14, gravity_fp28);
	// Serial.print("prx:");Serial.print(ypr[1],3,6);Serial.print("#");Serial.print(FP2FLOAT(ypr_fp14[1],14),3,6);Serial.println();
	// Serial.print("pry:");Serial.print(ypr[2],3,6);Serial.print("#");Serial.print(FP2FLOAT(ypr_fp14[2],14),3,6);Serial.println();

	static int16_t rad2deg_fp4 = int16_t(180*(1<<4)/M_PI);
	pX_fp8 = (ypr_fp14[1]*rad2deg_fp4)>>10;
	pX_fp8 = (ypr_fp14[2]*rad2deg_fp4)>>10;
	*/
}
	
void printData(DataFormatType pFormat) {
	float lYaw,lPitch,lRoll;

	switch(pFormat) {
		case QUATERNION:
		 // display quaternion values in easy matrix form: w x y z
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            Serial.print("quat\t");
            Serial.print(q.w);
            Serial.print("\t");
            Serial.print(q.x);
            Serial.print("\t");
            Serial.print(q.y);
            Serial.print("\t");
            Serial.println(q.z);
			break;
		case EULER:
		 // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);
			lYaw	= euler[0] * 180/M_PI;
			lPitch	= euler[1] * 180/M_PI;
			lRoll	= euler[2] * 180/M_PI;
			Serial.print(lYaw,1,4);
            Serial.print(" ");
            Serial.print(lPitch,1,4);
            Serial.print(" ");
            Serial.print(lRoll,1,4);
            Serial.println();
			break;
		case YAW_PITCH_ROLL:
		 // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
			lYaw	= ypr[0] * 180/M_PI;
			lPitch	= ypr[1] * 180/M_PI;
			lRoll	= ypr[2] * 180/M_PI;
			Serial.print(lYaw,1,4);
            Serial.print(" ");
            Serial.print(lPitch,1,4);
            Serial.print(" ");
            Serial.print(lRoll,1,4);
            Serial.println();

			break;
		case REAL_ACCELERATION:
			 // display real acceleration, adjusted to remove gravity
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            Serial.print("areal\t");
            Serial.print(aaReal.x);
            Serial.print("\t");
            Serial.print(aaReal.y);
            Serial.print("\t");
            Serial.println(aaReal.z);
			break;
		case WORLD_ACCELERATION:
			// display initial world-frame acceleration, adjusted to remove gravity
            // and rotated based on known orientation from quaternion
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
            Serial.print("aworld\t");
            Serial.print(aaWorld.x);
            Serial.print("\t");
            Serial.print(aaWorld.y);
            Serial.print("\t");
            Serial.println(aaWorld.z);
			break;
		default:
			;
	}		
}


void fetchCommand() {
	char lCmd = '0';
	if (Serial.available()) {
		lCmd = Serial.peek();
		bool found = true;
		switch (lCmd) {
			case 'h':
				Serial.println(F("MPU-6050 board"));
				Serial.println();
				Serial.println(F("y\tyaw/pitch/roll in degrees"));
				Serial.println(F("e\teuler angles "));
				Serial.println(F("t\tteapot data"));
				Serial.println(F("w\tworld acceleration"));
				Serial.println(F("a\treal acceleration"));
				Serial.println(F("q\tquaternion"));

				Serial.println(F("c\tcontinuous trace"));
				Serial.println(F("C\tcontinuous trace off"));
				break;
			case 'c':
				sTrace = true;
				break;
			case 'C':
				sTrace = false;
				sFormat = NONE;
				break;
			case 'y':
				sFormat = YAW_PITCH_ROLL;
				break;
			case 'e':
				sFormat = EULER;
				break;
			case 'q':
				sFormat = QUATERNION;
				break;
			case 'a':
				sFormat = REAL_ACCELERATION;
				break;
			case 'W':
				sFormat = WORLD_ACCELERATION;
				break;
			default:
				found = false;
				break;
		}
		if (found)
			Serial.read();
	}		
}

void dispatchIMUCommand() {
	fetchCommand();
	if (sTrace) {
		if (sFormat != NONE)
			printData(sFormat);
	} else {
		if (sFormat != NONE) {
			printData(sFormat);
			sFormat = NONE;		
		}		
	}
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

bool imuReady() {
	return dmpReady;
}
bool waitForIMU() {
    // if programming failed, don't try to do anything
	// this should never reached, just in case reset watch dog to restart
    if (!dmpReady) {
		Serial.print(F("IMU not ready."));

		resetMicroController();
	}	
	
		// wait for MPU interrupt or extra packet(s) available
		while (!mpuInterrupt && fifoCount < packetSize) {
			// other program behavior stuff here
			// .
			// .
			// .
			// if you are really paranoid you can frequently test in between other
			// stuff to see if mpuInterrupt is true, and if so, "break;" from the
			// while() loop to immediately process the MPU data
			// .
			// .
			// .
			// Serial.print(F("-"));
		}
		// Serial.print("#");Serial.print(count);
	
		// reset interrupt flag and get INT_STATUS byte
		mpuInterrupt = false;
		mpuIntStatus = mpu.getIntStatus();

		// get current FIFO count
		fifoCount = mpu.getFIFOCount();
			

		// check for overflow (this should never happen unless our code is too inefficient)
		if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
			// reset so we can continue cleanly
			mpu.resetFIFO();
			Serial.println(F("IMU:FIFO overflow!"));

		// otherwise, check for DMP data ready interrupt (this should happen frequently)
		} else if (mpuIntStatus & 0x02) {
			// wait for correct available data length, should be a VERY short wait
			while (fifoCount < packetSize) {
				fifoCount = mpu.getFIFOCount(); 
			}
						
			// read a packet from FIFO
			mpu.getFIFOBytes(fifoBuffer, packetSize);
       
			// track FIFO count here in case there is > 1 packet available
			// (this lets us immediately read more without waiting for an interrupt)
			fifoCount -= packetSize;
		}						
		return true;
}

