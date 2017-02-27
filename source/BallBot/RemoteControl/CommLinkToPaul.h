/*
 * CommLinkToPaul.h
 *
 * Created: 28.03.2013 10:32:36
 *  Author: JochenAlt
 */ 


#ifndef COMMLINKTOPAUL_H_
#define COMMLINKTOPAUL_H_

#include "LinkRemoteMain.h"

class CommLinkToPaul {
	public:
		void setup(void (*pSpareTimeLoop)(void));
		void callCalibrateIMU(int16_t &offsetX, int16_t &offsetY, int16_t &offsetZ);
		void callSpeechRequest(VoiceType pVoiceType, PoemType pPoemNo,  SongType pSongType, char* pText);			
		void callSendOption(uint8_t pVolume, VoiceType pVoice, LanguageType pLanguage, uint16_t pSpeechSpeed);
		void callControlConfiguration(ControlConfigurationType &pCtrlConfig);
		void callSetSpeed(	int16_t speedX, int16_t speedY, int16_t omega, 
							float &pBotVoltage, boolean &pIsbalancing, 
							float &pTiltX, float &pTiltY, int16_t &pPosX, int16_t &pPosY, 
							boolean &pBeingMobbed, char pTextBuffer[], boolean &pCommLinkOn);
	private:
		void (*spareTimeLoop)(void);
		CommRemoteAndMain linkToBot;
		uint8_t missedCommunications;
};

#endif /* COMMLINKTOPAUL_H_ */