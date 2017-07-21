// Display_utilities.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef DISPLAY_UTILITIES_H
#define DISPLAY_UTILITIES_H

#include <avr/io.h>
#include "display.h"

//Control PINs
#define DISPLAY_ON 				(PINB & (1<<PB3))    	// Set PORTB.3 as true
#define DISPLAY_TURN_ON 		PORTB|=(1<<PB3);		// Set PORTB.3 as true
#define DISPLAY_TURN_OFF 		PORTB&=~(1<<PB3);		// Set PORTB.3 as false

///Layout Colors
#define FGC yellow				///< Foreground color
#define BGC black				///< Background color
#define ERR	white				///< Warning/Active color
#define D_BGC blue				///< Dialog Background color
#define D_FGC white				///< Dialog Foreground color

//options strings 9 chars only!
//page 1
#define STRING_TRANSMIT_SLOW 			"Int.Slow:"
#define STRING_TRANSMIT_FAST 			"Int.Fast:"
#define STRING_HEAT_TIME 				"Pulse:"
#define STRING_MEASUREMENT_CYCLES 		"ADCcycs:"
#define STRING_FILLING_TIMEOUT 			"Timeout:"

//page 2
#define STRING_RES_MIN 					"R.Min:"
#define STRING_RES_MAX 					"R.Max:"
#define STRING_BATTMIN					"Bat.Min:"
#define STRING_BATTMAX					"Bat.Max:"
#define STRING_CRITVOLT					"Bat.Low:"

//page 3
#define STRING_POS 						"Pos:"
#define STRING_AUTOFILL 				"Autofill:"
#define STRING_HE_MIN_LVL 				"He.Min:"

//page 4
#define STRING_SPAN						"Pr.Span:"
#define STRING_ZERO						"Pr.Zero:"
#define STRING_ENABLE_PR				"Enable:"

//page 5
#define STRING_SHUTDOWN 				"Shutdown:"
#define STRING_DIAG 					"Diagmode:"
#define STRING_ADCSPAN					"R.Span:"
#define STRING_ADCZERO 					"R.Zero:"

// page 6
#define STRING_QUENCH_TIME 				"Quench:"
#define STRING_QUENCH_CURRENT 			"I quench:"
#define STRING_WAIT_TIME 				"Wait:"
#define STRING_MEAS_CURRENT 			"I meas:"
#define STRING_TOTAL_VOL 				"Volume:"

#define RIGHT 		1
#define LEFT 		0
#define	UPDATE 		3

#define DEVICE_ID 	1			// What is the interest and what is the link with the display ???
#define PASSWORD  	0

#define PAINT_ALL 	0
#define UPDATE_ONLY 1

// Call variables declared into main.c
// extern uint8_t dewar_volume;		// Max volume of Helium dewar

void draw_int(uint32_t number,uint8_t x, uint8_t y, char *unit, unsigned int color);
void draw_double(double number, uint8_t x, uint8_t y, uint8_t prec, char *unit, unsigned int color);
void paint_wait_screen(void);
void paint_he_level(double he_level, double total_volume);

void paint_batt(uint8_t batt, uint8_t critical_batt);
void paint_progress_bar(uint8_t x, uint8_t y, uint8_t progress);
void clear_progress_bar(uint8_t x, uint8_t y);
void draw_current_wait_time(uint8_t x, uint8_t y, uint16_t sec_must, uint16_t secs_is, unsigned int color);
void paint_buttons(char *top, char *bottom, uint8_t mitte);
void paint_enter_number(uint8_t param);
void paint_main(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update);
void paint_offline(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update);

void paint_options(uint8_t page);
void paint_opt_values_page1(_Bool transmit_slow_min, uint16_t transmit_slow, uint8_t transmit_fast, double heat_time, uint8_t meas_cycles, uint8_t fill_timeout);
void paint_opt_values_page2(double res_min, double res_max, double batt_min, double batt_max, uint8_t critical_batt);
void paint_opt_values_page3(char *device_pos, _Bool auto_fill_enabled, int8_t he_min, uint8_t fill_timeout, double wait_time);
void paint_opt_values_page4(double span, double zero, _Bool enable_pressure, double quench_current, double meas_current);
void paint_opt_values_page5(double adc_span, double adc_zero, double total_volume);

void paint_current_opt_page1(uint8_t option, uint8_t key);
void paint_current_opt_page2(uint8_t option, uint8_t key);
void paint_current_opt_page3(uint8_t option, uint8_t key);
void paint_current_opt_page4(uint8_t option, uint8_t key);
void paint_current_opt_page5(uint8_t option, uint8_t key);

void paint_measure(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update);
void paint_start_filling(char *letter, uint8_t number, _Bool update);

void paint_filling(uint16_t nr, char *position, double he_level, double total_volume, uint8_t batt, uint8_t critical_batt, _Bool update);
void update_login_digits(uint8_t welchen, uint8_t neuer_wert, uint8_t digit_davor, uint8_t richtung);
_Bool update_filling_pos(char (*pos_letters)[4], uint8_t (*pos_ranges)[21], char *pos);

uint16_t get_number(uint8_t *status, _Bool device_id);
void paint_diag(uint8_t page);
uint8_t toY(double value, uint8_t map_max, uint8_t y);

_Bool display_on(void);

#endif  // sreenutils.h
