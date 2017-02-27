/*
 * SpeechController.cpp
 *
 * Created: 18.02.2013 14:38:39
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "setup.h"
#include "SpeechController.h"
#include "SoftwareSerial.h"
#include "TimePassedBy.h"
#include <avr/wdt.h>
#include "MainMemory.h"

// songs are stolen from http://theflameofhope.co/dectalk%20speak%20window/
// char AndIiiiiiWillAlwaysLoveYouuuu_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 200][:n0][hxae<300,10>piy<300,10> brrrx<600,12>th<100>dey<600,10>tuw<600,15> yu<1200,14>_<300> hxae<300,10>piy<300,10> brrrx<600,12>th<100>dey<600,10> tuw<600,17>yu<1200,15>_<300> hxae<300,10>piy<300,10> brrrx<600,22>th<100>dey<600,19> dih<600,15>r eh<600,14>m<100,12>ih<350,12>k_<120>_<300> hxae<300,20>piy<300,20> brrrx<600,19>th<100>dey<600,15> tuw<600,17> yu<1200,15>][:n0]";

// unsupported:
// [:nb], [:nv] [:nr] [ap 90 gn 75}
// change all L<x> to LL<x>
// change all M<x> to MM<x>
// change all M<x> to MM<x>

// 44seconds
const char Edelweiss_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 180][:n0][:dv hs 95 br 0 as 90 ap 90 sm 90 ri 100][EY<800,15>DEL<400,18>VAY<900,25>S<300>EY<800,23>DEL<400,18>VAY<900,16>S<300>EH<800,15>VRIY<400>MOR<400>NIH<250,16>NX<150>YU<400,18>GRIY<1100,20>T<100>MIY<800,18>_<400>SMAO<500,15>LX<300>AE<250,18>N<100>D<50>WAY<1100,25>T<100>KLLIY<500,23>N<300>AE<250,18>N<100>D<50>BRAY<1100,16>T<100>YU<800,15>LLUH<300,18>K<100>HXAE<400>PIY<400,20>TUW<400,22>MIY<1000,23>TMIY<900,23>_<800>BLLAO<800,25>SAH<125,18>M<75>AH<250>V<150>SNOW<400,22>MEY<400,20>YU<400,18>BLLUW<500,15>M<300>AE<250,18>N<100>D<50>GROW<900,23>_<300>BLLUW<500,20>M<300>AE<250,23>N<100>D<50>GROW<800,25>FOR<400,23>EH<1200,22>VRR<800,18>_<400>EY<800,15>DEL<400,18>VAY<900,25>S<300>EY<800,23>DEL<400,18>VAY<900,16>S<300>BLLEH<600,15>S<200>MAY<400,18>HXOW<250>M<150>LLAE<250,20>N<100>D<50>FOR<400,22>EH<1200,23>VRR<800>][:n0]";
const char FavouriteThings1_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 190][:n2][:dv ap 200 sm 100 ri 100][R EY<200,17>N<100>DRAO<200,24>PS<100>AO<200>N<100>ROW<300,19>ZIX<200,17>Z<100>AE<150>N<100>D<50>WIH<300,12>SKRR<200,17>Z<100>AO<200>N<100>KIH<300,19>TAH<150,17>N<100>Z<50>_<300>BRAY<200>T<100>KAO<300,24>PRR<300>K EH<300,19>TEL<200,17>Z<100>AE<150>N<100>D<50>War<200,12>M<100>WUH<300,17>LL EH<200>N<100>MIH<300,19>TAH<150,17>N<100>Z<50>_<300>BRAW<200>N<100>PEY<300,24>PRR<300,22>PAE<300,17>KIH<300,19>JHIX<200,15>Z<100>TAY<200>D<100>AH<200,22>P<100>WIH<200,20>TH<100>STRIH<300,13>NX<200>Z<100>_<300>DHIY<200,12>Z<100>AR<300,13>AX<300,15>FYU<300,17>AH<200,18>V<100>MAY<300,20>FEY<300,22>VRR<300,24>EH<200,22>T<100>THIH<500,16>NX<300>Z<100>][:n0]";
const char FavouriteThings2_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 190][:n2][:dv ap 200 sm 100 ri 100][KRIY<200,17> M<100> KAH<300,24> LLRR<200> D<100> POW<300,19> NIY<200,17> Z<100> AE<150> N<100> D<50> KRIH<200,12> SP<100> AE<300,17> PEL<300> STRUW<300,19> DXEL<300,17> _<300> DOR<300> BEH<150,24> LX<100> Z<50> AE<150> N<100> D<50> SLLEY<300,19> BEH<150,17> LX<100> Z<50>AE<150> N<100> D<50> SHNIH<200,12> T<100> SEL<300,17> WIH<200> TH<100> NUW<300,19> DXEL<200,17> Z<100> _<300>WAY<150,17> LL<100> D<50> GIY<200,24> S<100> DHAE<200,22> T<100> FLLAY<300,17> WIH<200,19> TH<100> DHAX<300,15> MUW<200> N<100> AO<200,22> N<100> DHER<300,20> WIH<300,13> NX<200> Z<100> _<300>DHIY<200,12> Z<100> AR<300,13> AX<300,15> FYU<300,17> AH<200,19> V<100> MAY<300,20> FEY<300,22> VRR<300,24> EH<200,22> T<100> THIH<500,16> NX<300> Z<100>][:n0]";
const char FavouriteThings3_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 190][:n2][:dv ap 200 sm 100 ri 100][GRR<150,17> LL<100> Z<50> IH<200,24> N<100> WAY<200> T<100> DRREH<300,19> SIX<200,17> Z<100> WIH<200> TH<100> BLLUW<300,12> SAE<300,17> TAH<200> N<100> SAE<300,19> SHIX<200,17> Z<100> _<300> SNOW<300> FLLEY<200,24> KS<100> DHAE<200> T<100> STEY<300,19> AO<200,17> N<100> MAY<300> NOW<200,12> Z<100> AE<150,17> N<100> D<50> AY<300> LLAE<300,19> SHIX<200,17> Z<100> _<300>SIH<200> LL<100> VRR<300,24> WAY<200,22> T<100> WIH<200,17> N<100> TRR<200,19> Z<100> DHAE<200,15> T<100> MEH<150> LL<100> T<50> IH<200,22> N<100> TUW<300,20> SPRIH<400,13> NX<200> _<300> DHIY<200,12> Z<100> AR<300,13> AX<300,15> FYU<300,17> AH<200,19> V<100> MAY<300,20> FEY<300,22> VRR<300,23> EH<200,24> T<100> THIH<400,25> NX<100> Z<100> _<200>][:n0]";
const char FavouriteThings4_DecTalk_P[]	PROGMEM = "[:phone arpa speak on][:rate 190][:n2][:dv ap 200 sm 100 ri 100][WEH<200,24> N<100> DHAX<300> DAO<500> G<100> BAY<200,17> TS<100> _<300> WEH<200,22> N<100> DHAX<300> BIY<600> STIH<150,16> NX<100> Z<50> _<300> WEH<200,20> N<100> AY<200> M<100> FIY<600> LLIH<200,12> NX<100> SAE<1400,17> D<100> AY<300> SIH<200> M<100> PLLIY<300,19> RIY<300,17> MEH<200,19> M<100> BRR<300,17> MAY<300,19> FEY<300,20> VRR<300,22> EH<200,20> T<100> THIH<300,22> NX<200> Z<100> AE<150,20> N<100> D<50> DHEH<200,24> N<100> AY<300,25> DOW<150,24> N<100> T<70> FIY<600,25> LX<300> _<1000> SOW<900,24> BAE<2000,20> D<100>_<40>][:n0]";

const char HitMeWithYourBestShot_DecTalk_P[]	PROGMEM = "[:rate 230][:n2][:dv ap 200 sm 100 ri 100][WEH<125,12>LX<75>YXOR<200,14>AH<200,17>AXRIY<300,21>LL<100>TAH<300>F<100>KUH<200,19>KIY<200,17>WIH<125>TH<75>AX<200,14>LLAO<300,17>NX<100>HXIH<200>STOR<200,14>IY<400,19>_<500> AH<125,17>V<75> BRREY<200,19>K AH<125,17>N<75>LLIH<200,19>DXEL<200,17>HXAR<300,19>TS<100>LLAY<125>K<75>DHAX<200,17>WAH<125,19>N<75> IH<125>N<75>MIY<400,21>_<400>DHAE<200>TS<100>OW<200,19>KEY<400,17>LLEH<125,14>TS<75>SIY<400,17>HXAW<200>YU<200,14>DUW<400,19>IH<125,17>T<75>_<400>PUH<125,19>T<75>AH<125>P<75>YXOR<200,17>DUW<450,19>KS<150>LLEH<300,19>TS<100>GEH<125>T<75>DAW<300>N<100>TUW<400,21>IH<200,19>T<100>HXIH<125,21>T<75>MIY<200,19>WIH<125>TH<75>YXOR<200,17>BEH<300>ST<100>SHAO<300,21>T<100>_<1100>WAY<200,17>DOW<125,19>N<75>CHAX<200,17>HXIH<70,21>T<30>MIY<100,19>WIH<125>TH<75>YXOR<200,17>BEH<300,19>ST<100>SHAO<200,14>AO<300,12>T<100>_<1200>HXIH<125,21>T<75>MIY<200,19>WIH<125>TH<75>YXOR<200,17>BEH<300>ST<100>SHAO<300,21>T<100>_<800>FAY<200>RR<200,22>AX<200,24>WEH<400,19>EY<900,17>][:n0]";


SpeechController speechCtrl;

// connection to text2speech module EMIC-2
SoftwareSerial speechSerial(EMIC2_RX_PIN, EMIC2_TX_PIN); // RX, TX

void SpeechController::setup() {
	pinMode(EMIC2_RX_PIN, INPUT);
	pinMode(EMIC2_TX_PIN, OUTPUT);
	pinMode(EMIC2_SP_PLUS_PIN, INPUT);
	pinMode(EMIC2_TALKS_PIN, INPUT);

	// setup connection to EMIC-2
	speechSerial.begin(EMIC2_BAUDRATE);
	
	// talking is indicated
	pinMode(DONT_KNOW_LED,OUTPUT);
	
	// set values that will be improved with the first text
	peakAmplitude = 0;
	
	// initialize persistent stuff
	delay(10);
	setSpeedRate(memory.persistentMem.speechRate);
	delay(10);
	setLanguage(memory.persistentMem.language);
	delay(10);
	selectVoice(memory.persistentMem.voice);
	delay(10);
	setVolume(memory.persistentMem.volume);
}

void SpeechController::setDecTalkParser() {
	speechSerial.println("P0");
}

void SpeechController::setEpsonParser() {
	speechSerial.println("P1");
}

void SpeechController::sayVoltage(String beforeVoltage, String afterVoltage) {
	stopTalking();

	float volt = getCurrentVoltage();
	setEpsonParser();
	speechSerial.print("S ");
	speechSerial.print(beforeVoltage);
	speechSerial.print(int(volt));
	speechSerial.print(".");
	speechSerial.print(int(volt*10)-int(volt)*10);
	speechSerial.println(afterVoltage);
}
	

void SpeechController::sayEpson(const __FlashStringHelper* pLine_P) {
	stopTalking();

	setEpsonParser();
	speechSerial.print("S<<");
	send(pLine_P, true);
}

void SpeechController::sayDectalk(const __FlashStringHelper* pLine_P) {
	stopTalking();
	speechSerial.println("P0"); // dectalk parser
	speechSerial.print("S");
	send(pLine_P, false);
}


void SpeechController::send(const __FlashStringHelper* pLine_P, bool pWriteToBuffer) {
	stopTalking();

	if (textStrPtr != NULL) {
		textStrPtr = NULL;
	}
	textStrPtr = (char PROGMEM*)pLine_P;
	writeCurrentSpeechToBuffer = pWriteToBuffer;
	// send to serial is done in sayLoop
}

bool SpeechController::isTalking()  {
	return digitalRead(EMIC2_TALKS_PIN);
}

void SpeechController::stopTalking() {
	if (isTalking()) {
		speechSerial.println();
		speechSerial.println("X");
	}
}

boolean SpeechController::bufferEmpty() {
	return (textStrPtr == NULL);
}

void SpeechController::loop() {	
	
	boolean sendSpokenText = false;
	// if paul talks, the LED is on
	digitalWrite(DONT_KNOW_LED,isTalking());

	// call it dummywise, just to update the internal filters
	getCurrentAmplitude();
	
	// if there's something to say left
	if (textStrPtr != NULL) {
		// compute max characters to be send to EMIC
		// this is a dangerous part, since Softwareserial disables interrupts when a bit is sent, so 
		// take care that this fits into the time slot of one cycle (cylce takes 1000/REMOTECONTROL_FREQUENCY ms = 142ms)
		uint16_t maxCharacters = (EMIC2_BAUDRATE/14)/REMOTECONTROL_FREQUENCY / 2; // use up the half of the cycle time
		// max buffer of EMIC, allow him 100ms to send something to EMIC
		while ((maxCharacters--) > 0) {
			unsigned char c = pgm_read_byte(textStrPtr++);
			if (c == 0) {
				// string has been sent completely, end it with newline
				if (sendSpokenText)
					Serial.println();
				speechSerial.println();
				textStrPtr = NULL;
				break;
			}			
			if (sendSpokenText)
				Serial.print((char)c);
			speechSerial.print((char)c);
			if (writeCurrentSpeechToBuffer) {
				if ((c != '\\') && (c != '/') && (c != '<') && (c != '>'))
				writeToSpeechBuffer(c);
			}									
		}
	}		
}

void SpeechController::say(VoiceType pVoice, const __FlashStringHelper* pLine_P) {
	stopTalking();
	if (pVoice == ClintEastwood) {
		sayLikeClintEastwood(pLine_P);
	}
	else if (pVoice == RobotVoice) {
		sayLikeRobot(pLine_P);
	} else {
		selectVoice(pVoice);		
		setEpsonParser();
		speechSerial.print("S<<");
		send(pLine_P, true);
	}

}

void SpeechController::say(VoiceType pVoice, char pLine[]) {
	stopTalking();
	if (pVoice == ClintEastwood) {
		sayLikeClintEastwood(pLine);
	}
	else if (pVoice == RobotVoice) {
		sayLikeRobot(pLine);
	} else {
		selectVoice(pVoice);
		setEpsonParser();
		speechSerial.print("S");
		speechSerial.print(pLine);
		speechSerial.println(".");	
	}
}

void SpeechController::say(const __FlashStringHelper* pLine_P) {
	say(currentVoice,pLine_P);
}

void SpeechController::say(char pLine[]) {
	say(pLine);
}

void SpeechController::say(String pLine) {
	speechSerial.print("S");
	speechSerial.print(pLine);
	speechSerial.println(".");	
}


void SpeechController::sing(SongType pSong) {
	switch (pSong) {
		case EdelweissSong:
			sayDectalk(F_P(Edelweiss_DecTalk_P));
			break;
		case FavouriteThingsSong1:
			sayDectalk(F_P(FavouriteThings1_DecTalk_P));
			break;
		case FavouriteThingsSong2:
			sayDectalk(F_P(FavouriteThings2_DecTalk_P));
			break;
		case FavouriteThingsSong3:
			sayDectalk(F_P(FavouriteThings3_DecTalk_P));
			break;
		case FavouriteThingsSong4:
			sayDectalk(F_P(FavouriteThings4_DecTalk_P));
			break;
		case HitMeWithYourBestShotSong:
			sayDectalk(F_P(HitMeWithYourBestShot_DecTalk_P));
			break;				
		default:
			break;
	}	
}

void SpeechController::poem(PoemType pPoem) {
	switch (pPoem) {
		case RunChickenRun:
			sayEpson(F("Run chicken run. The farmers got the gun. The wife has the oven hot. And you are the one. So run and run. So you don't get served with a bun."));
			break;
		case ShaveMyLegs:
			sayEpson(F("Shave my legs shave my legs. Even though it is kind of hairy. Like my uncle Gary.Please do not let me beg."));
			break;
		case HowIHateTheNight:
			sayEpson(F("Now the world has gone to bed. Darkness won't engulf my head, I can see by infra-red. How I hate the night. Now I lay me down to sleep.Try to count electric sheep. Sweet dream wishes you can keep. How I hate the night."));         
			break;
		default:
			break;
	}	
}

void SpeechController::cmd(String pLine) {
	speechSerial.println(pLine);
}

void SpeechController::selectVoice(VoiceType pVoice /* 0..8 */) {
	if (currentVoice != pVoice) {
		int voiceNumber = pVoice;
		if (currentVoice <= ClintEastwood) {			
			speechSerial.print("N");
			speechSerial.println(char('0' + voiceNumber ));
		}		
		currentVoice = pVoice;	
	}
}

void SpeechController::setVolume(uint8_t pVolume /* 0..255 */) {
	if (pVolume != currentVolume) {
		currentVolume = pVolume;
	
		// EMIC want the volume between -48..+18
		int16_t emicVolume = (int16_t(pVolume)*(18+48))/256 - 48;
		speechSerial.print('V');
		speechSerial.println(emicVolume);	
	}
}

uint8_t SpeechController::getVolume() {
	return currentVolume;
}

// return current amplitude of speaker. 
// Returns value between 0..255 independent of volume by self-adaption.
uint8_t SpeechController::getCurrentAmplitude() {
	int16_t amplitude;
	if (amplitudeTimer.isDue_ms(1000/AMPLITUDE_MEASUREMENT_FREQUENCY)) {
		amplitude= analogRead(EMIC2_SP_PLUS_PIN);
	
		if (amplitude > peakAmplitude)
			peakAmplitude = amplitude; 
		uint16_t bottom = 483;
		uint16_t peak = 550;

		// compute number between 0..255 representing the amplitude
		lastAmplitude = amplitude-bottom;
		lastAmplitude = (int32_t(lastAmplitude)*256)/(peak-bottom);
		lastAmplitude = constrain(lastAmplitude,0,255); // should not apply ...
		
	}		

	return lastAmplitude;
}

void SpeechController::setSpeedRate(uint16_t pSpeed) { 
	if (pSpeed != currentSpeed) {
		stopTalking();
		speechSerial.println();
		speechSerial.print("W");
		speechSerial.println(pSpeed);
		speechSerial.println();
		currentSpeed = pSpeed;
	}	
}
void SpeechController::setLanguage(LanguageType pLangType) { 
	if (pLangType != currentLanguage) {
		stopTalking();
		speechSerial.println();
		switch (pLangType) {
			case English:
				speechSerial.println(F("L0"));
				break;
			case CastillianSpanish:
				speechSerial.println(("L1"));
				break;	
			case LatinSpanish:
				speechSerial.println(("L2"));
				break;	
			default:
				break;
		}		
		currentLanguage = pLangType;
	}		
}	

void SpeechController::sayLikeClintEastwood(const __FlashStringHelper* pLine_P) {
	textStrPtr = NULL;
	speechSerial.println();
	speechSerial.println(F("P0"));
	speechSerial.print(F("S[ :ra 160 :cp 50 :dv sm 100 la 20 hs 95 ap 100 br 45 g1 75] "));
	send(pLine_P, true);
	loop();
}

void SpeechController::sayLikeClintEastwood(char * pLine) {
	textStrPtr = NULL;
	speechSerial.println();
	speechSerial.println(F("P0"));
	speechSerial.print(F("S[ :ra 160 :cp 50 :dv sm 100 la 20 hs 95 ap 100 br 45 g1 75] "));
	speechSerial.print(pLine);
	speechSerial.println(F("."));
}

void SpeechController::sayLikeRobot(const __FlashStringHelper* pLine_P) {
	textStrPtr = NULL;
	speechSerial.println(F("P0"));
	speechSerial.print(F("S[:rate 200][:n0][:dv ap 90 pr 0] "));
	send(pLine_P, true);
	loop();
}

void SpeechController::sayLikeRobot(char * pLine) {
	textStrPtr = NULL;
	speechSerial.println();
	speechSerial.println(F("P0"));
	speechSerial.print(F("S[:rate 200][:n0][:dv ap 90 pr 0] "));
	speechSerial.print(pLine);
	speechSerial.println(F("."));
}

char SpeechController::readFromSpeechBuffer() {
	if (speechBufferLen > 0) {
		char c = speechBuffer[speechBufferReadIdx++];
		speechBufferLen--;
		if (speechBufferReadIdx >= sizeof(speechBuffer))
			speechBufferReadIdx  = 0;
		return c;
	}
	else 
		return char(0);
}

void SpeechController::writeToSpeechBuffer(char c) {
	if (speechBufferLen >= sizeof(speechBuffer))
		readFromSpeechBuffer(); // throw away and overwrite if buffer oveflow happens
		
	speechBuffer[speechBufferWriteIdx++] = c;
	speechBufferLen++;
	if (speechBufferWriteIdx >= sizeof(speechBuffer))
		speechBufferWriteIdx = 0;
}					

void SpeechController::printMenuHelp() {
	Serial.println();	
	Serial.print(F("Speech menu (amp="));
	Serial.print(peakAmplitude);
	Serial.println(')');

	Serial.println(F("t   - text2speech"));	
	Serial.println(F("c	  - command 2 emic2"));	
	Serial.println(F("f	  - say from flash"));	

	Serial.println(F("1..9   - sing 1..9"));	
	Serial.println(F("q	  - voice++"));	
	Serial.println(F("k	  - killing voice"));	
	Serial.println(F("l	  - clint eastwood"));	
	Serial.println(F("g	  - language"));	

	Serial.println(F("h   - this page"));	
	Serial.println(F("0   - main menu"));	
}

void SpeechController::menu() {
	String txt;
	bool textMode = false;
	bool execCmd = false;
	uint8_t voiceNumber = 0;
	uint8_t languageNo = 0;
	while (true) {
		wdt_reset();
		if (Serial.available())
			if (Serial.available()) {
				char inputChar = Serial.read();
				if (textMode) {
					if (inputChar == char(13) ||
						inputChar == char(10)) {
						Serial.print(F(" sent."));	
						if (execCmd)
							cmd(txt);
						else
							say(txt);
						Serial.println();
						textMode = false;						
						execCmd = false;
					} else {
						txt.concat(inputChar);
						Serial.print(inputChar);
					}						
				}					
				else
					switch (inputChar) {
						case '0':
							return;
							break;
						case '1':
							sing(EdelweissSong);
							break;
						case '2':
							sing(FavouriteThingsSong1);
							break;
						case '3':
							sing(FavouriteThingsSong2);
							break;
						case '4':
							sing(FavouriteThingsSong3);
							break;
						case '5':
							sing(FavouriteThingsSong4);
							break;
						case '6':
							sing(HitMeWithYourBestShotSong);
							break;
						case 'f':
							sayEpson(F_P("My mind is fading a way"));
							break;
						case 't':
							textMode=true;
							txt = "";
							Serial.print(F("text:"));
							break;
						case 'c':
							textMode=true;
							execCmd = true;
							txt = "";
							Serial.print(F("cmd:"));
							break;
						case 'q':
							voiceNumber++;
							if (voiceNumber>9)
								voiceNumber = 0;
							Serial.print(F("set voice "));
							Serial.println(voiceNumber);
							speechCtrl.selectVoice(VoiceType(voiceNumber));
							break;
						case 'g':
							languageNo++;
							if (languageNo>2)
								languageNo = 0;
							Serial.print(F("set language "));
							Serial.println(languageNo);
							speechCtrl.setLanguage((LanguageType)languageNo);
							break;

						case 'k':
							Serial.print(F("robot voice"));
							sayLikeRobot(F("I will kill you."));
							break;
						case 'l':
							Serial.print(F("clint eastwood voice"));
							sayLikeClintEastwood(F("Here I am. Brain like a planet."));
							break;

						case 'h':
							printMenuHelp();
							break;
						default:					
						break;
					}
			}
			loop();
	}												
}	