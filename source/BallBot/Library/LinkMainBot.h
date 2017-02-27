/*
 * MsgType.h
 *
 * Created: 18.02.2013 14:36:02
 * Author: JochenAlt
 * Datagram represent the message content,  in addition it
 *  o computes and checks a checksum
 */ 


#ifndef COMM_MAIN_AND_BOT_H_
#define COMM_MAIN_AND_BOT_H_

#include "Arduino.h"
#include "setup.h" // this include gives different definitions in bot board and main board
#include "FixedPoint.h"
#include "TimePassedBy.h"
#include "Paul.h"

class CommMainAndBot {
	public:
		void setup();

#ifdef MAIN_BOARD
		void sendSetSpeedRequest(bool pBalancingMode, int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, Datagram &pMsg);
		void parseSetSpeedResponse(Datagram &pMsg,int16_t &pTiltX, int16_t &pTiltY, int16_t &pPosX, int16_t &pPosY, int16_t &pAccelX, int16_t &pAccelY);
		void sendConfigureRequest(Datagram &pMsg, bool pSet, ControlConfigurationType &ctrlConfig);
		void parseConfigureResponse(Datagram &pMsg,ControlConfigurationType &pCtrlConfig);
		void sendCalibrateIMURequest(Datagram &pMsg);
		void parseCalibrateIMUResponse(Datagram &pMsg,int16_t &pOffsetX, int16_t &pOffsetY, int16_t &pOffsetZ);
#endif
#ifdef BOT_BOARD
		void sendCalibrateIMUResponse(int16_t pOffsetX, int16_t pOffsetY, int16_t pOffsetZ, Datagram &pMsg);
		void parseSetSpeedRequest(Datagram &pMsg, bool &pBalancingMode, int16_t &pSpeedX, int16_t &pSpeedY, int16_t &pOmega);
		void sendSetSpeedResponse(int16_t pTiltX, int16_t pTiltY, int16_t pPosX, int16_t pPosY, int16_t pAccelX, int16_t pAccelY, Datagram &pMsg);
		void sendConfigureResponse(Datagram &pMsg, ControlConfigurationType &pCtrlConfig);
		void parseConfigureRequest(Datagram &pMsg,bool &pSet, ControlConfigurationType &pCtrlConfig);
		void parseStatusRequest(Datagram &pMsg);
		void sendStatusResponse( Datagram &pMsg);
#endif
		bool receive(char pFirstChar, uint32_t pTimeoutUs, Datagram &pMsg);
	private:
		StreamComm comm;
		Stream* serial;
		Stream* error;
};

#endif /* COMM_MAIN_AND_BOT_H_ */