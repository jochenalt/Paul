/*
 * BotController.h
 *
 * Created: 08.01.2013 10:33:39
 * Author: JochenAlt
 *
 * Class to perform the balancing part, i.e. the main controller loop
 * - manages two panes of control plus the angular speed of the bot
 * - per pane, a control loop is executed that gets the current position and computes the error correction by use of 
 *   state feedback control
 * - 
 */ 


#ifndef BOTCONTROLLER_H_
#define BOTCONTROLLER_H_


class ControlPane {
	public:
		void init () {
			// sumError_fp0 = 0;		// integral part of PID controller, summing up the error
			absBasePos_fp8 = 0;		// absolute position of the base (origin = position when the bot has been switched on)
			absBodyPos_fp8 = 0;		// absolute position of the body (origin = position when the bot has been switched on)
			lastAbsBodyPos_fp8 = 0; // absolute body position of last loop
			lastBodyVelocity_fp3 = 0; 
			lastBaseVelocity_fp3 = 0;
			lastToBeSpeed_fp3 = 0;

			toBeAbsBasePos_fp8 = 0; // to-be position of the base
			bodyVelocity_fp3 = 0,	// absolute velocity the bot's body has
			speed_fp3 = 0;
			errorBasePosition_fp4 = 0;
			errorBodyPosition_fp4 = 0;
			errorBaseVelocity_fp3 = 0;
			errorBodyVelocity_fp3 = 0;
			accel_fp0  = 0;
			accel_fp0_filtered = 0;
		}

		int16_fp3_t bodyVelocity_fp3;	// absolute velocity of body
		int32_t toBeAbsBasePos_fp8;		// absolute to-be position of the bot 
		int32_t absBasePos_fp8;			// absolute as-is position of the bot
		int32_t absBodyPos_fp8;			// absolute as-is position of the bot's body
		int32_t lastAbsBodyPos_fp8;		// absolute as-is position of last loop
		int16_t lastBodyVelocity_fp3;	
		int16_t lastBaseVelocity_fp3;
		int16_t lastToBeSpeed_fp3;
		int16_t bodyAccel_fp0;
		int16_t baseAccel_fp0;
		int16_fp4_t errorBasePosition_fp4; 
		int16_fp4_t errorBodyPosition_fp4; 
		int16_fp3_t errorBaseVelocity_fp3; 
		int16_fp3_t errorBodyVelocity_fp3;
		int16_t     errorBodyAccel_fp0;
		int16_t     errorBaseAccel_fp0;
		
		int16_t speed_fp3;			// speed in x direction [mm/s]
		int16_t error_fp0;			// current error of control loop used to compute the acceleration
		int16_t accel_fp0;			// final acceleration out of the control loop
		int16_t accel_fp0_filtered;
		
		// compute new speed in the given pane, i.e. returns the error correction that keeps the bot balanced and on track
		int16_fp3_t getNewSpeed(	int16_t pFrequency, int16_fp19_t p_looptime_fp19, 
						int16_t pActualSpeed_fp3, int16_fp3_t pToBeSpeed_fp3, 
						int16_t pActualOmega_fp3, int16_fp3_t pToBeOmega_fp3, 

						int16_fp3_t pTilt_fp9, int16_fp7_t pAngularSpeed_fp7);
		void print();
};

// declarations	of own functions
class BotController {
	public:
		void init() {
			x.init();
			y.init();
			mobbingModeOn = false;
			memory.persistentMem.ctrlConfig.initDefaultValues();
		}			
		BotController() {
			init();
		}
		
		// reset calibration parameters to default
		void reset();
		// print calibration data to Serial
		void printCalibrationData();
		// print speed, angle, accelleration etc. of one loop
		void printLoopData();
		// print how the bot is supposed to behave (v, omega) to Serial
		void printToBeData();


		// if a cmd in the bot controller menu is available fron Serial, process is
		bool dispatchCommand(char pChar, bool &pQuit); 
		// print menu help to Serial
		void printMenuHelp();
		// set the speed of the bot 
		void setSpeed(int16_fp3_t pSpeedX_fp3, int16_fp3_t pSpeedY_fp3, int16_fp3_t pOmega_fp3);
		void getSpeed(int16_fp3_t &pSpeedX_fp3, int16_fp3_t &pSpeedY_fp3, int16_fp3_t &pOmega_fp3);

		// main loop of bot balancing 
		void loop(boolean pBalancingOn);
		// wait until IMU provides new angle and new gyro values		
		void waitForIMUData(int16_t &pPassed_us, int16_t &pFrequency, int16_fp8_t & pTiltX_FP8, int16_fp8_t &pTiltY_FP8, int16_fp7_t &pAngularSpeedX_fp7,int16_fp7_t &pAngularSpeedY_fp7);

		void getFilteredAngle(int16_t &pAccelX, int16_t &pAccelY);
		void getAbsolutePosition(int16_t &posX, int16_t &posY);
		void getFilteredAccel(int16_t &pAccelX, int16_t &pAccelY);
	
		// weights used in  state feedback control (angle, angular speed, position, velocity, acceleration)
		// ControlConfigurationType ctrlConfig;
		
		int16_t toBeSpeedX_fp3; // speed in x direction [mm/s]
		int16_t toBeSpeedY_fp3; // speed in y direction [mm/s]
		int16_t toBeOmega_fp3;  // omega [°/s]

		int16_t looptime_us;	// time that passed since last loop, used for computation between angle and angular velocity
		int16_t frequency_Hz;	// same, but as frequency [Hz]
		
		// data from IMU; angle and angular speed
		int16_fp7_t angularSpeedX_fp7;
		int16_fp7_t angularSpeedY_fp7; 
		int16_fp9_t tiltX_FP9;
		int16_fp9_t tiltY_FP9;  

		bool mobbingModeOn;		
		// feedback control is done in x-pane and y-pane seperately
		ControlPane x,y;
};

extern BotController botCtrl;


#endif /* BOTCONTROLLER_H_ */