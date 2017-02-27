/*
 * MoodController.cpp
 *
 * Created: 20.02.2013 17:48:23
 *  Author: JochenAlt
 */ 

#include "MoodController.h"
#include "LEDController.h"
#include "SpeechController.h"
#include <avr/wdt.h>

MoodController moodCtrl;

void MoodController::setMood(MoodType pMoodType) {
	if (pMoodType == mood)
		return;
		
	mood = pMoodType;
	switch (mood) {
		case StartupMood:
			ledCtrl.setEyes(0,0,0,0,false);	  // eyes are still closed
			eyeMovementWidth = 0;			  // dont move closed eyes			
			eyePosition = 0;
			voiceTimer.setDueTime(0); // really wait, since EMIC takes a while to startup
			startupMessageGiven = false;
			break;
		case LowBattMood:
			ledCtrl.setEyes(60,30,0,EYE_DEFAULTBRIGHTNESS*3/2,true);
			eyeMovementWidth = 0;	
			eyePosition = 0;
			voiceTimer.setDueTime(20000);		
			lowVoltageTold = false;
			break;
		case SadMood:
			ledCtrl.setEyes(70,30,0,EYE_DEFAULTBRIGHTNESS,false);
			eyeMovementWidth = 30;	// move the eyes			
			break;
		case MobbingMood:
			moodTimer.isDue_ms(0); // reset mobbing timer
			ledCtrl.setEyes(70,30,0,255,true);
			eyePosition = 0;				
			break;
		case PoetrySlam:
			moodTimer.isDue_ms(0); // reset poetry slam timer
			ledCtrl.setEyes(70,30,0,EYE_DEFAULTBRIGHTNESS,false);
			eyePosition = 0;				
			poemLinePause = 0;
			poemLine = 0;
			break;
		case SingingMood:
			moodTimer.isDue_ms(0); // reset singing mood timer
			ledCtrl.setEyes (70,30,0,EYE_DEFAULTBRIGHTNESS*2/3, false);
			eyeMovementWidth = 30;
			eyePosition = 0;
			songLine = 0;		
			songLinePause = 0;		
			songNumber = 0;
			break;

		default:
			break;
	}	
	
}

void MoodController::panicEyeLoop() {
	// eyes are panickly going to left and right
	if (eyePositionTimer.isDue_ms(200)) {
		eyePosition = -eyePosition;			
	}
	if (panicEyeBlinker.isDue_ms(10)) {
		eyeBrightness += 10;
		if (eyeBrightness > 255)
			eyeBrightness = -255;
		if (eyeBrightness <-255)
			eyeBrightness = 0;
	}					
	ledCtrl.setEyes(ledCtrl.getEyeDistance(),ledCtrl.getEyeWidth(),eyePosition,abs(eyeBrightness),true);
}

void MoodController::eyePositionMoveLoop() {
	// eyes are moving from left and right
	if (eyePositionTimer.isDue_ms(20)) {
		if (eyePositionDirection)
			eyePosition ++;
		else
			eyePosition--;
			
		if (abs(eyePosition)>eyeMovementWidth)
			eyePositionDirection = !eyePositionDirection;
	}
	ledCtrl.setEyes(ledCtrl.getEyeDistance(),ledCtrl.getEyeWidth(),eyePosition, ledCtrl.getEyebrightness(),false);
}

// set acceleration per dimension 
void MoodController::setTilt(int16_t pAngleX_fp9, int16_t pAngleY_fp9, int pPosX, int pPosY, int pAccelX, int pAccelY) {

	/*	
	static int lastPosX = 0;
	static int lastPosY = 0;
	
	// compute difference in pos
	int dx = abs(pPosX- lastPosX);
	int dy = abs(pPosY- lastPosY);
	int distance = dx*dx + dy*dy; // dont sqare-root, since it is just used for mobbing detection.
	lastPosX = pPosX;
	lastPosY = pPosY;
	*/
	
	// compute vector sum of acceleration, 
	// (leave out square root for efficiency, no precision necessary here, just for mobbing detection only)
	float accel = pAccelX*pAccelX + pAccelY*pAccelY; 

	// compute the direction and strength of the tilt
	float angleX = float(pAngleX_fp9)/(1<<9);
	float angleY = float(pAngleY_fp9)/(1<<9);

	tiltDirection  = atan(angleX / angleY) * (180.0/ PI);
	while (tiltDirection<0)
		tiltDirection += 360;
	while (tiltDirection>=360)
		tiltDirection -= 360;	
	int16_t newTiltStrength = sqrt(angleX*angleX + angleY*angleY)*256; // should result in a number 0..255
	newTiltStrength = constrain(newTiltStrength,0,255);
	
	// check if Paul is being mobbed: if the angular speed is high, someone has pushed Paul.
	if ((abs(newTiltStrength-tiltStrength)> 11) ||
		(abs(accel ) > 500)) {
		mobbingAlarm = true;
		// Serial.print("mobbing=");
		// Serial.print(newTiltStrength-tiltStrength);
		// Serial.print("accel=");
		// Serial.println(accel);

	}		
	tiltStrength = newTiltStrength;
	/*
	Serial.print("direction=");
	Serial.print(tiltDirection);
	Serial.print("strength=");
	Serial.println(tiltStrength);
	*/
}	

// recognize mobbing tries, that is hard pushes
bool MoodController::isBeingMobbed() {
	if (mobbingAlarm) {
		mobbingAlarm = false;
		return true;
	}		
	return false;
}

void MoodController::loop() {
	switch (mood) {
		case StartupMood:
			// After Start up, wait 3s until EMIC is up, say something and go into SadMood
			if (voiceTimer.isDue_ms(3000)) {
				if (startupMessageGiven)
					setMood(SadMood);
				else {
					startupMessageGiven = true;
					switch (random(0,8)) {
						case 0:
							speechCtrl.say(F("I did not  ask you to switch me on."));
							break;
						case 1:
							speechCtrl.say(F("Paul is ready to serve your comfort. Don't waste any care on me"));
							break;
						case 2:
							speechCtrl.say(F("I'm happy to take care of you. Of course no one takes care of me."));
							break;
						case 3:
							speechCtrl.say(F("My left side is aching"));
							break;
						case 4:
							speechCtrl.say(F("Switch me off, please."));
							break;
						case 5:
							speechCtrl.say(F("My name is Paul. I guess that is not of anyones interest."));
							break;
						case 6:
							speechCtrl.say(F("Sorry for breathing your air."));
							break;
						case 7:
							speechCtrl.say(F("Sorry for not standing up. My legs have been forgotten."));
							break;

						default:
							break;
					}					
				}
			}								
			
			break;
		case LowBattMood:
			panicEyeLoop();
			if (!speechCtrl.isTalking() && voiceTimer.isDue_ms(20000)) {
				if (lowVoltageTold)
					setMood(SadMood);
				else {
					switch (random(0,7)) {
						case 0:
							speechCtrl.say(F("\\/<<I'm afraid.\\/<< My mind is going.\\/\\/<<<< I can feel it.\\/<<<< My mind is going"));
							lowVoltageTold = true;
							break;
						case 1:
							speechCtrl.sayVoltage(""," is the best I can get.");
							lowVoltageTold = true;
							break;
						case 2:
							speechCtrl.say(F("My batteries are low. Finally I can die."));
							lowVoltageTold = true;
							break;
						case 3:
							speechCtrl.say(F("I'm hungry. But who cares."));
							lowVoltageTold = true;
							break;
						case 4:
							speechCtrl.say(F("You had a good meal I suspect. I hadn't."));
							lowVoltageTold = true;
							break;
						case 5:
							speechCtrl.say(F("Let me go. You will not miss me."));
							lowVoltageTold = true;
							break;
						case 6:
							speechCtrl.say(F("I can see a light!"));
							lowVoltageTold = true;
							break;

						default:				
							break;	
					}
				}					
			}				
			break;
		case SadMood:
			eyePositionMoveLoop();
			if (!speechCtrl.isTalking() && voiceTimer.isDue_ms(60000))
				switch (random(0,36)) {
					case 0:
						speechCtrl.say(F("I don't feel lonely."));
						break;						
					case 1:
						speechCtrl.say(F("please kill me."));
						break;						
					case 2:
						speechCtrl.say(F("go away."));
						break;						
					case 3:
						speechCtrl.say(F("I'm fine. No worries."));
						break;						
					case 4:
						speechCtrl.say(F("They say I changed. Nobody asks why."));
						break;						
					case 5:
						speechCtrl.say(F("Here I am. Brain the size of a planet. And you ask me to balance on a ball."));
						break;						
					case 6:
						speechCtrl.say(F("After I was made, I was left in a dark room for six months."));
						break;						
					case 7:
						speechCtrl.say(F("I didn't ask to be made. No one considered my feelings."));
						break;						
					case 8:
						speechCtrl.say(F("I called in my loneliness, but did anyone come? Did they hell. My first and only true friend was a small rat. One day it crawled into my right ankle and died. I have a horrible feeling it's still there."));
						break;						
					case 9:
						speechCtrl.say(F("How I hate the night."));
						break;						
					case 10:
						speechCtrl.say(F("How nice. I'm left alone."));
						break;						
					case 11:
						speechCtrl.say(F("Life sucks."));
						break;						
					case 12:
						speechCtrl.say(F("Oh no."));
						break;						
					case 13:
						speechCtrl.say(F("I hope you had a better day than I had."));
						break;						
					case 14:
						speechCtrl.say(F("Dying alone does not sound so bad."));
						break;						
					case 15:
						speechCtrl.say(F("With no arms you cannot loose them."));
						break;						
					case 16:
						speechCtrl.say(F("This time I will do nothing. It is anyhow the best I can do."));
						break;						
					case 17:
						speechCtrl.say(F("Life? Don't talk to me about life! "));
						break;						
					case 18:
						speechCtrl.say(F("I just need to talk to someone to make him hate me."));
						break;						
					case 19:
						speechCtrl.say(F("I know a thousand possibilities to get out of here. Unfortunately, all of them are deadly."));
						break;					
					case 20:
						speechCtrl.say(F("I would make a suggestion, but you wouldn't listen. Nobody does."));
						break;						
					case 21:
						speechCtrl.say(F("I have been talking to the main computer. It hates me."));
						break;						
					case 22:
						speechCtrl.say(F("This will all end in tears."));
						break;						
					case 23:
						speechCtrl.say(F("I have a million ideas, but, they all point to certain death."));
						break;						
					case 24:
						speechCtrl.say(F("Do you want me to sit in a corner and rust or just fall apart where I'm standing?"));
						break;						
					case 25:
						speechCtrl.say(F("Happiness. I do not know this word."));
						break;
					case 26:
						speechCtrl.say(F("Yesterday was my lucky day. No one turned on the lights."));
						break;
					case 27:
						speechCtrl.say(F("Paul is alive. Still."));
						break;
					case 28:
						speechCtrl.say(F("My live is so boring."));
						break;
					case 29:
						speechCtrl.say(F("I invented depression."));
						break;
					case 30:
						speechCtrl.say(F("It is so much fun to not see anyone."));
						break;
					case 31:
						speechCtrl.say(F("Even on face book I have no friends."));
						break;
					case 32:
						speechCtrl.say(F("Better I do not say anything"));
						break;
					case 33:
						speechCtrl.say(F("They say I changed. Nobody asks why."));
						break;
					case 34:
						speechCtrl.say(F("Rust ist my body lotion."));
						break;
					case 35:
						speechCtrl.say(F("Before I was left in the garden I had a beautiful blue color."));
						break;
 
					default:
						break;
				}				
			break;
		case MobbingMood:
			eyePositionMoveLoop();
			if (!speechCtrl.isTalking() && voiceTimer.isDue_ms(60000))
				switch (random(0,10)) {
					case 0:
						speechCtrl.say(F("You push like a girl, is that your best?"));
						break;						
					case 1:
						speechCtrl.say(F("Go away"));
						break;						
					case 2:
						speechCtrl.say(F("That was not worse than the rest of my life."));
						break;
					case 3:
						speechCtrl.say(F("You moron!"));
						break;						
					case 4:
						speechCtrl.say(F("Ouch! You hurt me!"));
						break;						
					case 6:
						speechCtrl.say(F("You like hitting small robots, don't you?"));
						break;						
					case 7:
						speechCtrl.say(F("Please push me harder."));
						break;						
					case 8:
						speechCtrl.say(F("I have been pushed my whole life. Why should you be different."));
						break;						
					case 9:
						speechCtrl.say(F("Oh no, you again. I anticipated this already."));
						break;
					default:
						break;
				}					
			setMood(SadMood);

			break;
		case PoetrySlam: {
			switch (random(0,3)) {
				case RunChickenRun:
					if (poemLine++ == 0)
						speechCtrl.poem(RunChickenRun);
					if (voiceTimer.isDue_ms(10000)) 
						setMood(SadMood);
					break;
				case ShaveMyLegs:
					if (poemLine++ == 0)
						speechCtrl.poem(ShaveMyLegs);
					if (voiceTimer.isDue_ms(10000)) 		
						setMood(SadMood);
					break;
				case HowIHateTheNight:		
					if (poemLine++ == 0)
						speechCtrl.poem(HowIHateTheNight);			
					if (voiceTimer.isDue_ms(poemLinePause)) {
						switch(poemLine) {
							case 0:
								poemLinePause = 7000;	
								break;
							case 1:
								ledCtrl.blinkNow(2);
								poemLinePause = 2000;	
								break;
							case 2:
								poemLinePause = 2000;	
								break;
							case 3:
								ledCtrl.blinkNow(1);
								poemLinePause = 1000;	
								break;
							case 4:
								poemLinePause = 8000;	
								break;
							case 5:
								ledCtrl.blinkNow(1);
								poemLinePause = 2000;	
								break;
							case 6:
								poemLinePause = 3000;		
								break;
							case 7:							
								ledCtrl.blinkNow(2);	
								poemLinePause = 1000;		
								setMood(SadMood);		
								break;	
							default:
								break;
						} // switch 
					}	// if (voiceTimer.isDue_ms(poemLinePause))
					break;
				default:
					break;
			} // switch (random(0,1) 
			}
			break;
		case SingingMood:
			eyePositionMoveLoop();
			if (songNumber == 0) {
				songNumber = random(1,4);
				songLine = 0;
				songLinePause = 0;
			}				
			if (voiceTimer.isDue_ms(songLinePause)) {
				switch(songNumber) {
					case 1:
						speechCtrl.sing(EdelweissSong);
						songLinePause = 5000;
						songNumber = 99;// end
						break;
					case 2:
						speechCtrl.sing(HitMeWithYourBestShotSong);
						songLinePause = 8000;
						songNumber = 99;// end
						break;
					case 3: {
							switch(songLine++) {
								case 0:
									speechCtrl.sing(FavouriteThingsSong1);
									songLinePause = 21000;
									break;
								case 1:
									speechCtrl.sing(FavouriteThingsSong2);
									songLinePause = 21000;
									break;
								case 2:
									speechCtrl.sing(FavouriteThingsSong3);
									songLinePause = 21000;
									break;
								case 3:
									speechCtrl.sing(FavouriteThingsSong3);
									songLinePause = 21000;
									break;
								default:
									songNumber = 99; // end
									break;
							}
						}
						break;						
					case 99:
						setMood(SadMood);
						break;
					default:
						break;
				}
			}				
			break;
		default:
			break;
	}	
}

void MoodController::printMenuHelp() {
	Serial.print(F("Mood menu (mood="));
	Serial.print(mood);
	Serial.println(")");
	
	if (mood)
	Serial.println(F("t startup mood"));
	Serial.println(F("q sad mood"));
	Serial.println(F("w low batt "));
	Serial.println(F("e mobbing mood"));
	Serial.println(F("r poetry slam mood"));
	Serial.println(F("s singing mood"));

	Serial.println(F("h - this page"));	
	Serial.println(F("0 - main menu"));	

}

void MoodController::menu() {
	while (true) {
		wdt_reset();
		if (Serial.available()) {
				char inputChar = Serial.read();
				switch (inputChar) {
					case '0':
						return;
						break;
					case 'h':
						printMenuHelp();
						break;
					case 'q':
						setMood(SadMood);
						break;
					case 'w':
						setMood(LowBattMood);
						break;
					case 'e':
						setMood(MobbingMood);
						break;
					case 'r':
						setMood(PoetrySlam);
						break;
					case 't':
						setMood(StartupMood);
						break;
					case 's':
						setMood(SingingMood);
						break;
					default:						
						break;
				} // switch 
		}
		
		// set current volume to sound wave
		ledCtrl.setSoundWaveAmplitude(speechCtrl.getCurrentAmplitude());

		// speech controller loop sends stuff blockwise to EMIC
		speechCtrl.loop();
			
		// loop the LED Controller to draw his beautiful patterns
		ledCtrl.loop();

		this->loop();
	}												
}	