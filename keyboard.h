// Keyboard.h

#ifndef	KEYBOARD_H
#define	KEYBOARD_H

// Enumeration for keys
enum {
	KEY_FILL = 1,	// S1
	KEY_MEASURE,	// S2
	HIDDEN_FUNCTION,	// S3&S4
	KEY_TOP_S5,
	KEY_UP_S6,
	KEY_LEFT_S7,
	KEY_RIGHT_S8,
	KEY_DOWN_S9,
	KEY_BOT_S10
};

// Add the following commands:

void keyboard_init(void);
ISR(PCINT3_vect);				// Interrupts service routine for keyboard
int keyhit(void);				// Get pressed key or 0,if no key pressed
int getkey(void);				// Freeze user interface and wait until user press a key

#endif  //keyboard.h
