/*
 * IMUController.cpp
 *
 * Created: 28.12.2012 13:28:08
 *  Author: JochenAlt
 */ 



#include "Arduino.h"
#include "Kalman.h"
#include "MPU6050.h"
#include <avr/eeprom.h>
#include <avr/wdt.h> 
#include "IMUController.h"
#include "../BallBotMemory.h"


int16_fp16_t Kalman::Q_angle_fp24;			// Process noise variance for the accelerometer
int16_fp16_t Kalman::Q_bias_fp16;			// Process noise variance for the gyro bias
int16_fp16_t Kalman::R_measure_fp20;	
IMUController imu;

// set all configuration data, that is the kalman parameters as well as an initial offset as null value
void IMUController::initData() {
	initFilterData();
	
	last_us = 0;
}


// switch off the power line of the IMU to force full reset. Afterwards the uC is reset to force a compete setup
void IMUController::fullReset() {
	Serial.println(F("IMU reset. "));
	// reset IMU cutting power of, reset completely
	digitalWrite(IMU_RESET_PIN,HIGH); 
			
	// restart completely
	resetMicroController();	
}

volatile uint16_t newImuDataAvailable = 0;

// MPU6000 INTERRUPT ON INT0
void MPU6050dataAvailableInterrupt()
{
  newImuDataAvailable = 1;
}

// initializes the MPU6050 via I2C, sets the samples frequency and the interrupt
bool IMUController::initMPU() {
	// join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();	
	
	// initialize MPU via I2C
	imu.mpu.initialize();

	 // verify connection
    bool connOk = imu.mpu.testConnection();
	
	// set internal low-pass filter to 184Hz
	//            |   ACCELEROMETER    |           GYROSCOPE
	// * DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
    // * ---------+-----------+--------+-----------+--------+-------------
    // * 0        | 260Hz     | 0ms    | 256Hz     | 0.98ms | 8kHz
    // * 1        | 184Hz     | 2.0ms  | 188Hz     | 1.9ms  | 1kHz
	mpu.setDLPFMode(1);
	delay_ms(1);
	
	// set sample rate of IMU
	// Frequency = 1Khz/(<rate>+1) = 166Hz  
	imu.mpu.setRate(1000/166+2);	// Fsample= 1Khz/(5+1) = 166Hz  
	delay_ms(1);

	// switch on interrupt on INT0, if new data is available
	imu.mpu.setIntEnabled(1);
	imu.mpu.setIntDataReadyEnabled(1);
	delay_ms(1);

	// clear register after any read
	imu.mpu.setInterruptLatchClear(1);
	
    // MPU_INT is connected to INT 0. Enable interrupt on INT0
    attachInterrupt(0,MPU6050dataAvailableInterrupt,RISING);
	
	return connOk;
}


// setup IMU and check connection.
// if IMU is not there, try again, if still failed, reset the complete uC.
void IMUController::setup() {
	if (!initMPU()) {
		Serial.print(F("failed, 2nd try, "));
		if (!initMPU()) {
			fullReset();
		}		
	}
	
	// init kalman filter
	initFilterData();	
}

// null out the covariance matrix of the Kalman filter
void IMUController::initFilterData() {
	imu.xKal.init();
	imu.yKal.init();
};

// returns the null value of the IMU 
void IMUController::getOffset(int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ) {
	pOffsetX = memory.persistentMem.imuControllerConfig.offsetRawX;
	pOffsetY = memory.persistentMem.imuControllerConfig.offsetRawY;
	pOffsetZ = memory.persistentMem.imuControllerConfig.offsetRawZ;
}

// define current orientation as future coord-system
void IMUController::calibrate() {
	int16_fp9_t tiltX_fp9, tiltY_fp9;
	int16_fp6_t angularSpeedX_fp7,angularSpeedY_fp7;

	getFiltered(tiltX_fp9,tiltY_fp9, angularSpeedX_fp7,angularSpeedY_fp7);
	
	// null out previous offset
	memory.persistentMem.imuControllerConfig.offsetRawX = 0;
	memory.persistentMem.imuControllerConfig.offsetRawY = 0;
	memory.persistentMem.imuControllerConfig.offsetRawZ = 0;
	
	// re-define offset values
	int32_t sumX = 0;
	int32_t sumY = 0;
	int32_t sumZ = 0;
	uint16_t count = 0;
	
	// start a loop for 2 seconds
	uint32_t start = milliseconds();
	do {
		wdt_reset();
		fetchRawData(); // wait until next data is available
		sumX += imu.accRawX;
		sumY += imu.accRawY;
		sumZ += (imu.accRawZ + (1<<14));
		count++;
	} while (milliseconds()-start < 2000);

	memory.persistentMem.imuControllerConfig.offsetRawX = sumX / count;
	memory.persistentMem.imuControllerConfig.offsetRawY = sumY / count;
	memory.persistentMem.imuControllerConfig.offsetRawZ = sumZ / count;

	initFilterData();
}


// fetches the data from IMU, triggered by an interrupt or by a waiting period of 10ms (maximum)
// returns passed time since last invocation in us
int32_t IMUController::fetchRawData() {
	// work without interrupt, simply wait for next full millisecond since acc rate is 1KHz
	uint32_t now = 0;
	uint32_t passed = 0;

	uint16_t i = 0;
	do {
		now = microseconds();
		passed = now - last_us;
		i++;
	} while ((i<2000) && (last_us != 0) && (passed<10000UL) && (newImuDataAvailable == 0));
	if (i == 2000) 
		Serial.println("fetchRawData:2000");

	imu.dT_us = passed;
	imu.last_us = now; 

	imu.dT_s_fp20 = mul16s_rsh(imu.dT_us , (1UL<<30)/1000000UL, 10);
	int16_t useless_gyroZ;		
	// acceleration has LSB of 8192 (->full range is 2g)
	// gyros have a 131 LSB/deg/s (-> full range is +/- 250 degrees/s)
	// (gyro x/y are swapped on purpose, since the x gyro is fused with the x accel)
	delayMicroseconds(200);	// required, believe me
	imu.mpu.getMotion6( &imu.accRawX, &imu.accRawY, &imu.accRawZ, 
					    &imu.gyroRawY, &imu.gyroRawX, &useless_gyroZ);

	newImuDataAvailable = 0;
	
	// calibrate the values
	imu.accRawX -= memory.persistentMem.imuControllerConfig.offsetRawX; 				
	imu.accRawY -= memory.persistentMem.imuControllerConfig.offsetRawY; 				
	imu.accRawZ -= memory.persistentMem.imuControllerConfig.offsetRawZ; 				
					
	// compute angles out of accelerations
	imu.angleX_fp10 = arctan2_fp10(-imu.accRawZ, imu.accRawX);
	imu.angleY_fp10 = arctan2_fp10(-imu.accRawZ, imu.accRawY);

	static int16_fp18_t gyroScale_fp22 = FLOAT2FP16(1.0/131.0,22); // =32017 convert to [deg/s]
	imu.gyroX_fp7 = -mul16s_rsh(gyroScale_fp22,imu.gyroRawX,15); 
	imu.gyroY_fp7 = +mul16s_rsh(gyroScale_fp22,imu.gyroRawY,15); 
	return imu.dT_us;	
}

// returns angle in x/y in degrees
void IMUController::getFiltered(int16_fp9_t &pAngleX_fp9,	int16_fp9_t& pAngleY_fp9,
								int16_fp7_t &pGyroX_fp7,	int16_fp7_t &pGyroY_fp7) {
	int16_fp9_t angleX_fp10,angleY_fp10;
	
	angleX_fp10 = xKal.getAngle_fp10(imu.angleX_fp10,-gyroX_fp7,dT_s_fp20);
	angleY_fp10 = yKal.getAngle_fp10(imu.angleY_fp10,-gyroY_fp7,dT_s_fp20);
	pGyroX_fp7 = xKal.getGyro_fp7();
	pGyroY_fp7 = yKal.getGyro_fp7();				

	pAngleX_fp9 = angleX_fp10 >> 1;
	pAngleY_fp9 = angleY_fp10 >> 1;
}

void IMUController::printMenuHelp() {
	Serial.println(F("IMU Controller - tuning "));
	Serial.println(F("q/w - inc/dec Kalman QAngle"));
	Serial.println(F("a/s - inc/dec Kalman QBias"));
	Serial.println(F("y/x - inc/dec Kalman RMeasure"));
	Serial.println(F("n   - calibrate offset with current values"));

	Serial.println(F("r   - reset IMU"));
	Serial.println(F("h   - help"));
	Serial.println(F("0   - exit"));
}

void IMUController::menuController() {
	printMenuHelp();
	newImuDataAvailable = 0;
	while (true)  {
		wdt_reset();
		
		uint32_t dT = fetchRawData(); // in [us]
		
		int16_fp9_t angleXKalman_fp9, angleYKalman_fp9;
		int16_fp6_t angularSpeedKalX_fp7, angularSpeedKalY_fp7;

		uint32_t start = microseconds();
		getFiltered(angleXKalman_fp9,angleYKalman_fp9,angularSpeedKalX_fp7,angularSpeedKalY_fp7);
		uint32_t end = microseconds();

		static uint32_t passedTimeus = 0;
		passedTimeus += dT;

		// display values every 200ms
		if (passedTimeus > 100L*1000L) {
			Serial.print("raw(");
			Serial.print(FP2FLOAT(imu.angleX_fp10,10),1,3);
			Serial.print(",");
			Serial.print(FP2FLOAT(imu.gyroX_fp7,7),1,3);
			Serial.print("/");
			Serial.print(FP2FLOAT(imu.angleY_fp10,10),1,3);
			Serial.print(",");
			Serial.print(FP2FLOAT(imu.gyroY_fp7,7),1,3);
			Serial.print(F(")"));
			Serial.print(F("Kal=("));
			Serial.print(FP2FLOAT(angleXKalman_fp9,9),1,3);
			Serial.print(",");
			Serial.print(FP2FLOAT(angularSpeedKalX_fp7,7),1,3);
			Serial.print("/");
			Serial.print(FP2FLOAT(angleYKalman_fp9,9),1,3);
			Serial.print(",");
			Serial.print(FP2FLOAT(angularSpeedKalY_fp7,7),1,3);

			Serial.print(F(") dT="));
			Serial.print(passedTimeus);
			Serial.print(F(" sample rate="));
			Serial.print(1000000L/passedTimeus);
			Serial.print(F("Hz filter time="));
			Serial.print(end-start);
			Serial.print(F("us"));

			Serial.println();
			passedTimeus = 0;
		}
		if (Serial.available()) {
			static char inputChar;
			inputChar = Serial.read();
			switch (inputChar) {
				case 'n': 
					Serial.println(F("nullify offset"));
					imu.calibrate();
					Serial.print(F("offset="));
					Serial.print(memory.persistentMem.imuControllerConfig.offsetRawX);
					Serial.print(F(","));
					Serial.print(memory.persistentMem.imuControllerConfig.offsetRawY);
					Serial.print(F(","));
					Serial.print(memory.persistentMem.imuControllerConfig.offsetRawZ);

					Serial.println(F(")"));

				case 'h': 
					printMenuHelp();
					break;
				case 'q': 
				case 'w': 
					if (inputChar == 'q')
						Kalman::Q_angle_fp24 += 8;
					else
						Kalman::Q_angle_fp24 -= 8;

					Serial.print(F("Qangle="));
					Serial.print(FP2FLOAT(Kalman::Q_angle_fp24,24),4,5);
					Serial.println();
					break;
				case 'a': 
				case 's': 
					if (inputChar == 'a')
						Kalman::Q_bias_fp16 += 16;
					else
						Kalman::Q_bias_fp16 -= 16;

					Serial.print(F("Q_bias="));
					Serial.print(FP2FLOAT(Kalman::Q_bias_fp16,16),4,5);
					Serial.println();
					break;
				case 'y': 
				case 'x': 
					if (inputChar == 'y')
						Kalman::R_measure_fp20 += 64;
					else
						Kalman::R_measure_fp20 -= 64;

					Serial.print(F("R_measure="));
					Serial.print(FP2FLOAT(Kalman::R_measure_fp20,20),4,5);
					Serial.println();
					break;
				case 'r':
						fullReset();
						break;
				case '0': 
					return;
			
				default:
					break;
			} // switch
		} // if (Serial.available()) 	

	} // while true		 
}

void IMUController::printCalibrationData() {	
	Serial.print(("Kalman parameters     : "));
	Serial.print(F("Q_angle="));Serial.print(FP2FLOAT(Kalman::Q_angle_fp24,24),4,0);
	Serial.print(F(" Q_bias="));Serial.print(FP2FLOAT(Kalman::Q_bias_fp16,16),4,0);
	Serial.print(F(" R_measure="));Serial.print(FP2FLOAT(Kalman::R_measure_fp20,20),4,0);Serial.println();

	Serial.print(F("IMU calibration      : (x,y,z)=("));
		Serial.print(memory.persistentMem.imuControllerConfig.offsetRawX,DEC);Serial.print(",");
		Serial.print(memory.persistentMem.imuControllerConfig.offsetRawY,DEC);Serial.print(",");
		Serial.print(memory.persistentMem.imuControllerConfig.offsetRawZ,DEC);
	Serial.println(")");
}	