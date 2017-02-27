/*
 * SpeechController.h
 *
 * Created: 18.02.2013 14:38:53
 *  Author: JochenAlt
 */ 


#ifndef SPEECHCONTROLLER_H_
#define SPEECHCONTROLLER_H_

#include "Arduino.h"
#include "TimePassedBy.h"
#include "Paul.h"




class SpeechController {
	public:
		SpeechController () {
			currentVolume = 186; // EMIC2 default volume, rescaled to 0..255
			textStrPtr = NULL;
			peakAmplitude = 0;
			lastAmplitude = 0;
			speechBufferReadIdx = 0;
			speechBufferWriteIdx = 0;
			speechBufferLen = 0;
			writeCurrentSpeechToBuffer = false;
		}
		
		// initialize software uart to talk to EMIC-2
		void setup();

		// have your say
		void sing(SongType pSong);
		void poem(PoemType pPoem);

		void say(VoiceType pVoice, const __FlashStringHelper* pLine_P);
		void say(VoiceType pVoice, char pLine[]);
		void say(String pLine);
		void say(const __FlashStringHelper* pLine_P);
		void say(char pLine[]);

		void sayEpson(const __FlashStringHelper* pLine_P);
		void sayDectalk(const __FlashStringHelper* pLine_P);
		void sayVoltage(String before, String after);
		bool isTalking() ;
		void stopTalking();
		boolean bufferEmpty();


		// send something to EMIC-2, pass BufferThis= true, if it should be available in getSpeakBuffer()
		void send(const __FlashStringHelper* pLine_P, bool pBufferThis);
		void sayLikeClintEastwood(const __FlashStringHelper* pLine_P);
		void sayLikeRobot(const __FlashStringHelper* pLine_P);
		void sayLikeClintEastwood(char	* pLine);
		void sayLikeRobot(char* pLine);

		void cmd(String pLine);
		void selectVoice(VoiceType voice /* 0..8 */);
		void setVolume(uint8_t pVolume /* 0..255 */);
		uint8_t getVolume();
		
		// return current amplitude of speaker. Returns value between 0..255 independent of volume by self-adaption.
		uint8_t getCurrentAmplitude();
		
		void setSpeedRate(uint16_t pSpeed);
		void setLanguage(LanguageType pLang);
		void setDecTalkParser();
		void setEpsonParser();
		void loop();
		
		void printMenuHelp();
		void menu();
		// returns a character of everything which is spoken by Paul
		void writeToSpeechBuffer(char c);
		char readFromSpeechBuffer();
	private:
		uint8_t currentVolume;
		VoiceType currentVoice;				
		LanguageType currentLanguage;
		uint8_t		currentSpeed;				
		
		// amplitude measurement
		int16_t peakAmplitude;			// top boundary
		int16_t lastAmplitude;

		TimePassedBy amplitudeTimer;	// timer is limit number of measurements
	
		// mechanism to send text to EMIC2 blockwise (due to just 9600 baud)
		TimePassedBy sendTimer;			// limit the time used for sending to EMIC2 per loop
		char* PROGMEM textStrPtr;		// current text position to be said
		bool writeCurrentSpeechToBuffer;
		char speechBuffer[128];
		uint8_t	speechBufferReadIdx;
		uint8_t	speechBufferWriteIdx;
		uint8_t	speechBufferLen;
};		

extern SpeechController speechCtrl;

#endif /* SPEECHCONTROLLER_H_ */