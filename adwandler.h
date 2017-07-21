#ifndef	ADWANDLER_H
#define	ADWANDLER_H

#include <avr/io.h>
#include <math.h>

#include "screenutils.h"
#include "timerutils.h"
#include "grafik.h"
#include "keyboard.h"

//ADC channels PortA
#define MAINVOLTAGE		0	
#define CURRENT 		1
#define VOLTAGE 		2
#define BATTERY 		3
#define PRESSURE 		4

#define ADC_LOOPS 		10

#define MEASURE_PIN_OFF 	PORTC&=~(1<<PC0);
#define MEASURE_PIN_ON	 	PORTC|=(1<<PC0);
#define CHECK_MEASURE_PIN 	(PINC & (1<<PC0))



void adc_init(uint8_t channel);
uint16_t readChannel(uint8_t mux, uint8_t avg);
uint16_t readChannel_calib(uint8_t channel, uint8_t avg, double adc_zero);

double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double t_nil, uint8_t show_progress);
uint8_t get_batt_level(double batt_min, double batt_max, double r_zero);

double round_double(double number, int digits);
double map_to_default(uint16_t adcVal);
double map_to_current(uint16_t adcVal);
double map_to_volt(uint16_t adcVal);
double map_to_batt(uint16_t adcVal);
int16_t map_to_pres(uint16_t adcVal, double zero, double span);
double calc_he_level(double res_x, double res_min, double res_max);

double map_to_test(uint16_t adcVal);

#endif //adwandler.h
