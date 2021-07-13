// c
#define RELEASE_KEYBOARD 1.00			// Copyright 2016, HZB, ILL/SANE & ISIS

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//#include "main.h"
#include "keyboard.h"
#include "timer_utilities.h"
#include "display.h"
#include "display_utilities.h"

// Variables for keyboard


volatile uint8_t pressed_key = 0; // Pressed key, checked in PCI_int
volatile uint8_t  not_ready_for_newkey = 0; // not ready to accept a new key, to be set to 0 if a new key can be pressed
volatile uint8_t key_released = 0; // key was released


// Initialize keyboard
void keyboard_init(void)
{
	// Buttons mapping
	// S1 on PC7
	// S2 on PA5
	// S3/S4 on PC6
	// S5 on PD2
	// S6 on PD3
	// S7 on PD4
	// S8 on PD5
	// S9 on PD6
	// S10 on PA7
	
	// DDxn = 0 means Pxn as input // DDxn = 1 means Pxn as output
	// Set pins 2, 3, 4, 5 & 6 as input on PORTD
	DDRD &= ~((1 << DDD6) | (1 << DDD5) | (1 << DDD4) | (1 << DDD3) | (1 << DDD2));
	// Set pins 6 & 7 as input on PORTC
	DDRC &= ~((1 << DDC6) | (1 << DDC7));
	// Set pin 5 & 7 as input on PORTA
	DDRA &= ~((1 << DDA5) | (1 << DDA7));
	
	// Intern pull up
	//	PORTD |= 0b01111100;		// Set intern pull up for pins 2,3,4,5,6
	PORTD |= (1 << DDD6); 		// Set intern pull up for pin D6
	PORTD |= (1 << DDD5); 		// Set intern pull up for pin D5
	PORTD |= (1 << DDD4); 		// Set intern pull up for pin D4
	PORTD |= (1 << DDD3); 		// Set intern pull up for pin D3
	PORTD |= (1 << DDD2); 		// Set intern pull up for pin D2
	PORTC |= (1 << DDC6); 		// Set intern pull up for pin C6
	PORTC |= (1 << DDC7); 		// Set intern pull up for pin C7
	//	PORTA |= (1 << DDA5); 		// Set intern pull up for pin A5  // will be handeled by the hardware electronics
	PORTA |= (1 << DDA7); 		// Set intern pull up for pin A7

	// Enable interrupts on PORT D, C & A for front panel buttons
	// Buttons on PORT D
	PCICR |= (1<<PCIE3);		// Enable interrupts PCINT31:24 (PORT D)
	PCMSK3 |= (1<<PCINT30) | (1<<PCINT29) | (1<<PCINT28) | (1<<PCINT27) | (1<<PCINT26);		// Enable only button pins
	// PCINT30 for PD6	// PCINT29 for PD5	// PCINT28 for PD4 	// PCINT27 for PD3	// PCINT26 for PD2

	//// Buttons on PORT C
	PCICR |= (1<<PCIE2);		// Enable interrupts PCINT23:16 (PORT C)
	PCMSK2 |= (1<<PCINT23) | (1<<PCINT22) ;		// Enable only button pins
	//// PCINT23 for PC7 	// PCINT22 for PC6
	
	
	//// Buttons on PORT A
	PCICR |= (1<<PCIE0);		// Enable interrupts PCINT7:0 (PORT A)
	PCMSK0 |= (1<<PCINT7) | (1<<PCINT5);		// Enable only button pins
	//// PCINT7 for PA7 // PCINT5 for PA5
}

// Interrupts service routine for PORTD (// PCINT3_vect for PORTD // PCINT2_vect for PORTC // PCINT1_vect for PORTB // PCINT0_vect for PORTA)
void check_my_keyboard();
ISR(PCINT3_vect)
{
	check_my_keyboard();
}
ISR(PCINT2_vect)
{
	check_my_keyboard();
}
ISR(PCINT0_vect)
{
	check_my_keyboard();
}

void check_my_keyboard()
{
	// Wake up CPU and save pressed button
	
	//	int8_t timer_running = 0;
	/*	set_timeout(0, TIMER_6, USE_TIMER);		// timer running?
	set_timeout(0, TIMER_6, RESET_TIMER);				// reset timer
	set_timeout(2000, TIMER_6, USE_TIMER);				// start timer (10ms)
	*/

	int8_t temp_pressed_key = keyhitwithoutinterrupt();
	
	if (!temp_pressed_key)
	// no button is pressed (so some button must have been released in the meantime as this routine is only executed if an interrupt has occurred)
	key_released = 1;
	else
	{	// some button is pressed
		if (!(not_ready_for_newkey || pressed_key))
		// the last pressed_key was already read and the execution of the task was done
		{
			pressed_key = temp_pressed_key;
			not_ready_for_newkey = 1;
		}
	}


}

// Get pressed key or 0, if no key pressed
int keyhitwithoutinterrupt(void)	// Without interrupt routine: Get pressed key or 0, if no key pressed (the later to be used to wait for a key to be released)
{
	
	
	register unsigned char value = (~(PIND))&0b01111100;
	switch(value)
	{
		case 0b00000100:
		return (!LVM.options->display_reversed)? KEY_TOP_S5 : KEY_BOT_S10;			// PIND2 -> TOP (S5)
		case 0b00001000:
		return (!LVM.options->display_reversed)? KEY_UP_S6 : KEY_DOWN_S9;			// PIND3 -> UP (S6)
		case 0b00010000:
		return (!LVM.options->display_reversed)? KEY_LEFT_S7 : KEY_RIGHT_S8;			// PIND4 -> LEFT (S7)
		case 0b00100000:
		return (!LVM.options->display_reversed)? KEY_RIGHT_S8 : KEY_LEFT_S7;			// PIND5 -> RIGHT (S8)
		case 0b01000000:
		return (!LVM.options->display_reversed)? KEY_DOWN_S9 : KEY_UP_S6;			// PIND6 -> DOWN (S9)
	}
	
	value = (~(PINC))&0b11000000;
	switch(value)
	{
		case 0b01000000:
		return HIDDEN_FUNCTION;	// PINC6 (S3 & S4)
		case 0b10000000:
		return KEY_FILL;		// PINC7 (S1 - FILL)
	}
	
	value = (~(PINA))&0b10100000;
	switch(value)
	{
		case 0b00100000:
		return KEY_MEASURE;		// PINA5 (S2 - MEASURE - SWITCH ON)
		case 0b10000000:
		return (!LVM.options->display_reversed)? KEY_BOT_S10 : KEY_TOP_S5;			// PINA7 -> BOT (S10)
	}
	
	return 0;
}


int keyhit_block(void)
{
	int result = pressed_key;
	pressed_key = 0;
	return result;
}

int keyhit_cont(void)
{
	int result = keyhit_block();
	not_ready_for_newkey = 0;
	return result;
}


// Freeze user interface and wait until user press a key
int getkey(void)
{
	int v;
	while(keyhitwithoutinterrupt()!=0);				// wait for release of key
	while((v = keyhitwithoutinterrupt())==0);		// wait for a new pressed key
	_delay_ms(40);
	return v;						// Return the pressed key
}


void ready_for_new_key()  // set not_ready_for_new_key to 0
{
	not_ready_for_newkey = 0;  // set not_ready_for_new_key to 0
}

void not_ready_for_new_key()  // set not_ready_for_new_key to 0
{
	not_ready_for_newkey = 1;  // set not_ready_for_new_key to 1
}