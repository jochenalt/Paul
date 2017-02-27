/*
 * LinkRemoteMain.h
 *
 * Created: 18.02.2013 14:36:02
 * Author: JochenAlt
 */ 

#ifndef LINKREMOTE_MAIN_H_
#define LINKREMOTE_MAIN_H_


#include "LinkRemoteMain.h"
#include "FixedPoint.h"
#include "MsgType.h"
#include "setup.h"

class CommRemoteAndMain {
	public:
		void setup();
#ifdef MAIN_BOARD
		void parseRCSpeechRequest(Datagram &pMsg, VoiceType &pVoiceType, PoemType &pPoemNo, SongType &pSongNo, uint8_t & pLen, char* pText);
		void sendRCSpeechResponse(Datagram &pMsg); 
		void sendRCOptionsResponse(Datagram &pMsg,uint8_t pVolume, VoiceType pVoiceNo, LanguageType pLanguage, uint16_t pSpeedRate);
		void parseRCOptionsRequest(Datagram &pMsg, uint8_t &pVolume, VoiceType &pVoiceNo, LanguageType& pLanguage, uint16_t &pSpeedRate);
		void sendRCConfigureResponse(Datagram &pMsg, ControlConfigurationType &pCtrlConfig);
		void parseRCConfigureRequest(Datagram &pMsg,bool &pSet, ControlConfigurationType &pCtrlConfig);
		void sendRCCalibrateIMUResponse(int16_t pOffsetX, int16_t pOffsetY, int16_t pOffsetZ, Datagram &pMsg); 
		void parseRCSetSpeedRequest(Datagram &pMsg, int16_t &pSpeedX, int16_t &pSpeedY, int16_t &pOmega);
		void sendRCSetSpeedResponse(int16_fp8_t pVoltage_fp8, bool pBalancingMode, int16_t pTiltX, int16_t pTiltY, int16_t pPosX, int16_t pPosY, boolean isMobbed, char pText[], Datagram &pMsg);
#endif
#ifdef REMOTE_BOARD
		void sendRCSpeechRequest(Datagram &pMsg, VoiceType pVoiceType, PoemType pPoemNo, SongType pSongNo, char* pText);
		void sendRCOptionsRequest(Datagram &pMsg, uint8_t pVolume,VoiceType pVoiceNo, LanguageType pLanguage, uint16_t pSpeedRate);
		void parseRCOptionsResponse(Datagram &pMsg, uint8_t &pVolume, VoiceType &pVoiceNo, LanguageType& pLanguage, uint16_t &pSpeed);
		void parseRCSpeechResponse(Datagram &pMsg);
		void sendRCConfigureRequest(Datagram &pMsg, bool pSet, ControlConfigurationType &ctrlConfig);
		void parseRCConfigureResponse(Datagram &pMsg,ControlConfigurationType &pCtrlConfig);
		void sendRCSetSpeedRequest(int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, Datagram &pMsg);
		void parseRCSetSpeedResponse(Datagram &pMsg,int16_t &pVoltage_fp8, boolean &pBalanceMode, int16_t &pTiltX, int16_t &pTiltY, int16_t &pPosX, int16_t &pPosY, boolean &pIsMobbed, char pSpokenText[]);
		void sendRCCalibrateIMURequest(Datagram &pMsg);
		void parseRCCalibrateIMUResponse(Datagram &pMsg,int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ);
#endif
		bool receive(char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg);
	private:
		StreamComm comm;
		Stream* serial;
		Stream* error;
};

#endif /* LINKREMOTE_MAIN_H_  */