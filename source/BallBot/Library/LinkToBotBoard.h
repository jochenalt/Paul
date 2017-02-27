/*
 * CommLinkToPaul.h
 *
 * Created: 28.03.2013 10:32:36
 *  Author: JochenAlt
 */ 


#ifndef COMM_LINK_MAIN_AND_BOT_H_
#define COMM_LINK_MAIN_AND_BOT_H_

#include "Arduino.h"
#include "LinkMainBot.h"

class CommLinkMainAndBot {
	public:
		void setup();
		void callControlConfiguration();
		void callCalibrateIMU(int16_t &offsetX, int16_t &offsetY, int16_t &offsetZ);
		boolean callSetSpeed(boolean pBotBalanceMode, int16_t pSpeedX, int16_t pSpeedY, int16_t pOmega, 
							int16_t &pTiltX_fp9, int16_t& pTiltY_fp9,int16_t& pPosX, int16_t& pPosY,
							int16_t &pAccelX, int16_t& pAccelY,
							void (*pSpareTimeLoop)(void));
	private:
		CommMainAndBot linkToBalBoard; // communication link between main board and balance board
};

#endif /* COMM_LINK_MAIN_AND_BOT_H_ */