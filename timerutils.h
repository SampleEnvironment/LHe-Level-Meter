#ifndef TIMERUTILS_H
#define TIMERUTILS_H


#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "screenutils.h"


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
	TIMER_5
};


extern volatile uint16_t t8_0_overflow;
extern volatile uint16_t t16_1_overflow;		///< TIMER1 overflow indicator


void init_0_timer8(void);
void timer8_0_start(void);	
void timer8_0_stop(void);

void init_1_timer16(void);
void timer16_1_start(void);	
void timer16_1_stop(void);

uint8_t set_timeout(uint16_t sec, uint8_t timer, uint8_t reset);
void timed_dialog(char *title, char *text, uint8_t timeout, unsigned int ForeColor, unsigned int BackColor); 

//16 bit Timer overflow
ISR(TIMER1_COMPA_vect);
//8 bit Timer overflow
ISR(TIMER0_OVF_vect); 


#endif  // timerutils.h
