#ifndef KANNE_H
#define KANNE_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "grafik.h"
#include "keyboard.h"
#include "adwandler.h"
#include "usart.h"

#include "screenutils.h"
#include "commutils.h"
#include "timerutils.h"

#define arr_size(x) 			(sizeof(x)/sizeof(x[0]))
#define END 					0x7E

#define FILL_RANGES_COUNT 		15
#define FILL_RANGES_LENGTH 		21
#define FILL_LETTERS_LENGTH 	4						//3 chars + \0 !

//Control PINs
#define DISPLAY_ON 				(PINB & (1<<PB3))    	//pin b3 
#define DISPLAY_TURN_ON 		PORTB|=(1<<PB3);		
#define DISPLAY_TURN_OFF 		PORTB&=~(1<<PB3);

#define SHUTDOWN 				PORTC &=~(1<<PC5); 		// Shut down - set C5 at 0	    

#define DO_MEASUREMENT			!(PINC & (1<<PC3))		//low triggers! C3

#define AUTO_FILL_TURNED_ON 	!(PINC & (1<<PC1)) 		// pin c1 
#define START_AUTO_FILL 		PORTC |=(1<<PC2);		//set pin c2
#define STOP_AUTO_FILL 			PORTC &=~(1<<PC2);		//clear pin c2

#define XBEE_AWAKE_TIME			30						//sec


#define XBEE_SLEEP (\
			PORTC |= (1<<PC4);\
			delay_ms(20)\
		)			


#define XBEE_WAKE_UP (\
			PORTC &= ~(1<<PC4);\
			delay_ms(40)\
		)


#define SEP						0x2F					//slash
#define COM_TIMEOUT_TIME 		25						//s
#define WAIT_TIMEOUT_TIME 		500						//ms
#define DISP_TIMEOUT_TIME 		20						//s

//#define ST_OVERFLOW_HRS 		791
//#define ST_OVERFLOW_MIN 		(13.2)
#define OPTIONS_PASSWORD		1
#define NUMBER_OPTIONS_BYTES	21

#define ERROR_HE_LEVEL			1111

#define TRANSMIT_SLOW_DEF 		5
#define TRANSMIT_FAST_DEF 		1
#define RES_MIN_DEF 			(2.1)
#define RES_MAX_DEF 			(350.1)
#define HEAT_TIME_DEF 			(2.1)
#define MEASUREMENT_CYCLES_DEF 	20
#define FILLING_TIMEOUT_DEF 	30
#define AUTO_FILL_HE_DEF 		10
#define SPAN_DEF				(1.5)
#define ZERO_DEF				0
#define ZERO_NULLPUNKT			5000	//nur positive zahlen! oder code aendern
#define BATT_MIN_DEF			10
#define BATT_MAX_DEF			14
#define CRITICAL_BATT_DEF		20

//calibration
#define R_SPAN_DEF				350
#define R_ZERO_DEF				0

#define MIN_TRANSMIT_SLOW 		1
#define MAX_TRANSMIT_SLOW 		24
#define MIN_TRANSMIT_FAST 		1
#define MAX_TRANSMIT_FAST 		10
#define MIN_RES_MAX 			(0.1)
#define MAX_RES_MAX 			(999.9)
#define MIN_RES_MIN 			(0)
#define MAX_RES_MIN 			(9.9)
#define MIN_SPAN				1
#define	MAX_SPAN				2000
#define MIN_ZERO                (-2000)
#define MAX_ZERO                2000
#define MIN_BATTMIN				0
#define MAX_BATTMIN				12
#define MIN_BATTMAX				12
#define MAX_BATTMAX				30
#define MIN_CRITVOLT			0
#define MAX_CRITVOLT			100

#define MIN_HEAT_TIME 			(0.1)
#define MAX_HEAT_TIME 			(9.9)
#define MIN_MEASUREMENT_CYCLES 	1
#define MAX_MEASUREMENT_CYCLES 	50
#define MIN_FILLING_TIMEOUT 	(2*MIN_TRANSMIT_FAST)
#define MAX_FILLING_TIMEOUT 	30
#define MIN_AUTO_FILL_HE 		0
#define MAX_AUTO_FILL_HE 		80

#define SET_NETWORK_ERROR 			status_byte|=(1<<0);
#define SET_NO_REPLY_ERROR 			status_byte|=(1<<1);
#define SET_STARTED_FILLING_ERROR 	status_byte|=(1<<2);
#define SET_STOPED_FILLING_ERROR 	status_byte|=(1<<3);
#define SET_CHANGED_OPTIONS_ERROR 	status_byte|=(1<<4);
#define SET_SLOW_TRANSMISSION_ERROR status_byte|=(1<<5);

#define CLEAR_NETWORK_ERROR 			status_byte&=~(1<<0);
#define CLEAR_NO_REPLY_ERROR 			status_byte&=~(1<<1);
#define CLEAR_STARTED_FILLING_ERROR 	status_byte&=~(1<<2);
#define CLEAR_STOPED_FILLING_ERROR 		status_byte&=~(1<<3);
#define CLEAR_CHANGED_OPTIONS_ERROR 	status_byte&=~(1<<4);
#define CLEAR_SLOW_TRANSMISSION_ERROR 	status_byte&=~(1<<5);

#define CHECK_NETWORK_ERROR 			(status_byte & (1<<0))
#define CHECK_NO_REPLY_ERROR 			(status_byte & (1<<1))
#define CHECK_STARTED_FILLING_ERROR 	(status_byte & (1<<2))
#define CHECK_STOPED_FILLING_ERROR 		(status_byte & (1<<3))
#define CHECK_CHANGED_OPTIONS_ERROR 	(status_byte & (1<<4))
#define CHECK_SLOW_TRANSMISSION_ERROR 	(status_byte & (1<<5))

#define FILL_WAITING_LABEL				"time left: %d min"

#define ALLOW_COM
//#define ALLOW_DEBUG
#define ALLOW_DISPLAY_TIMER
#define ALLOW_LOGIN
#define ALLOW_XBEE_SLEEP
#define ALLOW_EEPROM_SAVING
#define ALLOW_DOUBLE_CLICK		// wenn da, 1 mal klicken = display_on + funktion


enum  modi{	
	MODE_MAIN_WINDOW = 1,
	MODE_FILL_ACTION,
	MODE_ERROR,
	MODE_OFFLINE,
	MODE_MEASURE_ACTION,
	MODE_OPTIONS_WINDOW,
	MODE_SHUTDOWN_ACTION,
	MODE_DIAG_ACTION,
	MODE_CALIBRATION
};

enum  error{
	NO_ERROR = 1,
	LOGIN_FAILED,
	OFFLINE
};

extern volatile uint8_t cmd_line;			// RS232 command ready for processing indicator
extern volatile uint8_t *send_str_reader;
extern volatile uint8_t sending_cmd;
extern volatile uint8_t status_byte;

bool display_on(void);
bool auto_fill_pin_on(void);
ISR(USART1_RX_vect);	
ISR(USART1_TX_vect); 
int main(void);


#endif  // kanne.h
