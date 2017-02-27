/*
 * KeyboardController.h
 *
 * Created: 28.03.2013 17:23:20
 *  Author: JochenAlt
 */ 


#ifndef KEYBOARDCONTROLLER_H_
#define KEYBOARDCONTROLLER_H_


class KeyboardController {
	public:
		KeyboardController();

		void setup();
		void loop();
		
		int16_t getKey();

	private:
};

extern KeyboardController keyboard;

#endif /* KEYBOARDCONTROLLER_H_ */