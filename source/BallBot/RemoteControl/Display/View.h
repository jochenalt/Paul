/*
 * View.h
 *
 * Created: 26.03.2013 12:59:48
 *  Author: JochenAlt
 */ 


#ifndef VIEW_H_
#define VIEW_H_


class View {
	
	public:
		virtual void touchEvent(uint8_t pTouchId) = 0;
		virtual void startup() = 0;
		virtual void teardown() {};
		virtual void loop() = 0;
		void drawTouchArrowUp(int pTouchid, int x, int y);
		void drawTouchArrowDown(int pTouchid, int x, int y);
		
		void drawTabbedWindowTitle(char pTopWindow[], char* pWindowTitle[], uint8_t pTouchAreaId[], uint8_t pTitles);
};

// touch area for all dialogues
#define NO_TOUCH_AREA 0
#define TOUCH_AREA_BACK 1

// touch areas of main dialogue
#define TOUCH_AREA_SPEECH 2
#define TOUCH_AREA_OPTIONS 3

// touch areas of speech dialogues
#define TOUCH_AREA_SPEECH_SONGS 4
#define TOUCH_AREA_SPEECH_TALK 5
#define TOUCH_AREA_SPEECH_POEMS 6

// touch areas of options dialogues
#define TOUCH_AREA_OPTIONS_PAUL 7
#define TOUCH_AREA_OPTIONS_CONFIG 8
#define TOUCH_AREA_OPTIONS_REMOTE 9
#define TOUCH_AREA_OPTIONS_Speech 10

// touch areas for options dialogue
#define TOUCH_AREA1_UP 20
#define TOUCH_AREA1_DOWN 21
#define TOUCH_AREA2_UP 22
#define TOUCH_AREA2_DOWN 23

// touch areas for poem dialoge
#define TOUCH_AREA_POEMS1 30
#define TOUCH_AREA_POEMS2 31

// touch areas for song dialoge
#define TOUCH_AREA_SONGS1 40
#define TOUCH_AREA_SONGS2 41

// touch areas of options
#define TOUCH_AREA_LANGUAGE 50
#define TOUCH_AREA_VOICE 51
#define TOUCH_AREA_BRIGHTNESS 52
#define TOUCH_AREA_THEME 53
#define TOUCH_AREA_POWER_OFF 54

// when touching the splash, go to main screen
#define TOUCH_AREA_SPLASH 60

// when touching the WAIT-FOR-PAUL-screen go to config
#define TOUCH_AREA_WAIT_FOR_PAUL 61

#define TOUCH_AREA_THEME_1 70
#define TOUCH_AREA_THEME_2 71
#define TOUCH_AREA_THEME_3 72
#define TOUCH_AREA_THEME_4 73
#define TOUCH_AREA_THEME_5 74
#define TOUCH_AREA_THEME_6 75


#endif /* VIEW_H_ */