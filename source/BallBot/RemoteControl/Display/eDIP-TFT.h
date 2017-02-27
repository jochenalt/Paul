/*
 * eDIP_TFT.h
 *
 * Created: 20.03.2013 13:31:48
 *  Author: JochenAlt
 */ 


#ifndef EDIP_TFT_H_
#define EDIP_TFT_H_


#include "Arduino.h" 

class DisplayEDIP {
  public:
	enum TouchEventType { TOUCH_EVENT, DETOUCH_EVENT, DRAG_EVENT };

	void setup(uint8_t pSSPin, uint8_t pSBUFPIN);
    bool requestSendBuffer(char* data, uint8_t pBufferSize, uint8_t &pLen);
	bool waitUntilReceiveBufferEmpty(void (*pSpareTimeLoop)(void)); 
	void requestBuffer(uint16_t &pSendBufferBytesReady, uint16_t &pReceiveBufferBytesReady);
	bool parseTouchEvent(char pSendbuffer[], TouchEventType &pType, uint16_t &pX, uint16_t &pY);

    void smallProtoSelect(char address);
    void smallProtoDeselect(char address);

    void sendData(char* data, char len);
    void clear();
    void invert();
    void setDisplayColor(char fg, char bg);
    void fillDisplayColor(char bg);

    void terminalOn(boolean on);

    void cursor(boolean on);
    void setCursor(char col, char row);
	
	// bitmap operations
	void setTransparencyByBackground();
	void setTransparencyByCorner();
	void setNoTransparency();
	void displayFlashPicture(int x, int y,uint8_t no);

	
	// bargraph operations (really ugly)
    void defineBargraph(char dir, char no, int x1, int y1, int x2, int y2, byte sv, byte ev, char type);
    void updateBargraph(char no, char val);
    void setBargraphColor(char no, char fg, char bg, char fr);
    void makeBargraphTouch(char no);
    void linkBargraphLight(char no);
    void deleteBargraph(char no,char n1);
	
	// instrument operatons (even uglier)
    void defineInstrument(char no, int x1, int y1, char image, char angle, char sv, char ev);
    void updateInstrument(char no, char val);
    void redrawInstrument(char no);
    void deleteInstrument(char no, char n1, char n2);
	
    void setTextColor(char fg, char bg);
    void setTextFont(char font);
    void setTextAngle(char angle);
	// x=0..319, y=0..239, justification='L', 'C', 'R'
	enum TextJustificatontype {LEFT, CENTER,RIGHT };
    void drawText(int x1, int y1, char justification,char* text);
    void drawText(int x1, int y1, char justification,const __FlashStringHelper* text_P);
    void drawTextInArea(int x1, int y1, int x2, int y2, int justification,char* text);
	void writeSentenceInArea(int x1, int y1, int x2, int y2, char* text, int &x3, int &y3);
	void writeSentenceInArea(int x1, int y1, int x2, int y2, const __FlashStringHelper* pText_P, int &x3, int &y3);

	void sendTextToTerminal(char* text);
	void defineTerminal(boolean pBig, uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2);
	void setTerminalColor(char fg, char bg);
	void clearTerminal();
	
    void setLineColor(char fg, char bg);
    void setLineThick(char x, char y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int x2, int y2);
    void drawRectf(int x1, int y1, int x2, int y2, char color);
	void deleteRect(int x1, int y1, int x2, int y2);
	void copyRect(int x1, int y1, int x2, int y2, int x3, int y3);

	// touch operations
    void defineTouchKey(int x1, int y1, int x2, int y2, char down, char up, char* text);
    void defineTouchSwitch(int x1, int y1, int x2, int y2, char down, char up, char* text);
    void setTouchSwitch(char code,char value);
    void setTouchkeyColors(char n1, char n2, char n3, char s1, char s2, char s3);
    void setTouchkeyFont(char font);
    void setTouchkeyLabelColors(char nf,char sf);
    void setTouchGroup(char group);
    void removeTouchArea(char code,char n1);
	void defineTouchArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

	// general operations
	void setBrightness(char n1);
	void assignColor(char n1, char red,char green,char blue);
	void setPage(uint8_t pgNo);
	uint8_t getStringWidth(char* str);
	uint8_t getFontHeight(char font);
	uint8_t getMaxFontWidth(char font);
	uint8_t getMinFontWidth(char font);
	uint8_t getCurrentFontHeight();


	void beep();
	void errorBeep();

  private:

	uint8_t ssPIN;
	uint8_t sBUFPIN;
    int useSmallProtocol;
	uint8_t currentFont;

    bool sendSmallDC1(char* data, char len);
    bool sendSmallDC2(char* data, char len);

};

extern DisplayEDIP lcd;

#endif /* EDIP-TFT_H_ */