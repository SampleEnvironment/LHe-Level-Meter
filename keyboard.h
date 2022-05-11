// LVM.keyboard.h

#ifndef	KEYBOARD_H
#define	KEYBOARD_H
#include <avr/interrupt.h>
#include "main.h"
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

extern LVM_ModelType LVM;



// Add the following commands:

void keyboard_init(void);
ISR(PCINT3_vect);				// Interrupts service routine for keyboard
int keyhit_block(void);			// After Interrupt routine: Get pressed key or 0, if no key pressed: NORMAL ROUTINE
								// needs not_ready_for_new_key to be set to 0 somewhen afterwards
int keyhit_cont(void);			// After Interrupt routine: Get pressed key or 0, if no key pressed: NORMAL ROUTINE
								// sets not_ready_for_new_key to 0 so a new pressed key will be detected immediately after
int getkey(void);				// Freeze user interface and wait until user press a key
int keyhitwithoutinterrupt(void);	// Without interrupt routine: Get pressed key or 0,if no key pressed: ONLY TO WAIT FOR ANY BUTTON TO BE RELEASED
void ready_for_new_key(void);  // set not_ready_for_new_key to 0
void not_ready_for_new_key(void);
#endif  //LVM.keyboard.h
