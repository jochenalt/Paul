/*
 * IMUController.h
 *
 * Created: 28.12.2012 14:00:06
 * Author: JochenAlt
 *
 * Class to control the MPU6050 on base of a MPU 6050 Drotek breakout board. 
 *   - control MPU 6050 via I2C
 *   - configures MPU 6050 to deliver an interrupt with a defined sample rate (IMU_SAMPLE_FREQ)
 *   - fetches the accel and gyro data, computes angle and gyro and applies a Kalman filter
 *   - calibrates the offset of the imu in order to get a proper null value
 *   - stores the offset in EEPROM
 *   -
 */ 


#ifndef IMUCONTROLLER_H_
#define IMUCONTROLLER_H_

#include "Arduino.h"
#include "../fixedpoint.h"
#include "Kalman.h"
#include "MPU6050.h"


class IMUControllerConfig {
	public:
		void initDefaultValues() {
			offsetRawX = 541;
			offsetRawY = -571;
			offsetRawZ = -397;
		}

	int16_t offsetRawX;
	int16_t offsetRawY;
	int16_t offsetRawZ;
};

class IMUController {
	public:

	IMUController() {
		initData();
	}
	
	// set all configuration data, that is the kalman parameters as well as an initial offset as null value
	void initData();
		
	// fetches the data from IMU, triggered by an interrupt or by a waiting period of 10ms (maximum)
	// returns passed time since last invocation in us
	int32_t fetchRawData();
	void getFiltered(int16_fp9_t &pAngleX_fp9, int16_fp9_t& pAngleY_fp9,int16_fp7_t &pGyroX_fp7, int16_fp7_t &pGyroY_fp7);

	// setup IMU and check connection.
	// if IMU is not there, try again, if still failed, reset the complete uC.
	void setup();
	
	// initialize and null out the Kalman covariance matrix
	static void initFilterData();
	
	// switch off the power line of the IMU to force full reset. Afterwards the uC is reset to force a compete setup
	void fullReset();
	
	void menuController();
	void printCalibrationData();
	// calibrate current orientation of IMU as future origin
	void calibrate();
	// returns the null value of the IMU 
	void getOffset(int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ);
	private:
		// initializes the MPU6050 via I2C, sets the samples frequency and the interrupt
		bool initMPU();		
		
		// print help page of IMU menu to Serial
		void printMenuHelp();
		
		// raw (unfiltered) IMU data (gyroZ is not required)
		int16_t accRawX;	
		int16_t accRawY;
		int16_t accRawZ;
		int16_t gyroRawX;
		int16_t gyroRawY;			
	
		int16_t angleX_fp10;		// filtered angle in [ ° ]
		int16_t angleY_fp10;		// filtered angle in [ ° ]

		int16_t gyroX_fp7;			// filtered angular velocity in [ rad/s ]
		int16_t gyroY_fp7;			// filtered angular velocity in [ rad/s ]	
		
		uint16_t dT_us;				// time since waitforImu finished in last loop [us]
		uint32_t last_us;				// time since waitforImu finished in last loop [us]

		int16_fp20_t dT_s_fp20;		// time since waitforImu finished in last loop [us]
		MPU6050 mpu;				// connection to MPU6050 via I2C
		Kalman xKal, yKal;			// one Kalman filter per direction
};

extern IMUController imu;

#endif /* IMUCONTROLLER_H_ */