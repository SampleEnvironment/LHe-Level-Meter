#ifndef	KEYBOARD_H
#define	KEYBOARD_H

#include <avr/io.h>
#include <util/delay.h>

enum  {	
	KEY_LEFT_1= 1,
	KEY_LEFT_2,
	KEY_DOWN,
	KEY_UP,
	KEY_RIGHT_1,
	KEY_RIGHT_2,
	HIDDEN_FUNCTION,
	KEY_FILL
};

void keyboard_init(void);
int getkey(void);	//get new pressed key, waits until key pressed, return key 
int keyhit(void);	//get pressed key or 0,if no key pressed

#endif  //keyboard.h
