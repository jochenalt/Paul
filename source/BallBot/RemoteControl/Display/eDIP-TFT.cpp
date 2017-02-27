/*
 * eDIP_TFT.cpp
 *
 * Created: 20.03.2013 13:32:01
 *  Author: JochenAlt
 */ 


#include "eDIP-TFT.h"
#include "setup.h"
#include "Arduino.h"
#include <SPI.h>
#include "eDIP-TFT-flash.h"
#include "TimePassedBy.h"
#undef DEBUG 
#define DC1 0x11
#define DC2 0x12

#define NAK 0x15
#define ACK 0x06

// default colors
#define EA_BLACK 1
#define EA_BLUE 2
#define EA_RED 3
#define EA_GREEN 4
#define EA_PURPLE 5
#define EA_CYAN 6
#define EA_YELLOW 7
#define EA_WHITE 8
#define EA_DARKGREY 9
#define EA_ORANGE 10
#define EA_LILA 11
#define EA_DARKPURPLE 12
#define EA_MINT 13
#define EA_GRASSGREEN 14
#define EA_LIGHTBLUE 15
#define EA_LIGHTGREY 16
 
 
DisplayEDIP lcd;

void DisplayEDIP::setup(uint8_t pPinSS, uint8_t pSBUFPIN) {
	ssPIN = pPinSS;
	sBUFPIN = pSBUFPIN;
	pinMode(ssPIN, OUTPUT);
	pinMode(pSBUFPIN,INPUT);
	SPI.setClockDivider(SPI_CLOCK_DIV64);	// gives speed of 20Mhz/128 = 156 kHz, 200kHz is max
	SPI.setBitOrder(LSBFIRST);				// default of display
	SPI.setDataMode(SPI_MODE3);				// display's default: CPOL = 1, CPHA = 1
	SPI.begin();
	useSmallProtocol = true;
	currentFont = FONT_DEFAULT;
}

 
void DisplayEDIP::sendData(char* data, char len) {

#ifdef DEBUG
    char i;
    for (i=0;i<len;i++) {
      Serial.print(byte(data[i]),HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif  

    sendSmallDC1(data,len);
}

 

bool DisplayEDIP::sendSmallDC1(char* data, char len) {
	uint8_t tries = 0;
	digitalWrite(ssPIN,LOW);
  
	uint8_t i, checkSum;
	char ok = 0;  

	// try three times
	while ((tries<=5) && (ok == 0)) {
		tries++;
		SPI.transfer(0x11);
		checkSum = 0x11;
		SPI.transfer(len);
		checkSum += len;
  

		for(i=0; i < len; i++) {
			SPI.transfer(data[i]);
			checkSum += data[i];
		}
	
		// transfer checksum
		SPI.transfer(checkSum);
	
		// wait 6us before check for ack/nack (acc. to display's datasheet )
		delayMicroseconds(15);			   // no activity on clock
		char byteRead= SPI.transfer(0xFF); // dummy byte
		ok = (byteRead == ACK);
		if (!ok)
			delay(10);
	} // while not ok (potential endless loop, if never ok!)
	if (!ok) {
		setErrorLamp(F("sendSmallDC1: NACK"));
	}			
	digitalWrite(ssPIN,HIGH); 
	return ok;
}
 

bool DisplayEDIP::sendSmallDC2(char* data, char len) {

	uint8_t i, checksum;
	char ok = 0;  
	uint8_t tries = 0;

	// try three times
	while ((tries<=5) && (ok == 0)) {
		tries++;
		SPI.transfer(0x12);
		checksum = 0x12;
		for(i=0; i < len; i++) {
			SPI.transfer(data[i]);
			checksum = checksum + data[i];
		}			

		// send checksum
		SPI.transfer(checksum);
		delayMicroseconds(20);

		char byteRead= SPI.transfer(0xFF); // dummy byte
		ok = (byteRead == ACK);
		if (!ok) 
			delay(2);
	} // while not ok (potential endless loop, if never ok!)
	if (!ok) {
		setErrorLamp(F("sendSmallDC2: NACK"));
	}			
	return ok;
}



void DisplayEDIP::smallProtoSelect(char address) {
  char command [] = {  0x03,'A','S',address };
  sendSmallDC2(command,4);
}  

void DisplayEDIP::smallProtoDeselect(char address) {
  char command [] = { 0x03,'A','D',address };
  sendSmallDC2(command,4);
}  

void DisplayEDIP::requestBuffer(uint16_t &pSendBufferBytesReady, uint16_t &pReceiveBufferBytesReady) {
	digitalWrite(ssPIN,LOW); 

	char command [] = {  0x01,'I' };
	sendSmallDC2(command,2);

	delayMicroseconds(15);
	char dc2 = SPI.transfer(0xFF); // DC2
	uint8_t checksum = (uint8_t)dc2;
	if (dc2 != 0x12) {
		Serial.print(F("requestBuffer: no dc1 "));
		Serial.println(int(dc2));			
	}
	delayMicroseconds(15);
	char c = SPI.transfer(0xFF); // 0x2
	checksum = checksum + (uint8_t)c;
	if (c != 0x02) {
		Serial.print(F("requestBuffer: c "));
		Serial.println(int(c));			
	}

	delayMicroseconds(15);
	uint8_t x = SPI.transfer(0xFF);
	checksum  = checksum + x;
	pSendBufferBytesReady = x;
	
	delayMicroseconds(15);
	x = SPI.transfer(0xFF);
	checksum  = checksum + x;
	pReceiveBufferBytesReady = x;

	delayMicroseconds(15);
	uint8_t bcc  = SPI.transfer(0xFF); 
	if (bcc != checksum) {
		Serial.print(F("checksum="));
		Serial.print(checksum);
		Serial.print(F(" but should be "));
		Serial.println(bcc);

		setErrorLamp(F("checksum error"));		
	}
	digitalWrite(ssPIN,HIGH); 
}  

// wait until receive buffer of display has more than 128 bytes left
bool DisplayEDIP::waitUntilReceiveBufferEmpty(void (*pSpareTimeLoop)(void)) {
	uint16_t sendBufferBytesReady;
	uint16_t receiveBufferBytesReady;
	boolean wait = true;
	do {
		requestBuffer(sendBufferBytesReady,receiveBufferBytesReady);
		wait = receiveBufferBytesReady < 192;
		if (wait) {
			// Serial.print(F("requestBuffer:"));
			// Serial.print(receiveBufferBytesReady);
			TimePassedBy timer;
			while (!timer.isDue_ms(20))
				if (pSpareTimeLoop != NULL)
					pSpareTimeLoop();
		}
	} while (wait);
	// Serial.println(F("ok:"));
}

bool DisplayEDIP::requestSendBuffer(char pSendBuffer[], uint8_t pBufferSize, uint8_t &pLen) {
	if (digitalRead(sBUFPIN) == LOW) {
		digitalWrite(ssPIN,LOW); 

		char command [] = {  0x01,'S' };
		sendSmallDC2(command,2);
		delayMicroseconds(15);

		uint8_t dc1 = SPI.transfer(0xFF); // DC1
		if (dc1 != 0x11) {
			Serial.print(F("requestSendBuffer: no dc1 "));
			Serial.println(int(dc1));			
		}

		delayMicroseconds(15);
		pLen = SPI.transfer(0xFF); // len
		delayMicroseconds(15);

		if (pLen > pBufferSize-1) {
			Serial.println(F("sendbuffer too small"));
			pLen = pBufferSize;
		}
		
		int i = 0;
		uint8_t checksum = dc1 + pLen;
		for (;i<pLen;i++) {
			char c = SPI.transfer(0x00); // data
			delayMicroseconds(15);
			checksum = checksum + c;
			if (i<pBufferSize)
				pSendBuffer[i] = c;
		}

		if (i<pBufferSize)
			pSendBuffer[i] = 0; // delimiting 0
		else
			pSendBuffer[pBufferSize] = 0; // delimiting 0
			
		uint8_t bcc  = SPI.transfer(0xFF); 
		if (bcc != checksum) {
			Serial.print(F("checksum="));
			Serial.print(checksum);
			Serial.print(F(" but should be "));
			Serial.println(bcc);

			setErrorLamp(F("checksum error"));		
		}
		digitalWrite(ssPIN,HIGH); 

		return true;
	}
	
	return false; // nothing in send buffer
}  


void DisplayEDIP::clear() {
  char command [] = {27,'D','L'};
  sendData(command,sizeof(command));
}


void DisplayEDIP::invert() {
  char command [] = { 27,'D','I' };
  sendData(command,3);
}

void DisplayEDIP::setDisplayColor(char fg, char bg) {

  char command [] = { 27,'F','D',fg,bg};
  sendData(command,5);
}

 
void DisplayEDIP::fillDisplayColor(char bg) {
  char command [] = { 27,'D','F',bg };
  sendData(command,4);
}


void DisplayEDIP::terminalOn(boolean on) {
  if (on) {
    char command [] = {27,'T','E'};
    sendData(command,3);
  }
  else {
    char command [] = {27,'T','A'};
    sendData(command,3);
  }
}

void DisplayEDIP::clearTerminal() {
    char command [] = {12};
    sendData(command,sizeof(command));	
}

void DisplayEDIP::defineTerminal(boolean pBig, uint16_t x1,uint16_t y1, uint16_t x2,uint16_t y2) {
    char command [] = {27,'T','W', pBig?2:1, 1+x1/8, pBig?1+y1/16:1+y1/8, (x2-x1)/8,pBig?(y2-y1)/16:(y2-y1)/8};
    sendData(command,sizeof(command));
}

void DisplayEDIP::setTerminalColor(char fg, char bg) {
    char command [] = {27,'F','T', fg, bg};
    sendData(command,sizeof(command));	
}

void DisplayEDIP::cursor(boolean on) {
  if (on) {
    char command [] = {27,'T','C',1};
    sendData(command,4);
  }
  else {
    char command [] = {27,'T','C',0};
    sendData(command,4);
  }
}
 
void DisplayEDIP::setCursor(char col, char row) {
  char command [] = {27,'T','P',col,row};
  sendData(command,5);
}

void DisplayEDIP::setTransparencyByBackground() {
  char command [] = { 27,'U', 'T', 3 };
  sendData(command,sizeof(command));
 }

void DisplayEDIP::setTransparencyByCorner() {
  char command [] = { 27,'U', 'T', 1 };
  sendData(command,sizeof(command));
 }

void DisplayEDIP::setNoTransparency() {
  char command [] = { 27,'U', 'T', 0 };
  sendData(command,sizeof(command));
 }


void DisplayEDIP::displayFlashPicture(int x, int y, uint8_t no) {
  char command [] = { 27,'U', 'I', lowByte(x),highByte(x), lowByte(y),highByte(y),no };
  sendData(command,sizeof(command));
 }


void DisplayEDIP::defineBargraph(char dir, char no, int x1, int y1, int x2, int y2, byte sv, byte ev, char type) {
  char command [] = { 27,'B',dir,no, lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    char(sv), char(ev),type
  };
  sendData(command,15);
}

 void DisplayEDIP::updateBargraph(char no, char val) {
  char command [] = { 27,'B','A',no,val };
  sendData(command,5);
}

void DisplayEDIP::setBargraphColor(char no, char fg, char bg, char fr) {
  char command [] = {  27,'F','B',no,fg,bg,fr };
  sendData(command,7);
}

void DisplayEDIP::linkBargraphLight(char no) {
  char command [] = { 27,'Y','B',no };
  sendData(command,4);
}  

void DisplayEDIP::makeBargraphTouch(char no) {
  char command [] = {    27,'A','B',no };
  sendData(command,4);
}  

 

void DisplayEDIP::deleteBargraph(char no,char n1) {
  char command [] = {  27,'B','D',no,n1 };
  sendData(command,5);
}


void DisplayEDIP::defineInstrument(char no, int x1, int y1, char image, char angle, char sv, char ev) {

  char command [] = {	27,'I','P',no,
						lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
						image,angle,sv,ev};
  sendData(command,12);
}

void DisplayEDIP::updateInstrument(char no, char val) {
  char command [] = { 27,'I','A',no,val };
  sendData(command,5);
}

 

void DisplayEDIP::redrawInstrument(char no) {
  char command [] = {  27,'I','N',no };
  sendData(command,4);
}

 void DisplayEDIP::deleteInstrument(char no, char n1, char n2) {
  char command [] = {  27,'B','D',no,n1,n2 };
  sendData(command,6);

}

void DisplayEDIP::setLineColor(char fg, char bg) {
  char command [] = { 27,'F','G',fg,bg };
  sendData(command,5);
}

void DisplayEDIP::setLineThick(char x, char y) {
  char command [] = { 27,'G','Z',x,y  };
  sendData(command,5);
}

void DisplayEDIP::setTextColor(char fg, char bg) {
  char command [] = { 27,'F','Z',fg,bg };
  sendData(command,5);
}

uint8_t DisplayEDIP::getMaxFontWidth(char font) {	
	switch (font) {
	  case FONT_TINY:
		return FONT_TINY_WIDTH(1);
	  case FONT_SMALL:
		return FONT_SMALL_WIDTH(1);
	  case FONT_DEFAULT:
		return FONT_DEFAULT_WIDTH(1);
	  case FONT_LARGE:
		return FONT_LARGE_WIDTH(1);
	  case FONT_GRANT:
		return FONT_GRANT_WIDTH(1);
	  case FONT_HUGE:
		return FONT_HUGE_WIDTH(1);
	  case FONT_GIANT:
		return FONT_GIANT_WIDTH(1);
	  default:
		return 10;	
	}  
}

uint8_t DisplayEDIP::getMinFontWidth(char font) {	
	switch (font) {
	  case FONT_TINY:
		return FONT_TINY_WIDTH(0);
	  case FONT_SMALL:
		return FONT_SMALL_WIDTH(0);
	  case FONT_DEFAULT:
		return FONT_DEFAULT_WIDTH(0);
	  case FONT_LARGE:
		return FONT_LARGE_WIDTH(0);
	  case FONT_GRANT:
		return FONT_GRANT_WIDTH(0);
	  case FONT_HUGE:
		return FONT_HUGE_WIDTH(0);
	  case FONT_GIANT:
		return FONT_GIANT_WIDTH(0);
	  default:
		return 10;	
	}  
}

uint8_t DisplayEDIP::getFontHeight(char font) {	
	switch (font) {
	  case FONT_TINY:
		return FONT_TINY_HEIGHT;
	  case FONT_SMALL:
		return FONT_SMALL_HEIGHT;
	  case FONT_DEFAULT:
		return FONT_DEFAULT_HEIGHT;
	  case FONT_LARGE:
		return FONT_LARGE_HEIGHT;
	  case FONT_GRANT:
		return FONT_GRANT_HEIGHT;
	  case FONT_HUGE:
		return FONT_HUGE_HEIGHT;
	  case FONT_GIANT:
		return FONT_GIANT_HEIGHT;
	  default:
		return 10;	
	}  
}

uint8_t DisplayEDIP::getCurrentFontHeight() {
	return getFontHeight(currentFont);
}

// compute the number of pixel the passed string will need when drawn
uint8_t DisplayEDIP::getStringWidth(char* str) {
	if (str == NULL)
		return 0;

	// the following list of char width has been generated out of Segui20 with. It is 
	// the ratio of a character's width between min and max width, where min is ' and max is 'w'
	// (there are some strange characters above 127 which are wider, that's why the numbers go from 0..127 only )
	uint8_t charWidth[255-32+1] = {	19,	19,	29,	68,	59,	98,	88,	9,	19,	19,	39,	78,	9,	39,	9,	29,	59,	59,	59,	59,	59,	59,	59,	59,	59,	59,	9,	9,	78,	78,	78,	39,	128,	78,	59,	78,	78,	59,	49,	78,	88,	9,	29,	59,	49,	108,	88,	98,	59,	98,	59,	49,	59,	78,	68,	118,	68,	59,	68,	19,	29,	19,	78,	39,	9,	49,	68,	49,	68,	59,	19,	68,	59,	0,	0,	49,	0,	108,	59,	68,	68,	68,	29,	39,	19,	59,	49,	88,	39,	49,	49,	19,	9,	19,	78,	78,	59,	59,	9,	59,	29,	78,	29,	29,	29,	167,	49,	19,	128,	128,	68,	68,	68,	9,	9,	29,	29,	39,	59,	137,	19,	98,	39,	19,	128,	128,	49,	59,	19,	19,	59,	59,	59,	59,	9,	49,	39,	118,	29,	49,	78,	39,	118,	39,	39,	78,	29,	29,	9,	59,	49,	9,	0,	29,	39,	49,	98,	98,	98,	39,	78,	78,	78,	78,	78,	78,	108,	78,	59,	59,	59,	59,	9,	9,	9,	9,	78,	88,	98,	98,	98,	98,	98,	78,	98,	78,	78,	78,	78,	59,	59,	59,	49,	49,	49,	49,	49,	49,	108,	49,	59,	59,	59,	59,	0,	0,	0,	0,	59,	59,	68,	68,	68,	88,	68,	78,	68,	59,	59,	59,	59,	49,	68,	49};		
	uint16_t len = strlen(str);
	uint16_t x = 0;
	uint16_t maxFontWidth = getMaxFontWidth(currentFont);
	uint16_t minFontWidth = getMinFontWidth(currentFont);
	uint16_t diffFontWidth = maxFontWidth - minFontWidth;

	for (int i = 0;i<len;i++) {
		unsigned char c = str[i];
		if (c>=32) {
			uint8_t px = 1+minFontWidth + ((diffFontWidth*(int)charWidth[c-32])>> 7);
			x += px;
			/*
			Serial.print("c=");
			Serial.print(c);
			Serial.print("m=");
			Serial.print(minFontWidth);
			Serial.print("d=");
			Serial.print(diffFontWidth);
			Serial.print("w=");
			Serial.print((int)charWidth[c-32]);
			Serial.print("px=");
			Serial.println(px);
			*/
		}			
	}
	return x;	
}

void DisplayEDIP::setTextFont(char font) {
	char command [] = { 27,'Z','F',font };
	sendData(command,4);
  
	currentFont = font;
}

void DisplayEDIP::setTextAngle(char angle) {
  // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
  char command [] = {  27,'Z','W',angle };
  sendData(command,4);
}

void DisplayEDIP::drawText(int x1, int y1, char justification,const __FlashStringHelper* pText_P) {
	char PROGMEM* text_P = (char PROGMEM*)pText_P;
	byte len = strlen_P(text_P);
	byte i;

	char helper [len+8];

	char command [] = { 27,'Z',justification,
		lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
	};
	for (i=0;i<=sizeof(command)-1;i++) 
		helper[i] = command[i];
	
	for (i=0;i<=len;i++) {
	  	unsigned char c = pgm_read_byte(text_P++);
		helper[i+sizeof(command)] = c;
	}	
	sendData(helper,len+sizeof(command)+1);
}	

void DisplayEDIP::drawText(int x1, int y1, char justification,char* text) {

  byte len = strlen(text);
  byte i;

  char helper [len+8];

  char command [] = { 27,'Z',justification,
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
  };
  for (i=0;i<=6;i++) 
	helper[i] = command[i];
  for (i=0;i<=len;i++) 
	helper[i+7] = text[i];
	
  sendData(helper,len+8);
}

// draw a text in an area, fill the rest with background
// justification: 
// n1=1: Oben Links; n1=2: Oben Zentriert; n1=2 Oben Rechts
// n1=4: Mitte Links; n1=5: Mitte Zentriert; n1=6 Mitte Rechts
// n1=7: Unten Links; n1=8: Unten Zentriert; n1=9 Unten Rechts
void DisplayEDIP::drawTextInArea(int x1, int y1, int x2, int y2, int justification,char* text) {

  byte len = strlen(text);
  byte i;

  char command [] = { 27,'Z','B', 
		lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
		lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
		justification,
  };
  
  char helper [len+sizeof(command)+1];
  
  for (i=0;i<=sizeof(command)-1;i++) 
	helper[i] = command[i];
  for (i=0;i<=len;i++) 
	helper[i+sizeof(command)] = text[i];
	
  sendData(helper,len+sizeof(command)+1);
}

void DisplayEDIP::writeSentenceInArea(int x1, int y1, int x2, int y2, const __FlashStringHelper* pText_P, int &x3, int &y3) {
	char PROGMEM* text_P = (char PROGMEM*)pText_P;
	int8_t len = strlen_P(text_P);
	char buffer[len+1];
	strcpy_P(buffer,text_P);
	writeSentenceInArea(x1,y1,x2,y2,buffer,x3,y3);
}

// write text in area with word wrapping
// returns the position where the sentence ends.
void DisplayEDIP::writeSentenceInArea(int x1, int y1, int x2, int y2, char* text, int &x3, int &y3) {
	int8_t len = strlen(text);
	char str[len+1];
	strcpy(str, text); // copy, since strtok modifies the string
	
	char delimiters[] = {' ','!','.','.',',','-',':',';','?', char(13),char(10), char(0)};
		
	char* ptr = strtok(str, delimiters);
	int16_t x = x1;
	int16_t y = y1; 
	while (ptr != NULL) {
		uint8_t tokenLen = strlen(ptr);
		
		// since text is changed by strtok, use 
		// a new token string to copy and add the original delimiter
		char token[tokenLen+2];
		strcpy(token,ptr);
		char originalDelimiter = text[ptr-str + tokenLen];
		
		// if delimiter is a character, add it, otherwise omit it (chr(10) or chr(13)
		if (originalDelimiter>=32) {
			token[tokenLen++] = originalDelimiter; // assign the original delimiter char
			token[tokenLen] = 0;
 		}
		uint8_t lengthPx = getStringWidth(token); 
		// if word is too long, wrap it
		if (x + lengthPx > x2) {
			x = x1;
			y += getCurrentFontHeight();
		}
		drawText(x,y,'L',token);
		
		x +=  lengthPx;
		if ((x > x2) || (originalDelimiter == char(10)) || (originalDelimiter == char(13))) {
			y += getCurrentFontHeight();
			x = x1;
		}			
		
		// next token
		ptr = strtok(NULL, delimiters);
	}
	
	x3 = x;
	y3 = y;
}

void DisplayEDIP::sendTextToTerminal(char* text) {

  byte len = strlen(text);
  byte i;
  
  char command [] = { 27,'Z','T' };
  char helper [len+sizeof(command)+2];

  for (i=0;i<=sizeof(command)-1;i++) 
	helper[i] = command[i];
  for (i=0;i<=len;i++) 
	helper[i+sizeof(command)] = text[i];
	
  sendData(helper,len+sizeof(command));
}
 

void DisplayEDIP::drawLine(int x1, int y1, int x2, int y2) {

  char command [] = {
    27,'G','D',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
  };
  sendData(command,11);
}

 void DisplayEDIP::drawRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'G','R',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
  };
  sendData(command,11);
}


void DisplayEDIP::drawRectf(int x1, int y1, int x2, int y2, char color) {
  char command [] = {
    27,'R','F',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    color
  };
  sendData(command,12);
}

// fill with background color
void DisplayEDIP::deleteRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'R','L',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
  };
  sendData(command,sizeof(command));
}

// copy an area 
void DisplayEDIP::copyRect(int x1, int y1, int x2, int y2, int x3,int y3) {
  char command [] = {
    27,'R','C',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    lowByte(x3),highByte(x3),lowByte(y3),highByte(y3),

  };
  sendData(command,sizeof(command));
}

void DisplayEDIP::defineTouchKey(int x1, int y1, int x2, int y2, char down, char up, char* text) {
  byte len = strlen(text);
  byte i;
  char helper [len+13];
  char command [] = {
    27,'A','T',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    down,up
  };

  for (i=0;i<=12;i++) 
	helper[i] = command[i];
  for (i=0;i<=len;i++) 
	helper[i+13] = text[i];
  sendData(helper,len+14);
}

 

void DisplayEDIP::defineTouchSwitch(int x1, int y1, int x2, int y2, char down, char up, char* text) {
  byte len = strlen(text);
  byte i;
  char helper [len+13];
 char command [] = {
    27,'A','K',
    lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
    lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    down,up
  };
  for (i=0;i<=12;i++) 
	helper[i] = command[i];
  for (i=0;i<=len;i++) 
	helper[i+13] = text[i];
  sendData(helper,len+14);
}
 
void DisplayEDIP::setTouchSwitch(char code,char value) {

  char command [] = {
    27,'A','P',code,value
  };
  sendData(command,5);
}
 
void DisplayEDIP::setTouchkeyColors(char n1, char n2, char n3, char s1, char s2, char s3) {
  char command [] = {
    27,'F','E',n1,n2,n3,s1,s2,s3
  };
 sendData(command,9);
}

void DisplayEDIP::setTouchkeyFont(char font) {
  char command [] = {
   27,'A','F',font
  };
  sendData(command,4);

}


void DisplayEDIP::setTouchkeyLabelColors(char nf,char sf) {
  char command [] = {
    27,'F','A',nf,sf
  };
  sendData(command,5);
}

void DisplayEDIP::defineTouchArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  char command [] = {
    27,'A','H', lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),lowByte(x2),highByte(x2),lowByte(y2),highByte(y2) };
  sendData(command,sizeof(command));
}
 
// return true, if the passed buffer contains a touch event
bool DisplayEDIP::parseTouchEvent(char* pSendbuffer, TouchEventType &pType, uint16_t &pX, uint16_t &pY) {		
	if ((pSendbuffer[0] == 27) && (pSendbuffer[1] == 'H') && (pSendbuffer[2] == 5)) {
				// parse touch event		
		if (pSendbuffer[3] == 0)
			pType = DETOUCH_EVENT;
		else 
			if (pSendbuffer[3] == 1)
				pType = TOUCH_EVENT;
			else 
				if (pSendbuffer[3] == 2)
					pType = DRAG_EVENT;
				else {
					Serial.print(F("parseTouch: unknown type="));	
					Serial.println((int)pSendbuffer[3]);	
				}

  		pX = pSendbuffer[4] + (pSendbuffer[5]<<8);
		pY = pSendbuffer[6] + (pSendbuffer[7]<<8);


		return true;
	}
	return false;

}

void DisplayEDIP::setTouchGroup(char group) {
  char command [] = {
    27,'A','R',group
  };
  sendData(command,4);

}

void DisplayEDIP::removeTouchArea(char code,char n1) {
  char command [] = {
    27,'A','L',code,n1
  };
  sendData(command,5);
}


void DisplayEDIP::setBrightness(char n1) {
  char command [] = {
    27,'Y','H',n1
  };
  sendData(command,sizeof(command));
}
 
void DisplayEDIP::assignColor(char n1, char red,char green,char blue) {
  char command [] = {
    27,'F','P',n1, red,green,blue };
  sendData(command,sizeof(command));
}

void DisplayEDIP::setPage(uint8_t pgNo) {
  char command [] = {
    27,'M','K',pgNo };
  sendData(command,sizeof(command));
}

 
void DisplayEDIP::beep() {
	Serial.println(F("beep!"));
}

void DisplayEDIP::errorBeep() {
	Serial.println(F("beep! beep!"));
}

 

 

 


