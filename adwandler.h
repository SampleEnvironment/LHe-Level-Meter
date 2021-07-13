// Adwandler.h - Copyright 2016, HZB, ILL/SANE & ISIS

// Add the following commands:
//
// => void adc_init(uint8_t channel);
// => uint16_t readChannel(uint8_t mux, uint8_t avg);
// => uint16_t readChannel_calib(uint8_t channel, uint8_t nb_readings, double adc_zero);

// => double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double t_nil, uint8_t show_progress);
// => uint8_t get_batt_level(double batt_min, double batt_max, double r_zero);

// => double round_double(double number, int digits);
// => double map_to_default(uint16_t adcVal);
// => double map_to_current(uint16_t adcVal);
// => double map_to_volt(uint16_t adcVal);
// => double map_to_batt(uint16_t adcVal);
// => int16_t map_to_pres(uint16_t adcVal, double zero, double span);
// => double calc_he_level(double res_x, double res_min, double res_max);

// => double map_to_test(uint16_t adcVal);

#ifndef	ADWANDLER_H
#define	ADWANDLER_H

#define VOLTAGE_REF			(3.34)		// Voltage reference measured on the PCB
#define MAP_TO_CURRENT_FACTOR 0.29058   // Prop. constant between Current and measured ADC-Value
#define MAP_TO_CURRENT_OFFSET -2.2581   // Offset Current and measured ADC-Value
#define MAP_TO_VOLTAGE_FACTOR 0.0756	// Prop. constant between Voltage and measured ADC-Value
#define MAP_TO_VOLTAGE_OFFSET 0			// Offset between Voltage and measured ADC-Value
#define MAP_TO_BATT_FACTOR 0.01685		// Prop. constant between Battery voltage and measured ADC-Value
#define MAP_TO_BATT_OFFSET 0			// Offset between Battery voltage and measured ADC-Value
#define MAP_TO_PRESS_FACTOR 0.00978     // Prop. constant between pressure and measured ADC-Value will be corrected by span and zero

//Analog to Digital Converter channels on Port A
#define VOLT_SUPPLY_MEAS	0
#define CURRENT_PROBE_MEAS	1
#define VOLT_PROBE_MEAS		2
#define BATTERY 			3
#define PRESSURE 			4

#define ADC_LOOPS 			10

#define MEASURE_PIN_OFF 	PORTC&=~(1<<PC0);	// Set PORTC.0 to 0 to switch off the Wandler-L1720 (current supply board)
#define MEASURE_PIN_ON	 	PORTC|=(1<<PC0);	// Set PORTC.0 to 1 to switch on the Wandler-L1720 (current supply board)
#define CHECK_MEASURE_PIN 	(PINC & (1<<PC0))	// ???

extern LVM_ModelType LVM;


void adc_init(uint8_t channel);		// Initialize analog to digital converter on selected channel
double round_double(double number, int digits);	///rounds double number
double map_to_current(double adcVal);
double map_to_volt(double adcVal);
double map_to_default(double adcVal);
double map_to_test(double adcVal);
double map_to_batt(double adcVal);
double map_to_pres(double adcVal, double zero, double span);
double calc_he_level(double res_x, double res_min, double res_max);
double readChannel(uint8_t mux, uint16_t avg);
double readChannel_calib(uint8_t channel, uint8_t nb_readings, double adc_zero);
uint8_t get_batt_level(double batt_min, double batt_max);
//double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double t_nil, uint8_t show_progress);
double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double quench_time, double quench_current, double wait_time, double meas_current, uint8_t show_progress);

#endif //adwandler.h
