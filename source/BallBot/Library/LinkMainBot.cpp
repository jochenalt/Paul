/*
 * utilities.cpp
 *
 * Created: 18.02.2013 14:35:48
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "setup.h"
#include "MsgType.h"
#include "LinkMainBot.h"

#ifdef MAIN_BOARD
		void CommMainAndBot::setup() {
			serial = &Serial1;
			Datagram::setErrorStream(&Serial);
			StreamComm::setErrorStream(&Serial);
			error = &Serial;
		}
#endif
#ifdef BOT_BOARD
		void CommMainAndBot::setup() {
			serial = &Serial;
			Datagram::setErrorStream(&Serial);
			StreamComm::setErrorStream(&Serial);
			error = &Serial;
		}
#endif


#ifdef MAIN_BOARD
		void CommMainAndBot::sendSetSpeedRequest(bool pBalancingMode, int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(SET_SPEED_REQ,0);
			pMsg.setMessageDataInt16(idx,pSpeedX);
			pMsg.setMessageDataInt16(idx,pSpeedY);
			pMsg.setMessageDataInt16(idx,pOmega);
			pMsg.setMessageDataInt8(idx,pBalancingMode);		
			comm.send(serial, pMsg);
		}

		void CommMainAndBot::parseSetSpeedResponse(Datagram &pMsg,int16_t &pTiltX, int16_t &pTiltY, int16_t &pPosX, int16_t &pPosY,int16_t &pAccelX, int16_t &pAccelY) {
			uint8_t idx = 0;
			pTiltX= pMsg.getMessageDataInt16(idx);
			pTiltY= pMsg.getMessageDataInt16(idx);
			pPosX= pMsg.getMessageDataInt16(idx);
			pPosY= pMsg.getMessageDataInt16(idx);
			pAccelX= pMsg.getMessageDataInt16(idx);
			pAccelY= pMsg.getMessageDataInt16(idx);

		}
		

		void CommMainAndBot::sendConfigureRequest(Datagram &pMsg, bool pSet, ControlConfigurationType &ctrlConfig) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(CONFIGURE_REQ,0);
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

		void CommMainAndBot::parseConfigureResponse(Datagram &pMsg,ControlConfigurationType &pCtrlConfig) {
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
		
		// to be implemented in bot board
		void CommMainAndBot::sendCalibrateIMURequest(Datagram &pMsg) {
			pMsg.setMessageHeader(CALIBRATE_IMU_REQ,0);
			comm.send(serial, pMsg);
		}

		void CommMainAndBot::parseCalibrateIMUResponse(Datagram &pMsg,int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ) {
			uint8_t idx = 0;
			pOffsetX= pMsg.getMessageDataInt16(idx);
			pOffsetY= pMsg.getMessageDataInt16(idx);
			pOffsetZ= pMsg.getMessageDataInt16(idx);
		}
#endif
#ifdef BOT_BOARD
		void CommMainAndBot::sendCalibrateIMUResponse(int16_t pOffsetX, int16_t pOffsetY, int16_t pOffsetZ, Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(CALIBRATE_IMU_RES,0);
			pMsg.setMessageDataInt16(idx,pOffsetX);
			pMsg.setMessageDataInt16(idx,pOffsetY);
			pMsg.setMessageDataInt16(idx,pOffsetZ);
			comm.send(serial, pMsg);			
		}

				
		void CommMainAndBot::parseSetSpeedRequest(Datagram &pMsg, bool &pBalancingMode, int16_t &pSpeedX, int16_t &pSpeedY, int16_t &pOmega) {
			uint8_t idx = 0;
			pSpeedX = pMsg.getMessageDataInt16(idx);
			pSpeedY = pMsg.getMessageDataInt16(idx);
			pOmega  = pMsg.getMessageDataInt16(idx);
			pBalancingMode = pMsg.getMessageDataInt8(idx);
		}
		
		void CommMainAndBot::sendSetSpeedResponse(int16_t pTiltX, int16_t pTiltY, int16_t pPosX, int16_t pPosY, int16_t pAccelX, int16_t pAccelY, Datagram &pMsg) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(SET_SPEED_RES,0);
			pMsg.setMessageDataInt16(idx, pTiltX);
			pMsg.setMessageDataInt16(idx, pTiltY);
			pMsg.setMessageDataInt16(idx, pPosX);
			pMsg.setMessageDataInt16(idx, pPosY);
			pMsg.setMessageDataInt16(idx, pAccelX);
			pMsg.setMessageDataInt16(idx, pAccelY);

			comm.send(serial, pMsg);			
		}
	
		void CommMainAndBot::sendConfigureResponse(Datagram &pMsg, ControlConfigurationType &pCtrlConfig) {
			uint8_t idx = 0;
			pMsg.setMessageHeader(CONFIGURE_RES,0);
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

		void CommMainAndBot::parseConfigureRequest(Datagram &pMsg,bool &pSet, ControlConfigurationType &pCtrlConfig) {
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
		void CommMainAndBot::parseStatusRequest(Datagram &pMsg) {
			uint8_t idx = 0;
		}
		
		void CommMainAndBot::sendStatusResponse( Datagram &pMsg) {
			uint8_t idx = 0;
		}

#endif
		bool CommMainAndBot::receive(char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg) {
			if (pMsg.isMessage(pFirstChar)) {
				return comm.receive(serial,pFirstChar, pTimeoutUs, pMsg);
			}				
			else {
				return false;
			}				
		}			
