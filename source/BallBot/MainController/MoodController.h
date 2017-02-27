/*
 * MoodController.h
 *
 * Created: 20.02.2013 17:48:38
 * Author: JochenAlt
 */ 


#ifndef MOODCONTROLLER_H_
#define MOODCONTROLLER_H_

#include "Arduino.h"
#include "LEDController.h"
#include "TimePassedBy.h"

enum MoodType {StartupMood, LowBattMood, SadMood, MobbingMood, PoetrySlam,SingingMood };

class MoodController {
	public: 
		MoodController() {
			mood = StartupMood;
			poemLine = 0;
			poemLinePause	= 0;	
			tiltStrength = 0;
			songLine = 0;
			songLinePause = 0,
			tiltDirection = 0;
			mobbingAlarm = false;
			eyePositionPhase = 0;
			eyeMovementWidth = 0;
			lowVoltageTold = false;
			startupMessageGiven = false;
		}
		void setMood(MoodType pMoodtype);
		MoodType getMood() { return mood;};
		
		void loop();
		void printMenuHelp();
		void menu();
		void setTilt(int16_t pAngleX_fp9, int16_t pAngleY_fp9, int pPosX, int pPosY, int pAccelX, int pAccelY);
		int16_t getTiltAngle() { return tiltDirection;} 
		int16_t getTiltStrength() { return tiltStrength;} 
		bool isBeingMobbed();

	private:
		void panicEyeLoop();
		void eyePositionMoveLoop();
		void mobbingRecognition();

		MoodType mood;
		TimePassedBy eyePositionTimer;
		bool eyePositionDirection;
		uint8_t eyePositionPhase;
		TimePassedBy panicEyeBlinker; // panic eye blinking timer
		TimePassedBy voiceTimer;// timer when something is to be said
		TimePassedBy moodTimer;	// timer when this mood will end
		bool lowVoltageTold;	// true if low voltage has been told
		bool startupMessageGiven; // true of startup message has been given

		int16_t eyeBrightness;	// brightness of the eyes 0..255
		int16_t eyePosition;	// position of the eyes, 0..360°
		int16_t eyeMovementWidth; // width of the eye rolling
		
		uint16_t poemLine;		// line of the peom currently playing
		int16_t poemLinePause;	// pause until the next line plays
			
		int16_t songLine;		// number of the currently played song line
		int16_t songLinePause;	// pause until next song line is played
		int16_t songNumber;		// number of the song player. 0= song selection still to be done

		int16_t tiltDirection;	// direction the tilt goes to
		int16_t tiltStrength;	// strength of the tilt
		bool mobbingAlarm;		// true, if Paul has been Pushed
};

extern  MoodController moodCtrl;

#endif /* MOODCONTROLLER_H_ */