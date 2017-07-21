// Keyboard.c
#define RELEASE_KEYBOARD 1.00			// Copyright 2016, HZB, ILL/SANE & ISIS

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//#include "main.h"
#include "keyboard.h"

// Variables for keyboard
extern volatile uint8_t pressed_key;			// Pressed key, checked in PCI_int
extern volatile uint8_t execute_pressed_key;	// Remember to execute the pressed key

// Enable interrupts on PORT D, C & A for front panel buttons
// inline void pin_interrupts_init(void)
// {
// 	Buttons on PORT D
// 	PCICR |= (1<<PCIE3);		// Enable interrupts PCINT31:24 (PORT D)
// 	PCMSK3 |= (1<<PCINT30) | (1<<PCINT29) | (1<<PCINT28) | (1<<PCINT27) | (1<<PCINT26);		// Enable only button pins
// 	PCINT30 for PD6 as S9	// PCINT29 for PD5 as S8	// PCINT28 for PD4 as S7
// 	PCINT27 for PD3 as S6	// PCINT26 for PD2 as S5
// 
// 	Buttons on PORT C
// 	PCICR |= (1<<PCIE2);		// Enable interrupts PCINT23:16 (PORT C)
// 	PCMSK2 |= (1<<PCINT23) | (1<<PCINT22) | (1<<PCINT19);		// Enable only button pins
// 	PCINT23 for PC7 as S1	// PCINT22 for PC6 as S3/S4		// PCINT19 for PC3 as S2
// 		
// 	Buttons on PORT A
// 	PCICR |= (1<<PCIE0);		// Enable interrupts PCINT7:0 (PORT A)
// 	PCMSK0 |= (1<<PCINT7);		// Enable only button pins
// 	PCINT7 for PC7 as S10
// }

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
	PORTA |= (1 << DDA5); 		// Set intern pull up for pin A5
	PORTA |= (1 << DDA7); 		// Set intern pull up for pin A7

	// Enable interrupts on PORT D, C & A for front panel buttons
	// Buttons on PORT D
	PCICR |= (1<<PCIE3);		// Enable interrupts PCINT31:24 (PORT D)
	PCMSK3 |= (1<<PCINT30) | (1<<PCINT29) | (1<<PCINT28) | (1<<PCINT27) | (1<<PCINT26);		// Enable only button pins
	// PCINT30 for PD6	// PCINT29 for PD5	// PCINT28 for PD4 	// PCINT27 for PD3	// PCINT26 for PD2

	//// Buttons on PORT C
	//PCICR |= (1<<PCIE2);		// Enable interrupts PCINT23:16 (PORT C)
	//PCMSK2 |= (1<<PCINT23) | (1<<PCINT22) | (1<<PCINT19);		// Enable only button pins
	//// PCINT23 for PC7 	// PCINT22 for PC6	// PCINT19 for PC3
		//
	//// Buttons on PORT A
	//PCICR |= (1<<PCIE0);		// Enable interrupts PCINT7:0 (PORT A)
	//PCMSK0 |= (1<<PCINT7);		// Enable only button pins
	//// PCINT7 for PC7
}

// Interrupts service routine for PORTD (// PCINT3_vect for PORTD // PCINT2_vect for PORTC // PCINT1_vect for PORTB // PCINT0_vect for PORTA)
ISR(PCINT3_vect)
{
	// Wake up CPU and save pressed button
	if(execute_pressed_key || pressed_key)
		return;
		
	switch(keyhit())
	{		
		//case KEY_FILL:
			//pressed_key = KEY_FILL;
			//execute_pressed_key = 1;
			//break;
		//case KEY_MEASURE:
			//pressed_key = KEY_MEASURE;
			//execute_pressed_key = 1;
			//break;
// 		case HIDDEN_FUNCTION:
// 			pressed_key = HIDDEN_FUNCTION;
// 			execute_pressed_key = 1;
// 			break;
		case KEY_TOP_S5:
			pressed_key = KEY_TOP_S5;
			execute_pressed_key = 1;
			break;
		case KEY_UP_S6:	
			pressed_key = KEY_UP_S6;
			execute_pressed_key = 1;
			break;
		case KEY_LEFT_S7:		
			pressed_key = KEY_LEFT_S7;
			execute_pressed_key = 1;
			break;
		case KEY_RIGHT_S8:		
			pressed_key = KEY_RIGHT_S8;
			execute_pressed_key = 1;
			break;
		case KEY_DOWN_S9:
			pressed_key = KEY_DOWN_S9;
			execute_pressed_key = 1;
			break;
//		case KEY_BOT_S10:
//			pressed_key = KEY_BOT_S10;
//			execute_pressed_key = 1;
//			break;
		default:			
			pressed_key = 0;
			execute_pressed_key = 0;
			break;
	}
}

// Get pressed key or 0,if no key pressed
int keyhit(void) 
{
	register unsigned char value = (~(PIND))&0b01111100;
	switch(value)
	{
		case 0b00000100:
			return KEY_TOP_S5;			// PIND2 (S5)
		case 0b00001000:	
			return KEY_UP_S6;	// PIND3 (S6)
		case 0b00010000:			
			return KEY_LEFT_S7;		// PIND4 (S7)
		case 0b00100000:			
			return KEY_RIGHT_S8;		// PIND5 (S8)
		case 0b01000000:			
			return KEY_DOWN_S9;			// PIND6 (S9)
	}
	
	value = (~(PINC))&0b11001000;
	switch(value)
	{
//		case 0b00001000:
//			return KEY_MEASURE;		// PINC3 (S2 - MEASURE - SWITCH ON)
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
			return KEY_BOT_S10;			// PINA7 (S10)
	}
	
	return 0;
}

// Freeze user interface and wait until user press a key
int getkey(void) 
{
	int v;
	while(keyhit()!=0);				// wait for release of key
	while((v = keyhit())==0);		// wait for a new pressed key
	_delay_ms(40);
	return v;						// Return the pressed key
}
