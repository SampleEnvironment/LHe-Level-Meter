// Timer_Utilities.h - Copyright 2016, HZB, ILL/SANE & ISIS


#ifndef TIMER_UTILITIES_H
#define TIMER_UTILITIES_H

#include <math.h>
#include <avr/interrupt.h>

#define OVERFLOW_IN_MS_8_BIT				((pow(2,8)*1000.0)*(1.0/(F_CPU/1024.0)))
#define OVERFLOW_IN_MS_16_BIT				((pow(2,16)*1000.0)*(1.0/(F_CPU/1024.0)))

#define BIG_BIT_DELAY 		OVERFLOW_IN_MS_16_BIT							//(4551)
#define SMALL_BIT_DELAY 	OVERFLOW_IN_MS_8_BIT							//(17.78)	///< ((2^8)*1000)*(1.0/(F_CPU/1024.0))  ms!

#define SEC 				floor((pow(2,16)/OVERFLOW_IN_MS_16_BIT)*1000.0)		//14400		//  (2^16/fcpu_calc) *1000 = 1 sec

#define RESET_TIMER 1
#define USE_TIMER 	0

//Timer0-5
enum  {
	TIMER_REF = 1,
	TIMER_0,
	TIMER_1,
	TIMER_2,
	TIMER_3,
	TIMER_4,
	TIMER_5,
	TIMER_6,
	TIMER_7
};

extern volatile uint32_t t8_0_overflow;
extern volatile uint16_t t16_1_overflow;		///< TIMER1 overflow indicator
extern volatile uint16_t t8_2_overflow;

//==============================================================
// Timer utilities commands
//==============================================================

void init_0_timer8(void);
void timer8_0_start(void);
void timer8_0_stop(void);

void init_1_timer16(void);
void timer16_1_start(void);
void timer16_1_stop(void);

void init_2_timer8(void);
void timer8_2_start(void);
void timer8_2_stop(void);

//8 bit Timer overflow
ISR(TIMER0_OVF_vect);
//16 bit Timer overflow
ISR(TIMER1_COMPA_vect);
//8 bit Timer overflow
ISR(TIMER2_OVF_vect);


///sets, checks and resets given Timer
///use only 16bit_number-1 as max waittime or tweak implementation
uint32_t set_timeout(uint16_t sec, uint8_t timer, uint8_t reset);

//dialog timer is the slow transmission timer! used only on shut down (slow transmit doesn't matter there)
void timed_dialog(char *title, char *text, uint8_t timeout, unsigned int ForeColor, unsigned int BackColor);


#endif  // timerutils.h
