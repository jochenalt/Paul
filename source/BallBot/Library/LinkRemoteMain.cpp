#include "Arduino.h"
#include "LinkRemoteMain.h"

#ifdef MAIN_BOARD
		void CommRemoteAndMain::setup() {
			serial = &Serial;
			Datagram::setErrorStream(&Serial);
			StreamComm::setErrorStream(&Serial);
			error = &Serial;
		}
#endif
#ifdef REMOTE_BOARD 
		void CommRemoteAndMain::setup() {
			serial = &Serial1;
			Datagram::setErrorStream(&Serial);
			StreamComm::setErrorStream(&Serial);
			error = &Serial;
		}
#endif
#ifdef MAIN_BOARD
		void CommRemoteAndMain::parseRCSpeechRequest(Datagram &pMsg, VoiceType &pVoiceType, PoemType &pPoemNo, SongType &pSongNo, uint8_t & pLen, char* pText) {
			uint8_t idx = 0;
			pVoiceType = (VoiceType)pMsg.getMessageDataInt8(idx);
			pPoemNo = (PoemType)pMsg.getMessageDataInt8(idx);
			pSongNo = (SongType)pMsg.getMessageDataInt8(idx);
			pLen  = pMsg.getMessageDataInt8(idx);
			if (pLen > 0) {
				comm.receiveVarString(serial,10000UL,pLen,pText);
			}				
		}
		
		void CommRemoteAndMain::sendRCSpeechResponse(Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_SPEECH_RES,0);
			comm.send(serial, pMsg);
		}

				
		void CommRemoteAndMain::sendRCOptionsResponse(Datagram &pMsg,uint8_t pVolume, VoiceType pVoiceNo, LanguageType pLanguage, uint16_t pSpeedRate) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_SET_OPTIONS_RES,0);
			pMsg.setMessageDataInt8(idx,(int8_t)pVolume);
			pMsg.setMessageDataInt8(idx,(int8_t)pVoiceNo);
			pMsg.setMessageDataInt8(idx,(int8_t)pLanguage);
			pMsg.setMessageDataInt8(idx,(int8_t)(pSpeedRate/2));

			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCOptionsRequest(Datagram &pMsg, uint8_t &pVolume, VoiceType &pVoiceNo, LanguageType& pLanguage, uint16_t &pSpeed) {
			uint8_t idx = 0;
			pVolume = pMsg.getMessageDataInt8(idx);
			pVoiceNo = (VoiceType)pMsg.getMessageDataInt8(idx);
			pLanguage = (LanguageType)pMsg.getMessageDataInt8(idx);
			pSpeed= pMsg.getMessageDataInt8(idx)*2;
		}

		void CommRemoteAndMain::sendRCConfigureResponse(Datagram &pMsg, ControlConfigurationType &pCtrlConfig) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_CONFIGURE_RES,0);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.angleWeight_fp8);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.angularSpeedWeight_fp8);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.velocityWeight_fp10);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.positionWeight_fp10);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.accelWeight_fp7);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.bodyVelocityWeight_fp10);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.bodyPositionWeight_fp10);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.bodyAccelWeight_fp7);
			pMsg.setMessageDataInt16(idx,pCtrlConfig.omegaWeight_fp7);
			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCConfigureRequest(Datagram &pMsg,bool &pSet, ControlConfigurationType &pCtrlConfig) {
			uint8_t idx = 0;
			pSet= pMsg.getMessageDataInt8(idx);
			pCtrlConfig.angleWeight_fp8= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.angularSpeedWeight_fp8= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.velocityWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.positionWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.accelWeight_fp7= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyVelocityWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyPositionWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyAccelWeight_fp7= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.omegaWeight_fp7= pMsg.getMessageDataInt16(idx);

		}

		void CommRemoteAndMain::sendRCCalibrateIMUResponse(int16_t pOffsetX, int16_t pOffsetY, int16_t pOffsetZ, Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_CALIBRATE_IMU_RES,0);
			pMsg.setMessageDataInt16(idx,pOffsetX);
			pMsg.setMessageDataInt16(idx,pOffsetY);
			pMsg.setMessageDataInt16(idx,pOffsetZ);
			comm.send(serial, pMsg);			
		}

		void CommRemoteAndMain::parseRCSetSpeedRequest(Datagram &pMsg, int16_t &pSpeedX, int16_t &pSpeedY, int16_t &pOmega) {
			uint8_t idx = 0;
			pSpeedX = pMsg.getMessageDataInt16(idx);
			pSpeedY = pMsg.getMessageDataInt16(idx);
			pOmega  = pMsg.getMessageDataInt16(idx);
		}
		
		void CommRemoteAndMain::sendRCSetSpeedResponse(int16_fp8_t pVoltage_fp8, bool pBalancingMode, int16_t pTiltX, int16_t pTiltY, int16_t pPosX, int16_t pPosY, boolean isMobbed, char pText[], Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_SET_SPEED_RES,0);
			pMsg.setMessageDataInt16(idx, pTiltX);
			pMsg.setMessageDataInt16(idx, pTiltY);
			pMsg.setMessageDataInt16(idx, pPosX);
			pMsg.setMessageDataInt16(idx, pPosY);
			pMsg.setMessageDataInt8(idx, pBalancingMode);
			pMsg.setMessageDataInt16(idx, pVoltage_fp8);
			pMsg.setMessageDataInt8(idx, isMobbed);

			// send length of spoken text
			int16_t len = 0;
			if (pText != NULL) {
				len = strlen(pText);
			}				
			pMsg.setMessageDataInt8(idx, len);

			// send the text without ending \0 			
			for (int i = 0;i<len;i++) {
				pMsg.setMessageDataInt8(idx, pText[i]);				
			}
			comm.send(serial, pMsg);			
		}

#endif
#ifdef REMOTE_BOARD
		void CommRemoteAndMain::sendRCSpeechRequest(Datagram &pMsg, VoiceType pVoiceType, PoemType pPoemNo, SongType pSongNo, char* pText) {
			uint8_t idx = 0;
			uint16_t len = 0;
			if (pText != NULL) {
				len = strlen(pText);
				if (len>=255)
					len = 255;
			}					
			
			pMsg.setMessageHeader(RC_SPEECH_REQ,0);
			pMsg.setMessageDataInt8(idx,(int)pVoiceType);
			pMsg.setMessageDataInt8(idx,(int)pPoemNo);
			pMsg.setMessageDataInt8(idx,(int)pSongNo);
			pMsg.setMessageDataInt8(idx,len);
			comm.send(serial, pMsg,len, pText);
		}

		void CommRemoteAndMain::sendRCOptionsRequest(Datagram &pMsg, uint8_t pVolume,VoiceType pVoiceNo, LanguageType pLanguage, uint16_t pSpeedRate) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_SET_OPTIONS_REQ,0);
			pMsg.setMessageDataInt8(idx,pVolume);
			pMsg.setMessageDataInt8(idx,(uint8_t)pVoiceNo);
			pMsg.setMessageDataInt8(idx,(uint8_t)pLanguage);
			pMsg.setMessageDataInt8(idx,pSpeedRate);
			pSpeedRate = pSpeedRate*2;
			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCOptionsResponse(Datagram &pMsg, uint8_t &pVolume, VoiceType &pVoiceNo, LanguageType& pLanguage, uint16_t &pSpeedRate) {
			uint8_t idx = 0;
			pVolume = (uint8_t)pMsg.getMessageDataInt8(idx);
			pVoiceNo = (VoiceType)pMsg.getMessageDataInt8(idx);
			pLanguage  = (LanguageType)pMsg.getMessageDataInt8(idx);
			pSpeedRate = ((uint8_t)pMsg.getMessageDataInt8(idx))*2;

		}

		void CommRemoteAndMain::parseRCSpeechResponse(Datagram &pMsg) {
			uint8_t idx = 0;
		}

		void CommRemoteAndMain::sendRCConfigureRequest(Datagram &pMsg, bool pSet, ControlConfigurationType &ctrlConfig) {
			uint8_t idx = 0;
			
			pMsg.setMessageHeader(RC_CONFIGURE_REQ,0);
			pMsg.setMessageDataInt8(idx,pSet);
			pMsg.setMessageDataInt16(idx,ctrlConfig.angleWeight_fp8);
			pMsg.setMessageDataInt16(idx,ctrlConfig.angularSpeedWeight_fp8);
			pMsg.setMessageDataInt16(idx,ctrlConfig.velocityWeight_fp10);
			pMsg.setMessageDataInt16(idx,ctrlConfig.positionWeight_fp10);
			pMsg.setMessageDataInt16(idx,ctrlConfig.accelWeight_fp7);
			pMsg.setMessageDataInt16(idx,ctrlConfig.bodyVelocityWeight_fp10);
			pMsg.setMessageDataInt16(idx,ctrlConfig.bodyPositionWeight_fp10);
			pMsg.setMessageDataInt16(idx,ctrlConfig.bodyAccelWeight_fp7);
			pMsg.setMessageDataInt16(idx,ctrlConfig.omegaWeight_fp7);
			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCConfigureResponse(Datagram &pMsg,ControlConfigurationType &pCtrlConfig) {
			uint8_t idx = 0;
			pCtrlConfig.angleWeight_fp8= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.angularSpeedWeight_fp8= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.velocityWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.positionWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.accelWeight_fp7= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyVelocityWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyPositionWeight_fp10= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.bodyAccelWeight_fp7= pMsg.getMessageDataInt16(idx);
			pCtrlConfig.omegaWeight_fp7= pMsg.getMessageDataInt16(idx);
		}

		void CommRemoteAndMain::sendRCSetSpeedRequest(int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(RC_SET_SPEED_REQ,0);
			pMsg.setMessageDataInt16(idx,pSpeedX);
			pMsg.setMessageDataInt16(idx,pSpeedY);
			pMsg.setMessageDataInt16(idx,pOmega);
			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCSetSpeedResponse(Datagram &pMsg,int16_t &pVoltage_fp8, boolean &pBalanceMode, int16_t &pTiltX, int16_t &pTiltY, int16_t &pPosX, int16_t &pPosY, boolean &pIsMobbed, char pSpokenText[]) {
			uint8_t idx = 0;
			pTiltX= pMsg.getMessageDataInt16(idx);
			pTiltY= pMsg.getMessageDataInt16(idx);
			pPosX= pMsg.getMessageDataInt16(idx);
			pPosY= pMsg.getMessageDataInt16(idx);
			pBalanceMode = pMsg.getMessageDataInt8(idx);
			pVoltage_fp8 = pMsg.getMessageDataInt16(idx);
			pIsMobbed = pMsg.getMessageDataInt8(idx);
			uint8_t len = pMsg.getMessageDataInt8(idx);
			// receive text without trailing \0
			uint8_t textIdx = 0;
			for (int i = 0;i<len;i++) {
				char c = pMsg.getMessageDataInt8(idx);
				pSpokenText[textIdx++] = c;
			} 				
			
			pSpokenText[textIdx++] = 0; // add trailing \0
		}		

		void CommRemoteAndMain::sendRCCalibrateIMURequest(Datagram &pMsg) {
			pMsg.setMessageHeader(RC_CALIBRATE_IMU_REQ,0);
			comm.send(serial, pMsg);
		}

		void CommRemoteAndMain::parseRCCalibrateIMUResponse(Datagram &pMsg,int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ) {
			uint8_t idx = 0;
			pOffsetX= pMsg.getMessageDataInt16(idx);
			pOffsetY= pMsg.getMessageDataInt16(idx);
			pOffsetZ= pMsg.getMessageDataInt16(idx);
		}

#endif
		bool CommRemoteAndMain::receive(char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg) {
			if (pMsg.isMessage(pFirstChar)) {
				return comm.receive(serial,pFirstChar, pTimeoutUs, pMsg);
			}				
			else {
				return false;
			}				
		}					

