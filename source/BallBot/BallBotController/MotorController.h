                                                /*
 * MotorController.h
 *
 * Created: 09.01.2013 18:44:47
 * Author: JochenAlt
 * 
 * Class hat adds a PI controller on top of the Motor class. Does not completely encapsulate Motor class, setup still to be done
 *  - allows to set the motor speed in terms of °/s
 *  - retrieves motor encoder position in terms of ° 
 *  -  treats three motors as a whole.
 *  - has an EEPROM interface to get/set the PI terms
 *  - provides an interactive interface to test and calibrate the MotorController via UART
 */ 


#ifndef MOTORCONTROLLER_H_
#define MOTORCONTROLLER_H_


class MotorPIController {
public:
	void initDefaultValues() {
		Kp_fp8 = FLOAT2FP16(0.10,8); // proportional factor of PI controller
		Ki_fp8 = FLOAT2FP16(0.7,8);  // integrative factor of PI controller
	}
		
	void print();
	
	uint16_t Kp_fp8;
	uint16_t Ki_fp8;
};

class MotorController {
	public:
		MotorController() {
			currEncoder[0] = 0;
			currEncoder[1] = 0;
			currEncoder[2] = 0;
			lastTargetEncoder_fp8 = {0,0,0}; // target encoder position of last loop
			integratedError_fp8 = {0,0,0};   // diff between encoder as-is position and to-be position

		}
		
		// setup Motorcontroller. Has to be called before everything else. 
		// (Does not setup Motor class, Motor::setup needs to be called upfront)
		static void setup();
		
		void getWheelAngleChange(int16_fp8_t pAngleChange_fp8[WHEELS]);
		// set the speed of each motor in [°/s], pass the time since last call in order to compute  speed correctly
		void setWheelSpeed( int16_fp4_t pOmegaWheel_fp4[WHEELS], int16_t pPassedus, int16_t pFrequency);
		// stop all motors
		void stop();
		// print PI term to Serial
		void printCalibrationData();	
		// call menu that allows interactive testing of MotorController
		void menu();
		// print help of menu to Serial
		void printMenuHelp();

	private:
		int16_t termReductionBySpeed(int16_t pSpeed);
			
		int16_t currEncoder[WHEELS];  // current position of all encoders
		MovingAverage movAvr[WHEELS]; // moving average of the encoder pulses
		
		int32_fp8_t lastTargetEncoder_fp8[WHEELS]; // target encoder position in last loop
		int32_fp8_t integratedError_fp8[WHEELS];   // diff between encoder as-is position and to-be position
};

extern MotorController mCtrl;

#endif /* MOTORCONTROLLER_H_ */