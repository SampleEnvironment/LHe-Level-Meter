// Main.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_MAIN 1.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>		// There are several macros provided in this header file to actually put the device into sleep mode.
#include <util/delay.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "adwandler.h"
#include "display.h"
#include "display_utilities.h"
#include "keyboard.h"
#include "timer_utilities.h"
#include "usart.h"
#include "xbee.h"
#include "xbee_utilities.h"
#include "main.h"

volatile uint8_t cmd_line = 0;				// USART receive block
volatile uint8_t *send_str_reader;			// Pointer to the next byte to send via USART
volatile uint8_t sending_cmd=0;				// Number of bytes to send via USART
volatile uint8_t status_byte;				// Status byte	

// Variables for keyboard
volatile uint8_t pressed_key = 0;			// Pressed key, checked in PCI_int
volatile uint8_t execute_pressed_key = 0;	// Remember to execute the pressed key

// Declare and initialize global variables
// uint8_t dewar_volume = 0;					// Max volume of Helium dewar


//EEPROM vars
#ifdef ALLOW_EEPROM_SAVING
	//control
	//calibration
	uint16_t 	ee_r_span EEMEM = R_SPAN_DEF*10;
	uint16_t 	ee_r_zero EEMEM = R_ZERO_DEF;

	//options
	uint16_t 	ee_transmit_slow EEMEM = 0;
	uint8_t 	ee_transmit_slow_min EEMEM = 0;
	uint16_t 	ee_transmit_fast EEMEM = 0;
	uint8_t 	ee_transmit_fast_sec EEMEM = 0;
	uint16_t 	ee_quench_time EEMEM = 0;
	uint16_t 	ee_quench_current EEMEM = 0;
	uint16_t 	ee_wait_time EEMEM = 0;
	uint16_t 	ee_meas_current EEMEM = 0;
	uint8_t 	ee_meas_cycles EEMEM = 0;
	uint8_t 	ee_fill_timeout EEMEM = 0;
	uint8_t 	ee_he_min EEMEM = 10;
	uint16_t 	ee_res_min EEMEM = 0;
	uint16_t 	ee_res_max EEMEM = 0;
	uint16_t 	ee_span EEMEM = 0;
	uint16_t 	ee_zero EEMEM = 0;
	uint8_t		ee_enable_pressure = 1;
	uint16_t 	ee_batt_min EEMEM = 0;
	uint16_t 	ee_batt_max EEMEM = 0;
	uint8_t 	ee_critical_batt EEMEM = 0;
	uint16_t 	ee_total_volume EEMEM = 0;
#endif


//=========================================================================
// Check auto fill pin
//=========================================================================
inline _Bool auto_fill_pin_on(void)
{
	if(AUTO_FILL_TURNED_ON)
	{
		return 1; // If PC1 = 0 V
	}
	else {
		return 0;	 // If PC1 = VCC
	}
}

inline void diag_pulse(double *heat_time, double r_span, double r_zero)
{
	paint_diag(3);
	
	uint16_t diag_heat_timel_line = 0;
	uint8_t u_points[140];
	uint8_t i_points[140];
	uint16_t delay_between_points = ceil((*heat_time*1000)/50);
	
	uint16_t temp_ipoint = readChannel_calib(CURRENT_PROBE_MEAS, ADC_LOOPS, r_zero);
	u_points[0] = toY(map_to_volt( readChannel_calib(VOLT_PROBE_MEAS, ADC_LOOPS, r_zero) ), 35, 20);
	i_points[0] = toY(map_to_current(temp_ipoint), 100, 75);
	
	for(uint8_t i = 1; i<10;i++)
	{
		temp_ipoint = readChannel_calib(CURRENT_PROBE_MEAS, ADC_LOOPS, r_zero);
		u_points[i] = toY(map_to_volt( readChannel_calib(VOLT_PROBE_MEAS, ADC_LOOPS, r_zero) ), 35, 20);
		i_points[i] = toY(map_to_current(temp_ipoint), 100, 75);
		
		LCD_Draw(i+1, u_points[i-1],i+2, u_points[i],0,FGC);
		LCD_Draw(i+1, i_points[i-1],i+2, i_points[i],0,FGC);
		delay_ms(delay_between_points);
	}

	// Start PWM
	TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	DDRD |= (1 << PD7);			// Set PORTD.7 as output

	OCR2A = 160;

	MEASURE_PIN_ON				// Switch on the current supply board

	for(uint8_t i = 0; i<130;i++)
	{
		temp_ipoint = readChannel_calib(CURRENT_PROBE_MEAS, ADC_LOOPS, r_zero);
		u_points[i+10] = toY(map_to_volt( readChannel_calib(VOLT_PROBE_MEAS, ADC_LOOPS, r_zero) ), 35, 20);
		i_points[i+10] = toY(map_to_current(temp_ipoint), 100, 75);
		
		LCD_Draw(i+10,u_points[i+9],i+11,u_points[i+10],0,FGC);
		LCD_Draw(i+10,i_points[i+9],i+11,i_points[i+10],0,FGC);
		delay_ms(delay_between_points);
	}
	MEASURE_PIN_OFF

	// Stop PWM
	DDRD &= (0 << PD7);			// Set PORTD.7 as input

	diag_heat_timel_line = 2+10+40;
	LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,/*71*/126,0,ERR); // |
	draw_int(delay_between_points*(diag_heat_timel_line-2), 90, 2, "ms", ERR);
	
	_Bool back = true;
	while(back)
	{
		switch(keyhit())
		{
			case KEY_LEFT_S7:
						if(--diag_heat_timel_line < 25) 
						{
							diag_heat_timel_line++;
							break;
						}
						LCD_Draw(diag_heat_timel_line+1,20,diag_heat_timel_line+1,126,0,BGC);
						
						//draw old u point
						LCD_Plot(diag_heat_timel_line+1,u_points[diag_heat_timel_line-1], 0, FGC);
						//draw old i point
						LCD_Plot(diag_heat_timel_line+1,i_points[diag_heat_timel_line-1], 0, FGC);
						
						LCD_Plot(diag_heat_timel_line+1,71, 1, FGC);
						LCD_Plot(diag_heat_timel_line+1,126, 1, FGC);
						
						LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,126,0,ERR);
						
						if(1000 > (delay_between_points*(diag_heat_timel_line-3)))
							LCD_Print("      ", 90, 2, 2, 1, 1, FGC, BGC);
						else
							LCD_Print("     ", 90, 2, 2, 1, 1, FGC, BGC);
						draw_int(delay_between_points*(diag_heat_timel_line-3), 90, 2, "ms", ERR);
						_delay_ms(100);
						break;
			case KEY_RIGHT_S8:
						if(++diag_heat_timel_line > 135) 
						{
							diag_heat_timel_line--;
							break;
						}
						LCD_Draw(diag_heat_timel_line-1,20,diag_heat_timel_line-1,126,0,BGC);
						
						//draw old point
						LCD_Plot(diag_heat_timel_line-1, u_points[diag_heat_timel_line-3], 0, FGC);
						//draw old i point
						LCD_Plot(diag_heat_timel_line-1, i_points[diag_heat_timel_line-3], 0, FGC);
						
						
						LCD_Plot(diag_heat_timel_line-1,71, 1, FGC);
						LCD_Plot(diag_heat_timel_line-1,126, 1, FGC);
						
						LCD_Draw(diag_heat_timel_line,20,diag_heat_timel_line,126,0,ERR);
						
						//nur zu anzeige, je nach stellenanzahl, korrektur nur bei 3 stellen
						if(1000 > (delay_between_points*(diag_heat_timel_line-3)))
							LCD_Print("      ", 90, 2, 2, 1, 1, FGC, BGC);
						else
							LCD_Print("     ", 90, 2, 2, 1, 1, FGC, BGC);
						draw_int(delay_between_points*(diag_heat_timel_line-3), 90, 2, "ms", ERR);
						_delay_ms(100);
						break;
			case KEY_TOP_S5:
						*heat_time = (delay_between_points*(diag_heat_timel_line-3))/1000.0;
						#ifdef ALLOW_EEPROM_SAVING
							eeprom_write_word(&ee_quench_time, (uint16_t) ((*heat_time)*1000));
						#endif
						back = false;
						_delay_ms(100);
						break;
			case KEY_BOT_S10:
						back = false;
						_delay_ms(100);
						break;
			default:
						break;
		}
	}
}

void diag_page1(double r_zero, double r_span, double batt_min, double batt_max, double res_min, double res_max, double zero, double span)
{
	char temp[10];
	uint16_t diag_current_adc  	= readChannel_calib(CURRENT_PROBE_MEAS, 4*ADC_LOOPS, r_zero);
	uint16_t diag_voltage_adc 	= readChannel_calib(VOLT_PROBE_MEAS, 4*ADC_LOOPS, r_zero);
	uint16_t diag_pressure_adc 	= readChannel_calib(PRESSURE, ADC_LOOPS, r_zero);
	uint16_t diag_battery_adc 	= readChannel_calib(BATTERY, ADC_LOOPS, r_zero);
	
	if(diag_current_adc <= 0)	diag_current_adc = 1;
	
	double diag_res = 0;
	if( (diag_voltage_adc > 10) && (diag_current_adc > 10) ) 
	{
		diag_res = ((double) diag_voltage_adc/diag_current_adc)*r_span-10;
		//diag_res = ((diag_voltage_adc*3.34/1024)*23.06)/((diag_current_adc*3.34/1024)*0.08546);
	}
	double diag_battery_map	= get_batt_level(batt_min, batt_max, r_zero);
	double diag_battery_volt 	= map_to_batt(diag_battery_adc);
	double diag_pressure_map	= map_to_pres(diag_pressure_adc, zero, span);
	
	double he_level = calc_he_level(diag_res, res_min, res_max);
	
	// Resistance
	LCD_Print("         ", 40, 20, 2, 1, 1, FGC, BGC);
	if(he_level >= errCode_TooHighRes)
		LCD_Print("Res: disable", 2, 20, 2, 1, 1, FGC, BGC);
	else
		draw_double(diag_res, 40, 20, 1,"ohm", FGC);
	
	// He Liquid
	LCD_Print("         ", 40, 40, 2, 1, 1, FGC, BGC);
	if(he_level >= errCode_TooHighRes)
		LCD_Print("HeL: disable", 2, 40, 2, 1, 1, FGC, BGC);
	else
		draw_double(he_level, 40, 40, 1,"%", FGC);
	
	// Battery
	LCD_Print("     ", 40, 60, 2, 1, 1, FGC, BGC);
	draw_int(diag_battery_map, 50, 60,"%", FGC);
	LCD_Print("     ", 90, 60, 2, 1, 1, FGC, BGC);
	//draw_double(diag_battery_volt, 90, 60, 1,"V", FGC);
	dtostrf(diag_battery_volt,4,1,temp);
	strcat(temp, "V");
	LCD_Print("      ", 90, 60, 2, 1,1, FGC, BGC);
	LCD_Print(temp, 90, 60, 2, 1,1, FGC, BGC);
	
	// Pressure
	LCD_Print("         ", 40, 80, 2, 1, 1, FGC, BGC);
	draw_int(diag_pressure_map, 50, 80,"mbar", FGC);
	
	// Current
	(CHECK_MEASURE_PIN)?
		LCD_Print("ON ", 40, 100, 2, 1,1, ERR, BGC)
	:	LCD_Print("OFF", 40, 100, 2, 1,1, ERR, BGC);
}

inline void diag_page2(double r_zero)
{
	uint16_t diag_current_adc  = readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS);
	uint16_t diag_voltage_adc = readChannel(VOLT_PROBE_MEAS, ADC_LOOPS);
	uint16_t diag_main_voltage_adc = readChannel(VOLT_SUPPLY_MEAS, ADC_LOOPS);
	
	double diag_voltage_map = map_to_volt(diag_voltage_adc);
	double diag_current_map = map_to_current(diag_current_adc);
	double diag_main_voltage_map = map_to_volt(diag_main_voltage_adc);
	
	//ADCV
	LCD_Print("     ", 50, 20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc, 50, 20,"", FGC);
	LCD_Print("     ", 90, 20, 2, 1, 1, FGC, BGC);
	draw_int(diag_voltage_adc+r_zero, 90, 20,"", FGC);
	
	//ADCI
	LCD_Print("     ", 50, 40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc, 50, 40, "", FGC);
	LCD_Print("     ", 90, 40, 2, 1, 1, FGC, BGC);
	draw_int(diag_current_adc+r_zero, 90, 40,"", FGC);
	
	//I
	LCD_Print("       ", 50, 60, 2, 1, 1, FGC, BGC);
	draw_double(diag_current_map, 50, 60, 1,"mA", FGC);
	
	//U
	LCD_Print("       ", 50, 80, 2, 1, 1, FGC, BGC);
	draw_double(diag_voltage_map, 50, 80, 1,"V", FGC);
	
	//Us
	LCD_Print("       ", 50, 100, 2, 1, 1, FGC, BGC);
	draw_double(diag_main_voltage_map, 50, 100, 1,"V", FGC);
}

//=========================================================================
// MAIN PROGRAM
//=========================================================================
int main(void)  
{
	//=========================================================================
	// Declare Ports
	//=========================================================================
	// Shutdown pin (PORTB.1)
	DDRB |= (1<<DDB1);			// Set Pin B1 as output
	PORTB |= (1<<PB1);			// Set Pin B1 to maintain power supply

	// Measure button (PORTA.5)
	DDRA &= ~(1<<DDA5);			// Set Pin A5 as input
	PORTA |= (1<<PA5);			// Activate pull-up

	// Measurement pin (PORTC.0)
	DDRC |= (1<<DDC0);			// Set Pin C0 as output
	MEASURE_PIN_OFF				// Open the relay K1
	
	// XBee module (PORTA.6)
	DDRA |= (1<<DDA6);			// Set Pin A6 as output
	xbee_wake_up();				// Switch on Zigbee module
	
	// Display light control pin
	DDRB |= (1<<DDB3);			// Set Pin B3 as output
	DISPLAY_TURN_ON				// Turn on the display
	
	// Auto fill pin
	DDRC &= ~(1<<DDC1);			// Set Pin C1 as input
	PORTC |= (1<<PC1);			// Activate pull-up

	// Start_Auto_fill pin
	DDRB |= (1<<DDB0);			// Set Pin B0 as output
	PORTB &=~(1<<PB0);			// Clear Pin B0 // Added by JG on March 2017

	//=========================================================================
	// Initialization
	//=========================================================================
	LCD_Init();						// Initialize LCD
	keyboard_init();				// Initialize keyboard & interrupts on PORT D, C & A for front panel buttons
	usart_init(39);					// Initialize RS232 Communication, PRESCALE-Konstante, s. CPU-Manual oder usart.h
	adc_init(VOLT_SUPPLY_MEAS);		// Initialize ADC on PORTA.0 for measuring the voltage supply probe
	adc_init(CURRENT_PROBE_MEAS);	// Initialize ADC on PORTA.1 for measuring the current supply probe 
	adc_init(VOLT_PROBE_MEAS);		// Initialize ADC on PORTA.2 for measuring the voltage probe
	adc_init(BATTERY);				// Initialize ADC on PORTA.3 for measuring the battery voltage
	adc_init(PRESSURE);				// Initialize ADC on PORTA.4 for measuring the pressure gauge
	init_0_timer8();				// Initialize Timer0 (8-bit)
	init_1_timer16();				// Initialize Timer1 (16-bit)
	timer16_1_start();				// Timer 1 is always running

	//=========================================================================
	// Variables
	//=========================================================================
	uint16_t 	device_id = 0;						// Identification of the device (defined in display_utils)
	
	volatile uint16_t	ex_mode = ex_mode_main_window;
	volatile uint16_t	ex_errorCode = ex_errorCode_NoError;
	
	double 		he_level = 0;						// Init and clear Helium level
//	double		last_he_level = 200;
	_Bool		very_low_He_level = false;			// Flag for very low Helium level (< level min)
	uint8_t 	batt_level = 0;						// Init and clear battery level
	uint16_t 	pressure_level = 0;					// Init and clear pressure value
//	uint8_t 	meas_progress = 0;			
	
	uint8_t		saved_mode = 0;						// Variable to save the running mode before to change it
	_Bool		force_measurement = false;			// True if there is a user request for measurement
	_Bool 		forced_measurement_done = false;	// True if requested forced measurement done 
	
	// Device position (instruments or labs)
	char 		device_pos[5] = "POSI";		
	// Default positions, examples
	char 		fill_pos_letters[FILL_RANGES_COUNT][FILL_LETTERS_LENGTH] = {"\r","AB","C", "DEF","G", "H", "\r"};
	uint8_t 	fill_pos_ranges[FILL_RANGES_COUNT][FILL_RANGES_LENGTH] = {{END},{END,1,2,3,4,5,7,19,END}, {END,2,3,4,END}, {END,7,8,12,END}, {END,2,7,4,11,15,17,END}, {END,1,2,END}, {END}};
	
	//diag pages
	uint8_t 	diag_to_show = 1;
	
	double 		r_span = R_SPAN_DEF;
	double 		r_zero = R_ZERO_DEF;
	double 		temp_r_span = 0;
	double 		temp_r_zero = 0; 
	
	// Variables for XBee interface
	uint8_t		xbee_sleep_period = 1;
	_Bool 		xbee_sleeping = false;
	_Bool		xbee_busy = false;			// Flag for XBee activity
	//Options vars
	uint16_t	options_pw = OPTIONS_PASSWORD;
	uint16_t	entered_options_pw = 0;
	
	// Variables for time intervals for network connections
	uint16_t 	transmit_slow = TRANSMIT_SLOW_DEF;			// Define time interval between regular network connections (5 hours as default)
	_Bool 		transmit_slow_min = false;					// Time interval between regular network connections defined in minutes
	_Bool 		transmit_slow_changed = false;				// Set true if time interval between regular network connections has been changed
	uint16_t 	transmit_fast = TRANSMIT_FAST_DEF;			// Define reduced time interval when Helium is filling (1 minute as default)
	_Bool 		transmit_fast_sec = false;					// Reduced time interval between network connections defined in seconds

	_Bool 		auto_fill_enabled = false;
	_Bool 		auto_fill_started = false;

	double 		quench_time = QUENCH_TIME_DEF;				// Quench duration in seconds
	double 		quench_current = QUENCH_CURRENT_DEF;		// Quench current in mA
	double 		wait_time = WAIT_TIME_DEF;					// Stabilisation duration in seconds
	double 		meas_current = MEAS_CURRENT_DEF;			// Measurement current in mA

	double		total_volume = TOTAL_VOL_DEF;
	
	uint8_t 	meas_cycles = MEASUREMENT_CYCLES_DEF;
	uint8_t 	fill_meas_counter = 0;
	uint8_t 	fill_timeout = FILLING_TIMEOUT_DEF;
	int8_t 		he_min = AUTO_FILL_HE_DEF;
	double 		res_min = RES_MIN_DEF;
	double 		res_max = RES_MAX_DEF;
	double 		span = SPAN_DEF;
	double 		zero = ZERO_DEF;
	_Bool		enable_pressure = false;
	double 		batt_min = BATT_MIN_DEF;
	double 		batt_max = BATT_MAX_DEF;
	uint8_t 	critical_batt = CRITICAL_BATT_DEF;
	
	#ifdef ALLOW_EEPROM_SAVING
		r_span 				= (double) (eeprom_read_word(&ee_r_span)/10.0);			if(r_span < 0) 		r_span = 350.0;
		r_zero 				= (double) (eeprom_read_word(&ee_r_zero)/10.0);			if(r_zero < 0) 		r_zero = 0;
		temp_r_span 		= r_span;
		temp_r_zero 		= r_zero;
		transmit_slow 		= eeprom_read_word(&ee_transmit_slow); 						if(transmit_slow < 0) 		transmit_slow = TRANSMIT_SLOW_DEF;
		transmit_slow_min 	= eeprom_read_byte(&ee_transmit_slow_min);					if(transmit_slow == TRANSMIT_SLOW_DEF) 	transmit_slow_min = false;
		transmit_fast 		= eeprom_read_word(&ee_transmit_fast);						if(transmit_fast < 0) 		transmit_fast = TRANSMIT_FAST_DEF;
		transmit_fast_sec	= eeprom_read_byte(&ee_transmit_fast_sec);					if(transmit_fast == TRANSMIT_FAST_DEF) 	transmit_fast_sec = false;
		quench_time			= (double) (eeprom_read_word(&ee_quench_time)/1000.0);		if(quench_time < QUENCH_TIME_MIN) 			quench_time = QUENCH_TIME_DEF;
		quench_current		= (double) (eeprom_read_word(&ee_quench_current));			if(quench_current < QUENCH_CURRENT_MIN) 	quench_current = QUENCH_CURRENT_DEF;
		wait_time			= (double) (eeprom_read_word(&ee_wait_time)/1000.0);		if(wait_time < WAIT_TIME_MIN) 				wait_time = WAIT_TIME_DEF;
		meas_current		= (double) (eeprom_read_word(&ee_meas_current));			if(meas_current < MEAS_CURRENT_MIN)			meas_current = MEAS_CURRENT_DEF;
		meas_cycles 		= eeprom_read_byte(&ee_meas_cycles);						if(meas_cycles < 0) 		meas_cycles = MEASUREMENT_CYCLES_DEF;
		fill_timeout 		= eeprom_read_byte(&ee_fill_timeout);						if(fill_timeout < 0) 		fill_timeout = FILLING_TIMEOUT_DEF;
		he_min 				= eeprom_read_byte(&ee_he_min);								if(he_min > 100) 			he_min = AUTO_FILL_HE_DEF;
		res_min 			= (double) (eeprom_read_word(&ee_res_min)/10.0);			if(res_min < 0) 			res_min = RES_MIN_DEF;
		res_max 			= (double) (eeprom_read_word(&ee_res_max)/10.0);			if(res_max < 0) 			res_max = RES_MAX_DEF;
		span 				= (double) (eeprom_read_word(&ee_span)/10.0);				if(span < 0) 				span = SPAN_DEF;
		zero 				= (double) ((eeprom_read_word(&ee_zero)/10.0)-ZERO_NULLPUNKT);	if(zero < 0) 				zero = ZERO_DEF;
		enable_pressure 	= eeprom_read_byte(&ee_enable_pressure);					//if(enable_pressure < 0) 	enable_pressure = 1;
		batt_min 			= (double) (eeprom_read_word(&ee_batt_min)/10.0);			if(batt_min < 0) 			batt_min = BATT_MIN_DEF;
		batt_max 			= (double) (eeprom_read_word(&ee_batt_max)/10.0);			if(batt_max < 0) 			transmit_fast = BATT_MAX_DEF;
		critical_batt 		= eeprom_read_byte(&ee_critical_batt);						//if(critical_batt < 0) 		critical_batt = CRITICAL_BATT_DEF;
		total_volume		= (double) (eeprom_read_word(&ee_total_volume));			if(total_volume < TOTAL_VOL_MIN)	total_volume = TOTAL_VOL_DEF;
	#endif
	
	//Options flow control vars
	uint8_t 	active_option = 1;
	uint8_t 	active_value = 0;
	uint8_t 	active_page = 1;				// ID of active page
	_Bool 		options_changed = false;		// Set true if device settings are changed

	//=========================================================================
	// Conversion/temp variables
	//=========================================================================
	char 		temp[SINGLE_FRAME_LENGTH];
	uint8_t 	buffer[SINGLE_FRAME_LENGTH];

	//=========================================================================
	// XBee variables
	//=========================================================================
	uint32_t 	dest_low = 0;	//0x1234; 
	uint32_t 	dest_high = 0;	//0x5678;
	
	//=========================================================================
	// Enable global interrupts
	//=========================================================================
	sei();
	
	//=========================================================================
	// Display connection is in progress
	//=========================================================================
	// Clear screen	
	LCD_Cls(BGC);
	
	LCD_Print("Network connection", 5, 40, 2, 1, 1, FGC, BGC);
	LCD_Print("in progress ...", 5, 60, 2, 1, 1, FGC, BGC);
	
	//=========================================================================
	// Try to establish connection to the network 
	//=========================================================================
	#ifdef ALLOW_LOGIN
		int loop = 1;
		
		while(loop)
		{
			if(xbee_reset_connection())
			{
				if((dest_low = xbee_get_addr(DL_MSG_TYPE)) && (dest_high = xbee_get_addr(DH_MSG_TYPE))) 
				{
					break;
				}
			}
			LCD_Cls(BGC);
			LCD_Print("Connection", 5, 20, 2, 1, 1, FGC, BGC);
			LCD_Print("to network", 5, 40, 2, 1, 1, FGC, BGC);
			LCD_Print("failed!", 5, 75, 2, 1, 1, ERR, BGC);
			LCD_Print("Press any key to try again", 2, 110, 1, 1, 1, FGC, BGC);
			
			getkey();
			set_timeout(1000, TIMER_0, USE_TIMER);
			while(keyhit() != HIDDEN_FUNCTION)	
			{
				if(!set_timeout(0, TIMER_0, USE_TIMER)) 
				{
					break;	
				}
			}
			if(set_timeout(0,TIMER_0, USE_TIMER)) 
			{
				_delay_ms(500);
				if(keyhit() == HIDDEN_FUNCTION)
				{
					ex_mode = ex_mode_offline;
					ex_errorCode = ex_errorCode_Offline;
					break;
				}
			}
			LCD_Cls(BGC);
				LCD_Print("Connecting", 5, 40, 2, 1, 1, FGC, BGC);
				LCD_Print("to network ...", 5, 60, 2, 1, 1, FGC, BGC);
				
			loop -=1;
		}	// end of infinite loop

		LCD_Cls(BGC);
		LCD_Print("Connection", 5, 40, 2, 1, 1, FGC, BGC);
		LCD_Print("to network", 5, 60, 2, 1, 1, FGC, BGC);
		LCD_Print("succeeded", 5, 80, 2, 1, 1, FGC, BGC);
		_delay_ms(1000);
		
		//=========================================================================
		// Device Login 
		//=========================================================================
		if(ex_mode != ex_mode_offline)
		{	
			//enter device number
			_Bool login_okay = false;
			uint8_t status = 0;
			_Bool bad_id = false;
		
			while(!login_okay)
			{
				if(!bad_id)
				{
					device_id = get_number(&status, DEVICE_ID); // User prompt - Return the vessel number

					if(status) // Pressed cancel or hidden mode
					{
						login_okay = true;
						ex_mode = ex_mode_offline;
						ex_errorCode = ex_errorCode_Offline;
						break;
					}
				
					if(!device_id) continue;   // skip null ID
					sprintf(temp,"Do you really want\nto connect vessel %d\nto server?",device_id);
				}
				else sprintf(temp,"No vessel with ID\n%d found,\nstill login?",device_id);
			
				if (LCD_Dialog("Server", temp, D_FGC, D_BGC))
				{
					//bei ja senden
					status = (bad_id)?	
						xbee_send_login_msg(FORCE_LOGIN_MSG, device_id, buffer, dest_high, dest_low)
					:	xbee_send_login_msg(LOGIN_MSG, device_id, buffer, dest_high, dest_low);
				
					switch(status)
					{
						case 0:			
							//GOOD_OPTIONS:	
							// Transmit slow in minutes
							transmit_slow = (buffer[0]<<8) + buffer[1];
							transmit_slow_min = true;
						
							if(transmit_slow <= 0) 
							{
								transmit_slow = TRANSMIT_SLOW_DEF;
								transmit_slow_min = false;
							}
							if(transmit_slow > 60)
							{
								transmit_slow /= 60;
								transmit_slow_min = false;
							}
						
							// Transmit fast in seconds
							transmit_fast = (buffer[2]<<8) + buffer[3];
							transmit_fast_sec = true;

							if(transmit_fast <= 0) 
							{
								transmit_fast = TRANSMIT_FAST_DEF;
								transmit_fast_sec = false;
							}
							if(transmit_fast > 60)
							{
								transmit_fast /= 60;
								transmit_fast_sec = false;
							}
						
							// Minimum resistance multiplied by factor 10
							res_min = (double)(((buffer[4]<<8) + buffer[5])/10.0);
							if(res_min <= 0) res_min = RES_MIN_DEF;
						
							// Maximum resistance multiplied by factor 10
							res_max = (double)(((buffer[6]<<8) + buffer[7])/10.0);
							if(res_max <= 0) res_max = RES_MAX_DEF;
						
							// Quench time in ms - should be stored in seconds
							quench_time = round_double(((buffer[8]<<8) + buffer[9])/1000.0, 1);
							if(quench_time <= 0) quench_time = QUENCH_TIME_DEF;
						
							// Quench current in mA
							quench_current = (double)(((buffer[10]<<8) + buffer[11]), 1);
							if(quench_current <= 0) quench_current = QUENCH_CURRENT_DEF;
						
							// Wait time in ms - should be stored in seconds
							wait_time = round_double(((buffer[12]<<8) + buffer[13])/1000.0, 1);
							if(wait_time <= 0) wait_time = WAIT_TIME_DEF;

							// Measurement current in mA
							meas_current = (double)(((buffer[14]<<8) + buffer[15]));
							if(meas_current <= 0) meas_current = MEAS_CURRENT_DEF;

							// Number of measuring cycles of He probe
							meas_cycles = (!buffer[16])? MEASUREMENT_CYCLES_DEF : buffer[16];
							
							// Timeout while filling
							fill_timeout = (!buffer[17])? FILLING_TIMEOUT_DEF : buffer[17];
						
							// Span & zero
							span = ((buffer[18]<<8) + buffer[19])/10.0;
							if(span <= 0) span = SPAN_DEF;
						
							zero = (double)((((buffer[20]<<8) + buffer[21])/10.0)-ZERO_NULLPUNKT);
							//zero = ZERO_NULLPUNKT - zero;
						
							// Total volume of the dewar multiplied by factor 10
							total_volume = round_double(((buffer[22]<<8) + buffer[23])/10.0, 1);
							if(total_volume <= 0) total_volume = TOTAL_VOL_DEF;
							
							// Minimum voltage of the battery multiplied by factor 10
							batt_min = round_double(((buffer[24]<<8) + buffer[25])/10.0, 1);
							if(batt_min <= 0) batt_min = BATT_MIN_DEF;
							
							// Maximum voltage of the battery multiplied by factor 10
							batt_max = round_double(((buffer[26]<<8) + buffer[27])/10.0, 1);
							if(batt_max <= 0) batt_max = BATT_MAX_DEF;
						
							// Critical percentage of battery
							critical_batt = (!buffer[28])? CRITICAL_BATT_DEF : buffer[28];
						
							login_okay = true;
							break;
						case 1:			
							//BAD_OPTIONS:
							transmit_slow = TRANSMIT_SLOW_DEF;
							transmit_slow_min = false;
							transmit_fast = TRANSMIT_FAST_DEF;
							transmit_fast_sec = false;
							res_min = RES_MIN_DEF;
							res_max = RES_MAX_DEF;
							quench_time = QUENCH_TIME_DEF;
							quench_current = QUENCH_CURRENT_DEF;
							wait_time = WAIT_TIME_DEF;
							meas_current = MEAS_CURRENT_DEF;
							meas_cycles = MEASUREMENT_CYCLES_DEF;
							fill_timeout = FILLING_TIMEOUT_DEF;
							span = SPAN_DEF;
							zero = ZERO_DEF;
							total_volume = TOTAL_VOL_DEF;
							batt_min = BATT_MIN_DEF;
							batt_max = BATT_MAX_DEF;
							critical_batt = CRITICAL_BATT_DEF;
						
							login_okay = true;
							break;
						case 2:			
							//BAD_DEVICE_ID:
							bad_id = true;
							break;
						case 3:			
							//TOP_SECRET: hidden function
							ex_mode = ex_mode_offline;
							ex_errorCode = ex_errorCode_Offline;
							login_okay = true;
							break;
						case 0xFF:		
							//FAILED_LOGIN:
							ex_mode = ex_mode_error;
							ex_errorCode = ex_errorCode_LoginFailed;
							login_okay = true;
							break;
						default:		
							break;
					}//switch status
				}else bad_id = false;  //bei Nein muss du eben noch mal eingeben
			}//while login_okay
		}
	#else
		device_id = 1;
	#endif
	
	//=========================================================================
	//Initial setup 
	//=========================================================================
	switch(ex_errorCode)
	{
		case ex_errorCode_NoError:	
			#ifdef ALLOW_LOGIN
				//1. measurement
				MEASURE_PIN_ON
//				he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
				he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
				batt_level = get_batt_level(batt_min, batt_max,r_zero);
				pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
				MEASURE_PIN_OFF
				
				buffer[0] = device_id >> 8;
				buffer[1] = device_id;
				
				if(he_level < 0)	// If negative He level, send error code
				{
					buffer[2] = HE_LEVEL_ERROR>>8;
					buffer[3] = (uint8_t) HE_LEVEL_ERROR;
				}
				else
				{
					buffer[2] = ((uint16_t)(he_level*10))>>8;
					buffer[3] = ((uint16_t)(he_level*10));
				}
				buffer[4] = batt_level;
				buffer[5] = pressure_level >> 8;
				buffer[6] = pressure_level;
				buffer[7] = status_byte;
				
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				sending_cmd = xbee_pack_tx64_frame(STATUS_MSG, buffer, 8, dest_high, dest_low);
				xbee_send(buffer);
			#endif
			//paint screen
			paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
			//set slow transmission; transmit_slow_min indicates whether its in min or hours
			(transmit_slow_min)? 
					set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER)
				:	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);
				
			//set XBEE SLEEP and start waiting to wake it up
			xbee_sleep();
			xbee_sleeping = true;
			set_timeout(0, TIMER_5, RESET_TIMER);
			set_timeout(xbee_sleep_period*60, TIMER_5, USE_TIMER);
			break;
		case ex_errorCode_Offline:	
			//hidden function mode
			paint_offline(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);
			xbee_sleep();
			break;
		default:		
			break;
	}	
	#ifdef ALLOW_DISPLAY_TIMER
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
	#endif
	
	//=========================================================================
	// Main loop 
	//=========================================================================
	while(1) 
	{	
		switch(ex_mode)
		{	
			case ex_mode_main_window:
				break;
			case ex_mode_fill_action:
				break;
			case ex_mode_measure_action:
				break;
			case ex_mode_options_window:
				break;
			case ex_mode_shutdown_action:
				break;
			case ex_mode_diagnotic_action:
				break;
			case ex_mode_offline:
				break;
			case ex_mode_error:
				break;
			case ex_mode_calibration:
				break;
			default:
				ex_mode = ex_mode_main_window;
				break;
		}
			
		//=========================================================================
		// Measure levels and send to the database every "INTERVAL SLOW"
		//=========================================================================
		if((ex_errorCode != ex_errorCode_Offline) && (ex_mode == ex_mode_main_window))
		{
			if(!set_timeout(0, TIMER_1, USE_TIMER)) 
			{
				paint_wait_screen();
				
				MEASURE_PIN_ON
				//he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
				he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
				batt_level = get_batt_level(batt_min, batt_max, r_zero);
				pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
				MEASURE_PIN_OFF
				
				xbee_wake_up();

				buffer[0] = device_id>>8;
				buffer[1] = device_id;
				
				if(he_level < 0)	// If negative He level, send error code
				{
					buffer[2] = HE_LEVEL_ERROR>>8;
					buffer[3] = (uint8_t) HE_LEVEL_ERROR;
				}
				else
				{
					buffer[2] = ((uint16_t)(he_level*10))>>8;
					buffer[3] = ((uint16_t)(he_level*10));
				}
				
				buffer[4] = batt_level;
				buffer[5] = pressure_level >> 8;
				buffer[6] = pressure_level;
				buffer[7] = status_byte;
				
				#ifdef ALLOW_COM
					xbee_send_request(LONG_INTERVAL_MSG, buffer, 8, &dest_high, &dest_low);
				#else
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(LONG_INTERVAL_MSG, buffer, 8, dest_high, dest_low);
					xbee_send(buffer);
				#endif
				
				// Set timer for the next delay
				(transmit_slow_min) ? set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER) :	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);
				
				xbee_sleep();
				paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
			}
		}
		
		// Execute code according to running_mode 
		switch(ex_mode)
		{
		case ex_mode_main_window:			// Main mode, waits for something happened
			// Wait for someone press a key
			switch(keyhit())
			{	
				// Filling procedure				
				case KEY_FILL:	// S1 pressed	
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					if (!very_low_He_level)	
					{	// Filling only if actual level is higher than minimum level defined in options
						if(!update_filling_pos(fill_pos_letters, fill_pos_ranges, temp))
						{
							paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
							break;
						}
						// Copy position bytes from temp to device_pos (each digit is a byte)
						strcpy(device_pos, temp);
						// Clear display and inform about the sending
						LCD_Cls(BGC);
						LCD_Print("Sending data\nduring filling\nprocedure...", 5, 50, 2, 1, 1, FGC, BGC);
							
						// Pack frame (including device id, device position and status)
						uint8_t indx = 0;
						buffer[0] = device_id>>8;
						buffer[1] = device_id;
						indx = devicePos_to_buffer(device_pos, 2, buffer);
						buffer[indx++] = status_byte;
							
						xbee_wake_up();
						xbee_busy = true;
							
						#ifdef ALLOW_COM
							// Send packed message and get an answer
							xbee_send_request(FILLING_BEGIN_MSG, buffer, indx, &dest_high, &dest_low);
						#else
							// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
							sending_cmd = xbee_pack_tx64_frame(FILLING_BEGIN_MSG, buffer, indx, dest_high, dest_low);
							xbee_send(buffer);
						#endif
							
						paint_filling(device_id, device_pos, he_level, total_volume, batt_level, critical_batt,0);
						// Number of measurements 
						fill_meas_counter = (transmit_fast_sec)?
						((((uint16_t) fill_timeout*60))/transmit_fast) +1
						:(fill_timeout/transmit_fast) + 1;
						// Set filling mode
						ex_mode = ex_mode_fill_action;
					}
					else 
					{	// Helium level is too low, stop filling and go back to main mode
						LCD_Dialog("HE LEVEL", "Filling is disabled.\nHe level is too low.", D_FGC, D_BGC);
							
						ex_mode = ex_mode_main_window;
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
					}
				}	break;
				// Force single measurement				
				case KEY_MEASURE:	
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					paint_measure(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);
					ex_mode = ex_mode_measure_action;
				}	break;
				// Force the device to shutdown (as S8)
				case KEY_LEFT_S7:		
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					uint8_t status;	// Pressed "cancel" or "ok"
					entered_options_pw = get_number(&status, PASSWORD);
							
					if(status || (entered_options_pw != options_pw))
					{
						// Cancelled or wrong password, go back to the main mode
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
						ex_mode = ex_mode_main_window;
						break;
					}
					
					if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
					{
						// Confirmed shutting down, set shutdown action
						LCD_Cls(BGC);
						ex_mode = ex_mode_shutdown_action;
					}
					else
					{
						// Canceled, go back to the main mode
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
						ex_mode = ex_mode_main_window;
					}
				}	break;
				// Force the device to shutdown (as S7)
				case KEY_RIGHT_S8:	
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					uint8_t status;	// Pressed "cancel" or "ok"
					entered_options_pw = get_number(&status, PASSWORD);
							
					if(status || (entered_options_pw != options_pw))
					{
						// Cancelled or wrong password, go back to the main mode
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
						ex_mode = ex_mode_main_window;
						break;
					}
					
					if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
					{
						// Confirmed shutting down, set shutdown action
						LCD_Cls(BGC);
						ex_mode = ex_mode_shutdown_action;
					}
					else
					{
						// Cancelled, go back to the main mode
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
						ex_mode = ex_mode_main_window;
					}
				}	break;
				// Force to reconnect to the DB server
				case KEY_DOWN_S9:
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					LCD_Cls(BGC);
					LCD_Print("Connecting\nto the server...", 5, 50, 2, 1, 1, FGC, BGC);
					xbee_wake_up();
					xbee_reconnect(&dest_low, &dest_high);
					xbee_sleep();
					(CHECK_NETWORK_ERROR)? 
						LCD_Dialog("Network", "Couldn't connect\nto network", D_FGC, D_BGC)
						:	LCD_Dialog("Network", "Connection\nsuccessfull", D_FGC, D_BGC);
													
					paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
				}	break;
				// Go to options pages			
				case KEY_BOT_S10:
				{
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					uint8_t status;	// Pressed "cancel" or "ok"
					entered_options_pw = get_number(&status, PASSWORD);
						
					if(status || (entered_options_pw != options_pw))
					{
						// Cancelled or wrong password, go back to the main mode
						paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
						ex_mode = ex_mode_main_window;
						break;
					}
					// Set options mode	
					ex_mode = ex_mode_options_window;
					// Init. options flow control
					active_option = 1; active_value = 0; active_page = 1;
					// Display options page 1
					paint_options(active_page);
					// Show current option values
					paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, quench_time, meas_cycles, fill_timeout);
				}	break;

				default:		
					// Do anything
					ex_mode = ex_mode_main_window;

					break;
			}		// End switch 
			break;	// End case main window
			
/*
		case MODE_FILL_ACTION:			// Filling procedure
			switch(keyhit())
			{
				// Force to stop filling				
				case KEY_TOP_S5:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					if(LCD_Dialog("Filling","Do you really\nwant to stop\nfilling procedure?", D_FGC, D_BGC))
					{
						// Stop filling procedure!
						set_timeout(0, TIMER_4, RESET_TIMER); 		// Clear timer
						LCD_Cls(BGC);
						LCD_Print("Uploading\nto the server...", 5, 50, 2, 1, 1, FGC, BGC);		
						
						// Pack frame (including device ID, levels and status)
						buffer[0] = device_id>>8;
						buffer[1] = device_id;
						
						if(he_level < 0)	// If negative He level, send error code
						{
							buffer[2] = HE_LEVEL_ERROR>>8;
							buffer[3] = (uint8_t) HE_LEVEL_ERROR;
						}
						else {
							buffer[2] = ((uint16_t)(he_level*10))>>8;
							buffer[3] = ((uint16_t)(he_level*10));
						}
						buffer[4] = batt_level;
						buffer[5] = pressure_level >> 8;
						buffer[6] = pressure_level;
						buffer[7] = status_byte;
						
						#ifdef ALLOW_COM
							// Send packed message and get answer from database server
							xbee_send_request(FILLING_END_MSG, buffer, 8, &dest_high, &dest_low);
						#else
							// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
							sending_cmd = xbee_pack_tx64_frame(FILLING_END_MSG, buffer, 8, dest_high, dest_low);
							xbee_send(buffer);
						#endif
						// Regardless communication errors continue anyway
						paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
						ex_mode = ex_mode_main_window;
						xbee_sleep();
						xbee_busy = false;
					}
					else 
					{
						// Cancelled stopping request
						paint_filling(device_id, device_pos, he_level, batt_level,critical_batt, 0);
						//LCD_Print("Waiting...", 2, 20, 2, 1, 1, FGC, BGC);
						sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
						LCD_Print(temp, 5, 100, 1, 1, 1, FGC, BGC);
					}
					break;
				case KEY_UP_S6:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_S7:		
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_RIGHT_S8:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_DOWN_S9:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_BOT_S10:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					fill_meas_counter++;		// More filling needed
					if(transmit_fast_sec) {
						if(fill_meas_counter > ((((uint16_t) fill_timeout*60))/transmit_fast))	 	fill_meas_counter = (((uint16_t) fill_timeout*60))/transmit_fast;
					} else {
						if(fill_meas_counter > (fill_timeout/transmit_fast)) 						fill_meas_counter = fill_timeout/transmit_fast;
					}
					sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
					LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
					break;
				default:				
				{	_Bool filling_end = false;
					
					if(!(meas_progress = set_timeout(0, TIMER_4, USE_TIMER)))
					{	// Time between measurements is passed, new measurement
						LCD_Print("Measuring ...", 2, 20, 2, 1, 1, FGC, BGC);
						(transmit_fast_sec)?
							set_timeout(transmit_fast, TIMER_4, USE_TIMER)
						:	set_timeout(transmit_fast*60, TIMER_4, USE_TIMER);
						
						MEASURE_PIN_ON
						he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 1);
						batt_level = get_batt_level(batt_min, batt_max,r_zero);
						pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
						MEASURE_PIN_OFF
						
						paint_filling(device_id,device_pos,he_level,batt_level,critical_batt,1);
						
						// Pack data	
						buffer[0] = device_id>>8;
						buffer[1] = device_id;
						if(he_level < 0)	// If negative He level, send error code
						{
							buffer[2] = HE_LEVEL_ERROR>>8;
							buffer[3] = (uint8_t) HE_LEVEL_ERROR;
						}
						else
						{
							buffer[2] = ((uint16_t)(he_level*10))>>8;
							buffer[3] = ((uint16_t)(he_level*10));
						}
						buffer[4] = batt_level;
						buffer[5] = pressure_level >> 8;
						buffer[6] = pressure_level;
						buffer[7] = status_byte;
						
						// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
						sending_cmd = xbee_pack_tx64_frame(FILLING_MSG, buffer, 8, dest_high, dest_low);
						xbee_send(buffer);
						
						fill_meas_counter--;
						_delay_ms(150);
						
						//if autofilling and no more signal - end
						if(!auto_fill_pin_on() && auto_fill_started)
						{
							//FE
							filling_end = true;
							//set auto fill pin low
							STOP_AUTO_FILL
							auto_fill_started = false;
						}
						//if normal filling and counter=0 - end
						if(!fill_meas_counter && !auto_fill_started)
						{
							//FE
							filling_end = true;
						}
						//he_level twice lower then min, end/disable filling
						if((he_level < he_min) && !auto_fill_started)
						{
							if(last_he_level < he_min) 
							{
								STOP_AUTO_FILL
								timed_dialog("HE LEVEL", "Filling is disabled.\nHe Level is too low.", 10, D_FGC, D_BGC); 
								very_low_He_level = true;
								
								//FE
								filling_end = true;
								auto_fill_started = false;
								auto_fill_enabled = false;		//disable autofill regardless any signals
							}
						}
						last_he_level = he_level;
						if(!filling_end)
						{
							//set timer till next measurement
							LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
							clear_progress_bar(5, 105);
							
							sprintf(temp, FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
							LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
						}
					}//if(timer ready) 
					else if(!auto_fill_pin_on() && auto_fill_started)
					{
						//FE
						filling_end = true;
						//set auto fill pin low
						STOP_AUTO_FILL
						auto_fill_started = false;
					}
					else
					{
						(transmit_fast_sec)? 	draw_current_wait_time(70, 20, transmit_fast, meas_progress, FGC)
						:						draw_current_wait_time(70, 20, transmit_fast*60, meas_progress, FGC);
					}
						
					if(filling_end)
					{
						//send end ..
						buffer[0] = device_id>>8;
						buffer[1] = device_id;
						
						if(he_level < 0)	// If negative He level, send error code
						{
							buffer[2] = HE_LEVEL_ERROR>>8;
							buffer[3] = (uint8_t) HE_LEVEL_ERROR;
						}
						else {
							buffer[2] = ((uint16_t)(he_level*10))>>8;
							buffer[3] = ((uint16_t)(he_level*10));
						}
						
						buffer[4] = batt_level;
						buffer[5] = pressure_level >> 8;
						buffer[6] = pressure_level;
						buffer[7] = status_byte;
						
						#ifdef ALLOW_COM
							xbee_send_request(FILLING_END_MSG, buffer, 8, &dest_high, &dest_low);
						#else
							// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
							sending_cmd = xbee_pack_tx64_frame(FILLING_END_MSG, buffer, 8, dest_high, dest_low);
							xbee_send(buffer);
						#endif
						paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
						ex_mode = ex_mode_main_window;
						
						xbee_sleep();
						xbee_busy = false;
					}
					}break;	// End of default
					
			
			}
			break;
			
*/
		case ex_mode_measure_action:		// Measurement ask by the user or auto-mode
			// Switch on current supply board
			MEASURE_PIN_ON				
			// Get levels
//			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
			batt_level = get_batt_level(batt_min, batt_max,r_zero);
			pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
			// Switch off current supply board
			MEASURE_PIN_OFF
			// Wake up XBee module
			xbee_wake_up();
			// Pack data in frame
			buffer[0] = device_id>>8;
			buffer[1] = device_id;
			if(he_level < 0)	// If negative He level, send error code instead of a wrong value
			{
				buffer[2] = HE_LEVEL_ERROR>>8;
				buffer[3] = (uint8_t) HE_LEVEL_ERROR;
			}
			else
			{
				buffer[2] = ((uint16_t)(he_level*10))>>8;
				buffer[3] = ((uint16_t)(he_level*10));
			}
			buffer[4] = batt_level;
			buffer[5] = pressure_level >> 8;
			buffer[6] = pressure_level;
			buffer[7] = status_byte;
			#ifdef ALLOW_COM
				// Send to the database server and get answer
				xbee_send_request(LONG_INTERVAL_MSG, buffer, 8, &dest_high, &dest_low);
			#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				sending_cmd = xbee_pack_tx64_frame(LONG_INTERVAL_MSG, buffer, 8, dest_high, dest_low);
				xbee_send(buffer);
			#endif
			// Sleep XBee module
			xbee_sleep();
																		
			if(force_measurement)
			{
				forced_measurement_done = true;		// Clear forced measurement flag
			}
			else if(ex_errorCode == ex_errorCode_Offline)
			{
				paint_offline(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);		// Update display
				ex_mode = ex_mode_offline;													// Set OFFLINE_errCode mode
			}
			else
			{
				paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);	// Update display
				ex_mode = ex_mode_main_window;												// Set MAIN mode
			}
			break;

		case ex_mode_options_window:
			switch(active_page)
			{
				case 1: // Page 1
					switch(keyhit())
					{
						case KEY_DOWN_S9:	// Button down
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 0:	
									active_option++;
									if(active_option == 6) {active_option = 1;}		// Init counter
									paint_current_opt_page1(active_option, KEY_DOWN_S9);
									break;
								case 1:	
									//decrease transmission period
									transmit_slow--;
									if(!transmit_slow) 
									{
										if(!transmit_slow_min)
										{
											transmit_slow = 60;
											transmit_slow_min = true;
										}
										else
										{
											transmit_slow = 1;
										}
									}
									(transmit_slow_min) ? 
									draw_int(transmit_slow, 85, 20, "min", ERR) 
									: draw_int(transmit_slow, 85, 20, "h", ERR);
																		
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										transmit_slow--;
										if(!transmit_slow) 
										{
											if(!transmit_slow_min)
											{
												transmit_slow = 60;
												transmit_slow_min = true;
											}
											else
											{
												transmit_slow = 1;
											}
										}
										(transmit_slow_min) ?
										draw_int(transmit_slow, 85, 20, "min", ERR)
										: draw_int(transmit_slow, 85, 20, "h", ERR);
										_delay_ms(50);
									}
																				
									transmit_slow_changed = true;
									break;
								case 2: 
									//decrease transmission period while filling
									transmit_fast--;
									if(!transmit_fast) 
									{
										if(!transmit_fast_sec)
										{
											transmit_fast = 60;
											transmit_fast_sec = true;
										}
										else
										{
										transmit_fast = 1;
										}
									}
									if(transmit_fast < TRANSMIT_FAST_MIN)
										transmit_fast = TRANSMIT_FAST_MAX;
										
										(transmit_fast_sec) ? 
										draw_int(transmit_fast, 85, 40, "sec", ERR) 
										: draw_int(transmit_fast, 85, 40, "min", ERR);
																				
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											transmit_fast--;
											if(!transmit_fast) 
											{
												if(!transmit_fast_sec)
												{
													transmit_fast = 60;
													transmit_fast_sec = true;
												}
												else
												{
													transmit_fast = 1;
												}
											}
											if(transmit_fast < TRANSMIT_FAST_MIN)
												transmit_fast = TRANSMIT_FAST_MAX;
											
											(transmit_fast_sec) ?
											draw_int(transmit_fast, 85, 40, "sec", ERR)
											: draw_int(transmit_fast, 85, 40, "min", ERR);
											_delay_ms(50);
										}
									break;
								case 3:	
									//decrease quench time
									quench_time-=1;
									if(quench_time < QUENCH_TIME_MIN) quench_time = QUENCH_TIME_MAX;
									draw_double(quench_time, 76, 60, 0,"s", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										quench_time-=1;
										if(quench_time < QUENCH_TIME_MIN) quench_time = QUENCH_TIME_MAX;
										draw_double(quench_time, 76, 60, 0, "s", ERR);
										_delay_ms(50);
									}
									break;
								case 4:	
									//decrease t n
									meas_cycles--;
									if(meas_cycles < MEASUREMENT_CYCLES_MIN) meas_cycles = MEASUREMENT_CYCLES_MAX;
									draw_int(meas_cycles, 85, 80, "c", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										meas_cycles--;
										if(meas_cycles < MEASUREMENT_CYCLES_MIN) meas_cycles = MEASUREMENT_CYCLES_MAX;
										draw_int(meas_cycles, 85, 80, "c", ERR);
										_delay_ms(50);
									}
									break;
								case 5:	
									//decrease fill_timeout
									fill_timeout--;
									if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
									draw_int(fill_timeout, 85, 100, "min", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										fill_timeout--;
										if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
										draw_int(fill_timeout, 85, 100, "min", ERR);
										_delay_ms(50);
									}
									break;
								}
								options_changed = true;
								break;
																
						case KEY_BOT_S10:
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							active_page = 2;
							paint_options(active_page);
							active_option = 1;
							active_value = 0;
							paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
							break;
																	
						case KEY_LEFT_S7:		
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 1:	
									active_value = 0;
									LCD_Print(STRING_TRANSMIT_SLOW,			2, 20, 2, 1,1, ERR, BGC);
									(transmit_slow_min) ? 
										draw_int(transmit_slow, 85, 20, "min", FGC) 
									: 	draw_int(transmit_slow, 85, 20, "h", FGC);
									break;
								case 2: 
									active_value = 0;
									LCD_Print(STRING_TRANSMIT_FAST,			2, 40, 2, 1,1, ERR, BGC);
									draw_int(transmit_fast, 85, 40, "min", FGC);
									break;
								case 3: 
									active_value = 0;
									LCD_Print(STRING_HEAT_TIME,				2, 60, 2, 1,1, ERR, BGC);
									draw_double(quench_time, 76, 60, 0, "s", FGC);
									break;
								case 4: 
									active_value = 0;
									LCD_Print(STRING_MEASUREMENT_CYCLES,	2, 80, 2, 1,1, ERR, BGC);
									draw_int(meas_cycles, 85, 80, "c", FGC);
									break;
								case 5: 
									active_value = 0;
									LCD_Print(STRING_FILLING_TIMEOUT,		2, 100, 2, 1,1, ERR, BGC);
									draw_int(fill_timeout, 85, 100, "min", FGC);
									break;
							}
							break;
														
						case KEY_RIGHT_S8:	
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							if (!active_value) 
							{
								switch(active_option) 
								{
									case 1:	
										active_value = 1;
										LCD_Print(STRING_TRANSMIT_SLOW,			2, 20, 2, 1,1, FGC, BGC);
										(transmit_slow_min) ? 
											draw_int(transmit_slow, 85, 20, "min", ERR) 
										: 	draw_int(transmit_slow, 85, 20, "h", ERR);
										break;
									case 2: 
										active_value = 2;
										LCD_Print(STRING_TRANSMIT_FAST,			2, 40, 2, 1,1, FGC, BGC);
										draw_int(transmit_fast, 85, 40, "min", ERR);
										break;
									case 3: 
										active_value = 3;
										LCD_Print(STRING_HEAT_TIME,				2, 60, 2, 1,1, FGC, BGC);
										draw_double(quench_time, 76, 60, 0, "s", ERR);
										break;
									case 4: 
										active_value = 4;
										LCD_Print(STRING_MEASUREMENT_CYCLES,	2, 80, 2, 1,1, FGC, BGC);
										draw_int(meas_cycles, 85, 80, "c", ERR);
										break;
									case 5: 
										active_value = 5;
										LCD_Print(STRING_FILLING_TIMEOUT,		2, 100, 2, 1,1, FGC, BGC);
										draw_int(fill_timeout, 85, 100, "min", ERR);
										break;
								}
							}
							break;
																	
						case KEY_UP_S6:	
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 0: 
									active_option--;
									if(!active_option) active_option = 5;	
									
									paint_current_opt_page1(active_option, KEY_UP_S6);
									break;
								case 1:	
									//increase transmission period
									transmit_slow++;
									if((transmit_slow > TRANSMIT_SLOW_MAX) && (!transmit_slow_min)) transmit_slow = TRANSMIT_SLOW_MIN;
									
									if((transmit_slow > 60) && (transmit_slow_min))
									{
										transmit_slow=1;
										transmit_slow_min = false;
										LCD_Print("     ", 85, 20, 2, 1,1, ERR, BGC);
									}
									(transmit_slow_min) ? 
										draw_int(transmit_slow, 85, 20, "min", ERR) 
									: 	draw_int(transmit_slow, 85, 20, "h", ERR);
									
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										transmit_slow++;
										if((transmit_slow > TRANSMIT_SLOW_MAX) && (!transmit_slow_min)) transmit_slow = TRANSMIT_SLOW_MIN;
										
										if((transmit_slow > 60) && (transmit_slow_min))
										{
											transmit_slow=1;
											transmit_slow_min = false;
											LCD_Print("     ", 85, 20, 2, 1,1, ERR, BGC);
										}
										(transmit_slow_min) ? 
											draw_int(transmit_slow, 85, 20, "min", ERR) 
										: 	draw_int(transmit_slow, 85, 20, "h", ERR);
										_delay_ms(50);
									}//while(keyhit()==KEY_UP_S6)
									
									transmit_slow_changed = true;
									break;
								case 2: 
									//increase transmission period while filling
									transmit_fast++;
									if((transmit_fast > TRANSMIT_FAST_MAX) && (!transmit_fast_sec)) transmit_fast = TRANSMIT_FAST_MIN;
									
									if((transmit_fast>60) && (transmit_fast_sec)) 
									{
										transmit_fast=1;
										transmit_fast_sec = false;
										LCD_Print("     ", 85, 40, 2, 1,1, ERR, BGC);
									}
									(transmit_fast_sec) ? 
										draw_int(transmit_fast, 85, 40, "sec", ERR) 
									: 	draw_int(transmit_fast, 85, 40, "min", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										transmit_fast++;
										if((transmit_fast > TRANSMIT_FAST_MAX) && (!transmit_fast_sec)) transmit_fast = TRANSMIT_FAST_MIN;
										
										if((transmit_fast>60) && (transmit_fast_sec)) 
										{
											transmit_fast=1;
											transmit_fast_sec = false;
											LCD_Print("     ", 85, 40, 2, 1,1, ERR, BGC);
										}
										(transmit_fast_sec) ? 
											draw_int(transmit_fast, 85, 40, "sec", ERR) 
										: 	draw_int(transmit_fast, 85, 40, "min", ERR);
										_delay_ms(50);
									}
									break;
								case 3:	
									//increase quench time
									quench_time+=1;
									if(quench_time > QUENCH_TIME_MAX) quench_time = QUENCH_TIME_MIN;
									draw_double(quench_time, 76, 60, 0, "s", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										quench_time+=1;
										if(quench_time > QUENCH_TIME_MAX) quench_time = QUENCH_TIME_MIN;
										draw_double(quench_time, 76, 60, 0, "s", ERR);
										_delay_ms(50);
									}
									break;
								case 4:	
									//increase t n
									meas_cycles++;
									if(meas_cycles > MEASUREMENT_CYCLES_MAX) meas_cycles = MEASUREMENT_CYCLES_MIN;
									draw_int(meas_cycles, 85, 80, "c", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										meas_cycles++;
										if(meas_cycles > MEASUREMENT_CYCLES_MAX) meas_cycles = MEASUREMENT_CYCLES_MIN;
										draw_int(meas_cycles, 85, 80, "c", ERR);
										_delay_ms(50);
									}
									break;
								case 5:	
									//increase fill_timeout
									fill_timeout++;
									if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
									draw_int(fill_timeout, 85, 100, "min", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										fill_timeout++;
										if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
										draw_int(fill_timeout, 85, 100, "min", ERR);
										_delay_ms(50);
									}
									break;
							}
							options_changed = true;
							break;
										
						case KEY_TOP_S5:	
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							if(ex_errorCode != ex_errorCode_Offline)
							{
								if(options_changed)
								{
									xbee_wake_up();
									buffer[0] = device_id>>8;
									buffer[1] = device_id;
									
									if(transmit_slow_min) 
									{
										buffer[2] = transmit_slow>>8;
										buffer[3] = transmit_slow;
									}
									else
									{
										buffer[2] = (transmit_slow*60)>>8;
										buffer[3] = transmit_slow*60;
									}
									if(transmit_fast_sec) 
									{
										buffer[4] = transmit_fast>>8;
										buffer[5] = transmit_fast;
									}
									else
									{
										buffer[4] = (transmit_fast*60)>>8;
										buffer[5] = transmit_fast*60;
									}
									buffer[6] = ((uint16_t)(res_min*10))>>8;
									buffer[7] = res_min*10;
									buffer[8] = ((uint16_t)(res_max*10))>>8;
									buffer[9] = res_max*10;

									buffer[10] = ((uint16_t)(quench_time*1000))>>8;
									buffer[11] = (quench_time*1000);
									buffer[12] = ((uint16_t)quench_current)>>8;
									buffer[13] = quench_current;
									buffer[14] = ((uint16_t)(wait_time*1000))>>8;
									buffer[15] = (wait_time*1000);
									buffer[16] = ((uint16_t)meas_current)>>8;
									buffer[17] = meas_current;

									buffer[18] = meas_cycles;
									buffer[19] = fill_timeout;
									buffer[20] = ((uint16_t)(span*10))>>8;
									buffer[21] = span*10;
									buffer[22] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
									buffer[23] = (ZERO_NULLPUNKT+zero)*10;
									buffer[24] = ((uint16_t)(batt_min*10))>>8;
									buffer[25] = batt_min*10;
									buffer[26] = ((uint16_t)(batt_max*10))>>8;
									buffer[27] = batt_max*10;
									buffer[28] = critical_batt;
									buffer[29] = status_byte;
									
									LCD_Cls(BGC);
									LCD_Print("Saving settings...", 5, 50, 2, 1, 1, FGC, BGC);
									
									#ifdef ALLOW_COM
										xbee_send_request(OPTIONS_CHANGED_MSG, buffer, 30, &dest_high, &dest_low);
									#else
										// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
										sending_cmd = xbee_pack_tx64_frame(OPTIONS_CHANGED_MSG, buffer, 29, dest_high, dest_low);
										xbee_send(buffer);
									#endif
									xbee_sleep();
									
									#ifdef ALLOW_EEPROM_SAVING
										eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
										eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
										
										eeprom_write_word(&ee_transmit_slow, transmit_slow);
										eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
										eeprom_write_word(&ee_transmit_fast, transmit_fast);
										eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
										eeprom_write_word(&ee_quench_time, (uint16_t) (quench_time*1000));
										eeprom_write_word(&ee_quench_current, (uint16_t)quench_current);
										eeprom_write_word(&ee_wait_time, (uint16_t) (wait_time*1000));
										eeprom_write_word(&ee_meas_current, (uint16_t)meas_current);
										
										eeprom_write_byte(&ee_meas_cycles, meas_cycles);
										eeprom_write_byte(&ee_fill_timeout, fill_timeout);
										eeprom_write_byte(&ee_he_min, he_min);
										eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
										eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
										eeprom_write_word(&ee_span, (uint16_t) (span*10));
										eeprom_write_word(&ee_zero, (uint16_t) ((zero+ZERO_NULLPUNKT)*10));
										eeprom_write_byte(&ee_enable_pressure, enable_pressure);
										eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
										eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
										eeprom_write_byte(&ee_critical_batt, critical_batt);
									#endif
								}
								
								paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
								ex_mode = ex_mode_main_window;
								options_changed = false;
							}
							else
							{
								ex_mode = ex_mode_offline;
								paint_offline(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);
							}
							break;
						default:			
							break;
					}
						break;
				case 2: // Page 2
					switch(keyhit())
					{
						case KEY_DOWN_S9:	
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
							switch(active_value) 
							{
								case 0: 
									active_option++;
									if(active_option == 6) {active_option = 1;}
									paint_current_opt_page2(active_option, KEY_DOWN_S9);
									break;
								case 1:	
									//decrease r min
									res_min-=0.1;
									if(res_min < RES_MIN_MIN) res_min = RES_MIN_MAX;
									draw_double(res_min, 76, 20, 1, "o", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										res_min-=0.1;
										if(res_min < RES_MIN_MIN) res_min = RES_MIN_MAX;
										draw_double(res_min, 76, 20, 1, "o", ERR);
										_delay_ms(50);
									}
									break;
								case 2:	
									//decrease r max
									res_max-=0.1;
									if(res_max < RES_MAX_MIN) res_max = RES_MAX_MAX;
									draw_double(res_max, 76, 40, 1, "o", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										res_max--;
										if(res_max < RES_MAX_MIN) res_max = RES_MAX_MAX;
										draw_double(res_max, 76, 40, 1, "o", ERR);
										_delay_ms(50);
									}
									break;
								case 3:	
									// Decrease battery min
									batt_min-=0.1;
									if(batt_min < BATT_MIN_MIN) batt_min = BATT_MIN_MAX;
									draw_double(batt_min, 76, 60, 1, "V", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										batt_min-=0.1;
										if(batt_min < BATT_MIN_MIN) batt_min = BATT_MIN_MAX;
										draw_double(batt_min, 76, 60, 1, "V", ERR);
										_delay_ms(50);
									}
									break;
								case 4:	
									// Decrease battery max
									batt_max-=0.1;
									if(batt_max < BATT_MAX_MIN) batt_max = BATT_MAX_MAX;
									draw_double(batt_max, 76, 80, 1, "V", ERR);
									_delay_ms(300); 
									while(keyhit()==KEY_DOWN_S9)
									{
										batt_max-=0.1;
										if(batt_max < BATT_MAX_MIN) batt_max = BATT_MAX_MAX;
										draw_double(batt_max, 76, 80, 1, "V", ERR);
										_delay_ms(50);
									}
									break;
								case 5:	
									//decrease critical batt
									critical_batt--;
									if(critical_batt < 1) critical_batt = 100;
									draw_int(critical_batt, 85, 100, "%", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										critical_batt--;
										if(critical_batt < 1) critical_batt = 100;
										draw_int(critical_batt, 85, 100, "%", ERR);
										_delay_ms(50);
									}
									break;
								default:
									break;
							}
							options_changed = true;
							break;
									
						case KEY_BOT_S10:	
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
										active_page = 3;
										paint_options(active_page);
										active_option = 1;
										active_value = 0;
										paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout, wait_time);
										break;
										
						case KEY_LEFT_S7:		
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
										switch(active_value) 
										{
											case 1:	
													active_value=0;
													LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, ERR, BGC);
													draw_double(res_min, 76, 20, 1, "o", FGC);
													break;
											case 2: 
													active_value=0;
													LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, ERR, BGC);
													draw_double(res_max, 76, 40, 1, "o", FGC);
													break;
											case 3: 
													active_value=0;
													LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, ERR, BGC);
													draw_double(batt_min, 76, 60, 1, "V", FGC);
													break;
											case 4: 
													active_value=0;
													LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, ERR, BGC);
													draw_double(batt_max, 76, 80, 1, "V", FGC);
													break;
											case 5: 
													active_value=0;
													LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, ERR, BGC);
													draw_int(critical_batt, 85, 100, "%", FGC);
													break;
										}
										break;
										
						case KEY_RIGHT_S8:	
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
										if (!active_value) {
											switch(active_option) 
											{
												case 1:	
														active_value=1;
														LCD_Print(STRING_RES_MIN, 2, 20, 2, 1,1, FGC, BGC);
														draw_double(res_min, 76, 20, 1, "o", ERR);
														break;
												case 2: 
														active_value=2;
														LCD_Print(STRING_RES_MAX, 2, 40, 2, 1,1, FGC, BGC);
														draw_double(res_max, 76, 40, 1, "o", ERR);
														break;
												case 3: 
														active_value=3;
														LCD_Print(STRING_BATTMIN, 2, 60, 2, 1,1, FGC, BGC);
														draw_double(batt_min, 76, 60, 1, "V", ERR);
														break;
												case 4: 
														active_value=4;
														LCD_Print(STRING_BATTMAX, 2, 80, 2, 1,1, FGC, BGC);
														draw_double(batt_max, 76, 80, 1, "V", ERR);
														break;
												case 5: 
														active_value=5;
														LCD_Print(STRING_CRITVOLT, 2, 100, 2, 1,1, FGC, BGC);
														draw_int(critical_batt, 85, 100, "%", ERR);
														break;
											}
										}
										break;
										
						case KEY_UP_S6:	
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
										switch(active_value) 
										{
											case 0: 
													active_option--;
													if(!active_option) active_option = 5;	
													
													paint_current_opt_page2(active_option, KEY_UP_S6);
													break;
											case 1:	
													//increase r min
													res_min+=0.1;
													if(res_min > RES_MIN_MAX) res_min = RES_MIN_MIN;
													draw_double(res_min, 76, 20, 1, "o", ERR);
													_delay_ms(300);
													while(keyhit()==KEY_UP_S6)
													{
														res_min+=0.1;
														if(res_min > RES_MIN_MAX) res_min = RES_MIN_MIN;
														draw_double(res_min, 76, 20, 1, "o", ERR);
														_delay_ms(50);
													}
													break;
											case 2:	
													//increase r max
													res_max+=0.1;
													if(res_max > RES_MAX_MAX) res_max = RES_MAX_MIN;
													draw_double(res_max, 76, 40, 1, "o", ERR);
													_delay_ms(300);
													while(keyhit()==KEY_UP_S6)
													{
														res_max++;
														if(res_max > RES_MAX_MAX) res_max = RES_MAX_MIN;
														draw_double(res_max, 76, 40, 1, "o", ERR);
														_delay_ms(50);
													}
													break;
											case 3:	
													//increase batt min
													batt_min+=0.1;
													if(batt_min > BATT_MIN_MAX) batt_min = BATT_MIN_MIN;
													draw_double(batt_min, 76, 60, 1, "V", ERR);
													_delay_ms(300);
													while(keyhit()==KEY_UP_S6)
													{
														batt_min+=0.1;
														if(batt_min > BATT_MIN_MAX) batt_min = BATT_MIN_MIN;
														draw_double(batt_min, 76, 60, 1, "V", ERR);
														_delay_ms(50);
													}
													break;
											case 4:	
													//increase batt max
													batt_max+=0.1;
													if(batt_max > BATT_MAX_MAX) batt_max = BATT_MAX_MIN;
													draw_double(batt_max, 76, 80, 1, "V", ERR);
													_delay_ms(300);
													while(keyhit()==KEY_UP_S6)
													{
														batt_max+=0.1;
														if(batt_max > BATT_MAX_MAX) batt_max = BATT_MAX_MIN;
														draw_double(batt_max, 76, 80, 1, "V", ERR);
														_delay_ms(50);
													}
													break;
											case 5:	
													//increase critical batt
													critical_batt++;
													if(critical_batt > 99) critical_batt = 1;
													draw_int(critical_batt, 85, 100, "%", ERR);
													_delay_ms(300);
													while(keyhit()==KEY_UP_S6)
													{
														critical_batt++;
														if(critical_batt > 99) critical_batt = 1;
														draw_int(critical_batt, 85, 100, "%", ERR);
														_delay_ms(50);
													}
													break;	
											default:
													break;
										}
										options_changed = true;
										break;
										
						case KEY_TOP_S5:	
										
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
										active_page = 1;
										paint_options(active_page);
										active_option = 1;
										active_value = 0;
										paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, quench_time, meas_cycles, fill_timeout);
										break;
						default:			
										break;
					}
					break;												
				
				case 3: // Page 3 - Autofill
					switch(keyhit())
					{
						case KEY_DOWN_S9:	
										
						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}
						switch(active_value)
						{
							case 0:
									active_option++;
									if(active_option == 6) active_option = 1;
													
									paint_current_opt_page3(active_option, KEY_DOWN_S9);
									break;
							case 1:
									//pos verändern (wird sofort mit down (->) ausgeführt)
									break;
							case 2:
									auto_fill_enabled = (auto_fill_enabled)?  false : true;
									(auto_fill_enabled)?
										LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
									:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
									break;
							case 3:
									//decrease autofill value
									he_min--;
									if(he_min < MIN_AUTO_FILL_HE) he_min = MAX_AUTO_FILL_HE;
									draw_int(he_min, 85, 60, "%", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										he_min--;
										if(he_min < MIN_AUTO_FILL_HE) he_min = MAX_AUTO_FILL_HE;
										draw_int(he_min, 85, 60, "%", ERR);
										_delay_ms(50);
									}
									break;
							case 4:
									fill_timeout--;
									if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
									draw_int(fill_timeout, 85, 80, "min", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										fill_timeout--;
										if(fill_timeout < MIN_FILLING_TIMEOUT) fill_timeout = MAX_FILLING_TIMEOUT;
										draw_int(fill_timeout, 85, 80, "min", ERR);
										_delay_ms(50);
									}
									break;
							case 5:
									//decrease waiting time
									wait_time-=1;
									if(wait_time < WAIT_TIME_MIN) wait_time = WAIT_TIME_MAX;
									draw_double(wait_time, 85, 100, 0,"s", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_DOWN_S9)
									{
										wait_time-=1;
										if(wait_time < WAIT_TIME_MIN) wait_time = WAIT_TIME_MAX;
										draw_double(wait_time, 85, 100, 0, "s", ERR);
										_delay_ms(50);
									}
									break;
						}
						break;

						case KEY_BOT_S10:	

						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}
						active_page = 4;
						paint_options(active_page);
						active_option = 1;
						active_value = 0;
										
						paint_opt_values_page4(span, zero, enable_pressure, quench_current, meas_current);
						break;
										
						case KEY_LEFT_S7:		
										
						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}
	
						switch(active_value) 
						{
							case 1:	
									active_value=0;
									LCD_Print(STRING_POS,					2, 20, 2, 1,1, ERR, BGC);
									LCD_Print(device_pos, 85, 20, 2, 1,1, FGC, BGC);
									break;
							case 2: 
									active_value=0;
									LCD_Print(STRING_AUTOFILL,				2, 40, 2, 1,1, ERR, BGC);
													
									(auto_fill_enabled)?
										LCD_Print("on ", 85, 40, 2, 1,1, FGC, BGC)
									:	LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
									break;
							case 3:
									active_value=0;
									LCD_Print(STRING_HE_MIN_LVL,			2, 60, 2, 1,1, ERR, BGC);
									draw_int(he_min, 85, 60, "%", FGC);
									break;
							case 4:
									active_value=0;
									LCD_Print(STRING_FILLING_TIMEOUT,		2, 80, 2, 1,1, ERR, BGC);
									draw_int(fill_timeout, 85, 80, "min", FGC);
									break;
							case 5:
									active_value = 0;
									LCD_Print(STRING_WAIT_TIME,	2, 100, 2, 1, 1, ERR, BGC);
									draw_double(wait_time, 85, 100, 0, "s", FGC);
									break;
						}
						break;
										
						case KEY_RIGHT_S8:	
										
						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}
	
						if(!active_value)
						{
							switch(active_option) 
							{
								case 1:	
										//position bytes are in temp
										if(update_filling_pos(fill_pos_letters, fill_pos_ranges, temp)) strcpy(device_pos, temp);
										active_value=0;
														
										paint_options(active_page);
										paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout, wait_time);
										break;
								case 2: 
										active_value=2;
										LCD_Print(STRING_AUTOFILL, 2, 40, 2, 1,1, FGC, BGC);
														
										(auto_fill_enabled)?
											LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
										:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
										break;
								case 3:
										active_value=3;
										LCD_Print(STRING_HE_MIN_LVL, 2, 60, 2, 1,1, FGC, BGC);
										draw_int(he_min, 85, 60, "%", ERR);
										break;
								case 4:
										active_value=4;
										LCD_Print(STRING_FILLING_TIMEOUT, 2, 80, 2, 1,1, FGC, BGC);
										draw_int(fill_timeout, 85, 80, "min", ERR);
										break;
								case 5:
										active_value = 5;
										LCD_Print(STRING_WAIT_TIME, 2, 100, 2, 1,1, FGC, BGC);
										draw_double(wait_time, 85, 100, 0, "s", ERR);
										break;
							}
						}
						break;
										
						case KEY_UP_S6:	
										
						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}

						switch(active_value)
						{
							case 0:
									active_option--;
									if(!active_option) active_option = 5;
									paint_current_opt_page3(active_option, KEY_UP_S6);
									break;
							case 1:
									//pos verändern
									break;
							case 2:
									auto_fill_enabled = (auto_fill_enabled)?  false : true;
									(auto_fill_enabled)?
										LCD_Print("on ", 85, 40, 2, 1,1, ERR, BGC)
									:	LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
									break;
							case 3:
									//increase autofill value
									he_min++;
									if(he_min > MAX_AUTO_FILL_HE) he_min = MIN_AUTO_FILL_HE;
									draw_int(he_min, 85, 60, "%", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										he_min++;
										if(he_min > MAX_AUTO_FILL_HE) he_min = MIN_AUTO_FILL_HE;
										draw_int(he_min, 85, 60, "%", ERR);
										_delay_ms(50);
									}
									break;
							case 4:
									fill_timeout++;
									if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
									draw_int(fill_timeout, 85, 80, "min", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										fill_timeout++;
										if(fill_timeout > MAX_FILLING_TIMEOUT) fill_timeout = MIN_FILLING_TIMEOUT;
										draw_int(fill_timeout, 85, 80, "min", ERR);
										_delay_ms(50);
									}
									break;
							case 5:
									//increase wait time
									wait_time+=1;
									if(wait_time > WAIT_TIME_MAX) wait_time = WAIT_TIME_MIN;
									draw_double(wait_time, 85, 100, 0, "s", ERR);
									_delay_ms(300);
									while(keyhit()==KEY_UP_S6)
									{
										wait_time+=1;
										if(wait_time > WAIT_TIME_MAX) wait_time = WAIT_TIME_MIN;
										draw_double(wait_time, 85, 100, 0, "s", ERR);
										_delay_ms(50);
									}
									break;
						}
						options_changed = true;
						break;
										
						case KEY_TOP_S5:	
										
						if (!display_on()) 
						// Display was off, turn it on
						{
							#ifdef ALLOW_DOUBLE_CLICK 
								;		// Double click allowed, execute following code
							#else
								break;	// Double click not allowed, break here and wait for a pressed key 
							#endif
						}
											active_page = 2;
											paint_options(active_page);
											active_option = 1;
											active_value = 0;
										
											paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
											break;	
						}
						break;

				case 4: // Page 4 - Pressure
					switch(keyhit())
					{
						case KEY_DOWN_S9:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
					
							switch(active_value)
							{
								case 0:
										active_option++;
										if(active_option == 6) active_option = 1;
										paint_current_opt_page4(active_option, KEY_DOWN_S9);
										break;
								case 1:	
										//decrease span
										span-=0.1;
										if(span < MIN_SPAN) span = MIN_SPAN;
										draw_double(span, 76, 20, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											span-=1;
											if(span < MIN_SPAN) span = MIN_SPAN;
											draw_double(span, 76, 20, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 2:	
										//decrease zero
										zero-=0.1;
										if(zero < MIN_ZERO) zero = MIN_ZERO;
										draw_double(zero, 76, 40, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											zero-=1;
											if(zero < MIN_ZERO) zero = MIN_ZERO;
											draw_double(zero, 76, 40, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 3:	
										//decrease batt min
										enable_pressure =(enable_pressure)? false : true;
										(enable_pressure)?
											LCD_Print("on ", 85, 60, 2, 1,1, ERR, BGC)
										:	LCD_Print("off", 85, 60, 2, 1,1, ERR, BGC);
										break;
								case 4:
										//decrease quench current
										quench_current-=1;
										if(quench_current < QUENCH_CURRENT_MIN) quench_current = QUENCH_CURRENT_MAX;
										draw_double(quench_current, 85, 80, 0,"mA", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											quench_current-=1;
											if(quench_current < QUENCH_CURRENT_MIN) quench_current = QUENCH_CURRENT_MAX;
											draw_double(quench_current, 85, 80, 0, "mA", ERR);
											_delay_ms(50);
										}
										break;
								case 5:
										//decrease measurement current
										meas_current-=1;
										if(meas_current < MEAS_CURRENT_MIN) meas_current = MEAS_CURRENT_MAX;
										draw_double(meas_current, 85, 100, 0,"mA", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											meas_current-=1;
											if(meas_current < MEAS_CURRENT_MIN) meas_current = MEAS_CURRENT_MAX;
											draw_double(meas_current, 85, 100, 0, "mA", ERR);
											_delay_ms(50);
										}
										break;

							}
							break;

						case KEY_BOT_S10:
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}

							active_page = 5;
							paint_options(active_page);
							active_option = 1;
							active_value = 0;
										
							paint_opt_values_page5(r_span, r_zero, total_volume);
							break;

						case KEY_LEFT_S7:	
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 1:	
										active_value=0;
										LCD_Print(STRING_SPAN, 2, 20, 2, 1,1, ERR, BGC);
										draw_double(span, 76, 20, 1, "", FGC);
										break;
								case 2: 
										active_value=0;
										LCD_Print(STRING_ZERO, 2, 40, 2, 1,1, ERR, BGC);
										draw_double(zero, 76, 40, 1, "", FGC);
										break;
								case 3: 
										active_value=0;
										(enable_pressure)?
											LCD_Print("on ", 85, 60, 2, 1,1, FGC, BGC)
										:	LCD_Print("off", 85, 60, 2, 1,1, FGC, BGC);
										break;
								case 4:
										active_value = 0;
										LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1, 1, ERR, BGC);
										draw_double(quench_current, 85, 80, 0, "mA", FGC);
										break;
								case 5:
										active_value = 0;
										LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1, 1, ERR, BGC);
										draw_double(meas_current, 85, 100, 0, "mA", FGC);
										break;
							}
							break;

						case KEY_RIGHT_S8:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_option) 
							{
								case 1:	
										active_value=1;
										LCD_Print(STRING_SPAN, 2, 20, 2, 1, 1, FGC, BGC);
										draw_double(span, 76, 20, 1, "", ERR);
										break;
								case 2: 
										active_value=2;
										LCD_Print(STRING_ZERO, 2, 40, 2, 1, 1, FGC, BGC);
										draw_double(zero, 76, 40, 1, "", ERR);
										break;
								case 3: 
										active_value=3;
										LCD_Print(STRING_ENABLE_PR, 2, 60, 2, 1, 1, FGC, BGC);
										(enable_pressure)?
											LCD_Print("on ", 85, 60, 2, 1, 1, ERR, BGC)
										:	LCD_Print("off", 85, 60, 2, 1, 1, ERR, BGC);
										break;
								case 4:
										active_value = 4;
										LCD_Print(STRING_QUENCH_CURRENT, 2, 80, 2, 1, 1, FGC, BGC);
										draw_double(quench_current, 85, 80, 0, "mA", ERR);
										break;
								case 5:
										active_value = 5;
										LCD_Print(STRING_MEAS_CURRENT, 2, 100, 2, 1,1, FGC, BGC);
										draw_double(meas_current, 85, 100, 0, "mA", ERR);
										break;
							}
							break;

						case KEY_UP_S6:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value)
							{
								case 0:
										active_option--;
										if(!active_option) active_option = 5;
										paint_current_opt_page4(active_option, KEY_UP_S6);
										break;
								case 1:	
										//increase span
										span+=0.1;
										if(span > MAX_SPAN) span = MAX_SPAN;
										draw_double(span, 76, 20, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											span+=1;
											if(span > MAX_SPAN) span = MAX_SPAN;
											draw_double(span, 76, 20, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 2:	
										//increase zero
										zero+=0.1;
										if(zero > MAX_ZERO) zero = MAX_ZERO;
										draw_double(zero, 76, 40, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											zero+=1;
											//if(zero > MAX_ZERO) zero = MAX_ZERO;
											draw_double(zero, 76, 40, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 3:	
										enable_pressure =(enable_pressure)? false : true;
										(enable_pressure)?
											LCD_Print("on ", 85, 60, 2, 1,1, ERR, BGC)
										:	LCD_Print("off", 85, 60, 2, 1,1, ERR, BGC);
										break;
								case 4:
										//increase quench current
										quench_current+=1;
										if(quench_current > QUENCH_CURRENT_MAX) quench_current = QUENCH_CURRENT_MIN;
										draw_double(quench_current, 85, 80, 0, "mA", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											quench_current+=1;
											if(quench_current > QUENCH_CURRENT_MAX) quench_current = QUENCH_CURRENT_MIN;
											draw_double(quench_current, 85, 80, 0, "mA", ERR);
											_delay_ms(50);
										}
										break;
								case 5:
										//increase measurement current
										meas_current+=1;
										if(meas_current > MEAS_CURRENT_MAX) meas_current = MEAS_CURRENT_MIN;
										draw_double(meas_current, 85, 100, 0, "mA", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											meas_current+=1;
											if(meas_current > MEAS_CURRENT_MAX) meas_current = MEAS_CURRENT_MIN;
											draw_double(meas_current, 85, 100, 0, "mA", ERR);
											_delay_ms(50);
										}
										break;
										}
										options_changed = true;
										break;
											
						case KEY_TOP_S5:
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							active_page = 3;
							paint_options(active_page);
							active_option = 1;
							active_value = 0;
										
							paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout, wait_time);
							break;
						default: 		
										break;
					}	
					break;
				
				case 5:	// Page 5 - Diag/shutdown
					switch(keyhit())
					{
						case KEY_DOWN_S9:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 0: 
										active_option++;
										if(active_option == 6) active_option = 1;	
										paint_current_opt_page5(active_option, KEY_DOWN_S9);
										break;
								case 1:	
										if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\nthe system?", D_FGC, D_BGC))
										{
											LCD_Cls(BGC);
											ex_mode = ex_mode_shutdown_action;
										}
										else {
											active_page = 5;
											paint_options(active_page);
											active_option = 1;
											active_value = 0;
											paint_opt_values_page5(r_span, r_zero, total_volume);
										}
										break;
								case 2: 
										paint_diag(1);
										execute_pressed_key=0;pressed_key=0;
										ex_mode = ex_mode_diagnotic_action;
										break;
								case 3: 
										r_span-=0.1;
										//if(r_span < 0) r_span = 2;
										draw_double(r_span, 76, 60, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											r_span-=0.1;
											//if(r_span < 0) r_span = 2;
											draw_double(r_span, 76, 60, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 4: 
										r_zero-=0.1;
										//if(r_span < 0) r_span = 2;
										draw_double(r_zero, 76, 80, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_DOWN_S9)
										{
											r_zero-=0.1;
											//if(r_span < 0) r_span = 2;
											draw_double(r_zero, 76, 80, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								default:
										break;
							}
							break;
									
						case KEY_BOT_S10:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							if(ex_errorCode != ex_errorCode_Offline)
							{
								if(options_changed)
								{
									xbee_wake_up();
									buffer[0] = device_id>>8;
									buffer[1] = device_id;
												
									if(transmit_slow_min) 
									{
										buffer[2] = transmit_slow>>8;
										buffer[3] = transmit_slow;
									}
									else
									{
										buffer[2] = (transmit_slow*60)>>8;
										buffer[3] = transmit_slow*60;
									}
									if(transmit_fast_sec) 
									{
										buffer[4] = transmit_fast>>8;
										buffer[5] = transmit_fast;
									}
									else
									{
										buffer[4] = (transmit_fast*60)>>8;
										buffer[5] = transmit_fast*60;
									}
									buffer[6] = ((uint16_t)(res_min*10))>>8;
									buffer[7] = res_min*10;
									buffer[8] = ((uint16_t)(res_max*10))>>8;
									buffer[9] = res_max*10;

									buffer[10] = ((uint16_t)(quench_time*1000))>>8;
									buffer[11] = (quench_time*1000);
									buffer[12] = ((uint16_t)quench_current)>>8;
									buffer[13] = quench_current;
									buffer[14] = ((uint16_t)(wait_time*1000))>>8;
									buffer[15] = (wait_time*1000);
									buffer[16] = ((uint16_t)meas_current)>>8;
									buffer[17] = meas_current;

									buffer[18] = meas_cycles;
									buffer[19] = fill_timeout;
									buffer[20] = ((uint16_t)(span*10))>>8;
									buffer[21] = span*10;
									buffer[22] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
									buffer[23] = (ZERO_NULLPUNKT+zero)*10;
									buffer[24] = ((uint16_t)(batt_min*10))>>8;
									buffer[25] = batt_min*10;
									buffer[26] = ((uint16_t)(batt_max*10))>>8;
									buffer[27] = batt_max*10;
									buffer[28] = critical_batt;
									buffer[29] = status_byte;

									LCD_Cls(BGC);
									LCD_Print("Saving settings...", 5, 50, 2, 1, 1, FGC, BGC);
									#ifdef ALLOW_COM
										xbee_send_request(OPTIONS_CHANGED_MSG, buffer, 30, &dest_high, &dest_low);
									#else
										// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
										sending_cmd = xbee_pack_tx64_frame(OPTIONS_CHANGED_MSG, buffer, 29, dest_high, dest_low);
										xbee_send(buffer);
									#endif
									xbee_sleep();
												
									#ifdef ALLOW_EEPROM_SAVING
										eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
										eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
													
										eeprom_write_word(&ee_transmit_slow, transmit_slow);
										eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
										eeprom_write_word(&ee_transmit_fast, transmit_fast);
										eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
										eeprom_write_word(&ee_quench_time, (uint16_t) (quench_time*1000));
										eeprom_write_word(&ee_quench_current, (uint16_t) quench_current);
										eeprom_write_word(&ee_wait_time, (uint16_t) (wait_time*1000));
										eeprom_write_word(&ee_meas_current, (uint16_t) meas_current);
													
										eeprom_write_byte(&ee_meas_cycles, meas_cycles);
										eeprom_write_byte(&ee_fill_timeout, fill_timeout);
										eeprom_write_byte(&ee_he_min, he_min);
										eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
										eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
										eeprom_write_word(&ee_span, (uint16_t) (span*10));
										eeprom_write_word(&ee_zero, (uint16_t) ((ZERO_NULLPUNKT+zero)*10));
										eeprom_write_byte(&ee_enable_pressure, enable_pressure);
										eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
										eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
										eeprom_write_byte(&ee_critical_batt, critical_batt);
									#endif
								}
								paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
								ex_mode = ex_mode_main_window;
								options_changed = false;
							}
							else
							{
								ex_mode = ex_mode_offline;
								paint_offline(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);
							}
							break;
										
						case KEY_LEFT_S7:		
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 1:	
										active_value=0;
										LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, ERR, BGC);
										LCD_Print("off", 85, 20, 2, 1,1, FGC, BGC);
										break;
								case 2: 
										active_value=0;
										LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, ERR, BGC);
										LCD_Print("off", 85, 40, 2, 1,1, FGC, BGC);
										break;
								case 3: 
										active_value=0;
										LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, ERR, BGC);
										draw_double(r_span, 76, 60, 1, "", FGC);
										break;		
								case 4: 
										active_value=0;
										LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, ERR, BGC);
										draw_double(r_zero, 76, 80, 1, "", FGC);
										break;
								case 5:
										active_value=0;
										LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, ERR, BGC);
										draw_double(total_volume, 76, 100, 1, "L", FGC);
										break;
							}
							break;
										
						case KEY_RIGHT_S8:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							if (!active_value)
							{
								switch(active_option) 
								{
									case 1:	
											active_value=1;
											LCD_Print(STRING_SHUTDOWN, 2, 20, 2, 1,1, FGC, BGC);
											LCD_Print("off", 85, 20, 2, 1,1, ERR, BGC);
											break;
									case 2: 
											active_value=2;
											LCD_Print(STRING_DIAG, 2, 40, 2, 1,1, FGC, BGC);
											LCD_Print("off", 85, 40, 2, 1,1, ERR, BGC);
											break;
									case 3: 
											active_value=3;
											LCD_Print(STRING_ADCSPAN, 2, 60, 2, 1,1, FGC, BGC);
											draw_double(r_span, 76, 60, 1, "", ERR);
											break;
									case 4: 
											active_value=4;
											LCD_Print(STRING_ADCZERO, 2, 80, 2, 1,1, FGC, BGC);
											draw_double(r_zero, 76, 80, 1, "", ERR);
											break;
									case 5:
											active_value=0;
											LCD_Print(STRING_TOTAL_VOL, 2, 100, 2, 1,1, FGC, BGC);
											draw_double(total_volume, 76, 100, 1, "L", FGC);
											break;
								}
							}
							break;
										
						case KEY_UP_S6:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							switch(active_value) 
							{
								case 0:
										active_option--;
										if(!active_option) active_option = 5;	
										paint_current_opt_page5(active_option, KEY_UP_S6);
										break;
								case 1:	
										if(LCD_Dialog(STRING_SHUTDOWN,"Do you really\nwant to shut down\n the system?", D_FGC, D_BGC))
										{
											LCD_Cls(BGC);
											ex_mode = ex_mode_shutdown_action;
										}
										else {
											active_page = 5;
											paint_options(active_page);
											active_option = 1;
											active_value = 0;
										}
										break;
								case 2: 
										paint_diag(1);
										execute_pressed_key=0;pressed_key=0;
										ex_mode = ex_mode_diagnotic_action;
										break;
								case 3: 
										r_span+=0.1;
										//if(r_span < 0) r_span = 2;
										draw_double(r_span, 76, 60, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											r_span+=0.1;
											//if(r_span < 0) r_span = 2;
											draw_double(r_span, 76, 60, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								case 4: 
										r_zero+=0.1;
										//if(r_span < 0) r_span = 2;
										draw_double(r_zero, 76, 80, 1, "", ERR);
										_delay_ms(300);
										while(keyhit()==KEY_UP_S6)
										{
											r_zero+=0.1;
											//if(r_span < 0) r_span = 2;
											draw_double(r_zero, 76, 80, 1, "", ERR);
											_delay_ms(50);
										}
										break;
								default:
										break;
							}
							options_changed = true;
							break;
										
						case KEY_TOP_S5:	
										
							if (!display_on()) 
							// Display was off, turn it on
							{
								#ifdef ALLOW_DOUBLE_CLICK 
									;		// Double click allowed, execute following code
								#else
									break;	// Double click not allowed, break here and wait for a pressed key 
								#endif
							}
							active_page = 4;
							paint_options(active_page);
							active_option = 1;
							active_value = 0;
										
							paint_opt_values_page4(span, zero, enable_pressure, quench_current, meas_current);
							break;
						default:		
							break;
					}
					break;
				default:
					break;
				}
			
			break;

		case ex_mode_shutdown_action:		// Shutdown requested by the user
			// Switch on current supply board
			MEASURE_PIN_ON
			// Get levels
//			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
			batt_level = get_batt_level(batt_min, batt_max, r_zero);
			pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
			// Switch off current supply board
			MEASURE_PIN_OFF
			// Wake up XBee module
			xbee_wake_up();
			// Pack data in frame
			buffer[0] = device_id>>8;
			buffer[1] = device_id;
			if(he_level < 0)	// If negative He level, send error code instead of a wrong value
			{
				buffer[2] = HE_LEVEL_ERROR>>8;
				buffer[3] = (uint8_t) HE_LEVEL_ERROR;
			}
			else
			{
				buffer[2] = ((uint16_t)(he_level*10))>>8;
				buffer[3] = ((uint16_t)(he_level*10));
			}
			buffer[4] = batt_level;
			buffer[5] = pressure_level >> 8;
			buffer[6] = pressure_level;
			buffer[7] = status_byte;
			// Send to the database server
			#ifdef ALLOW_COM
				xbee_send_request(LOGOUT_MSG, buffer, 8, &dest_high, &dest_low);
			#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				sending_cmd = xbee_pack_tx64_frame(LOGOUT_MSG, buffer, 8, dest_high, dest_low);
				xbee_send(buffer);
			#endif
			// Sleep XBee module
			xbee_sleep();
			// Set shutdown pin
			SHUTDOWN
			// PROGRAM ENDS HERE
			_delay_ms(2000);			
			//while(1);	// If shutting down does not work, program will stay here
			ex_mode = ex_mode_error;
			ex_errorCode = ex_errorCode_Shutdown;
			break;


		case ex_mode_diagnotic_action:
			switch(pressed_key)//switch(keyhit())
			{
				case KEY_DOWN_S9:		// Switch ON/OFF current source (only on page 1)
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					if(diag_to_show == 1)
					{
						if(CHECK_MEASURE_PIN)
						{
							// Stop PWM
							DDRD &= (0 << PD7);			// Set PORTD.7 as input

							MEASURE_PIN_OFF

							LCD_Print("OFF", 40, 100, 2, 1,1, ERR, BGC);
						}
						else
						{
							// Start PWM
							TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
							TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
							DDRD |= (1 << PD7);			// Set PORTD.7 as output

							// Switch on current supply to 150 mA to quench the He probe (153.67 mA measured)
							// OCR2A value should be in 0 to 255 range
							// 0 gives 0 mA / 255 gives 238 mA
							//OCR2A = 160;
							OCR2A = (uint8_t)round(meas_current*(double)(255/238));

							MEASURE_PIN_ON
							
							LCD_Print("ON ", 40, 100, 2, 1,1, ERR, BGC);
						}
					}
					_delay_ms(250);
					break;
													
				case KEY_BOT_S10:	
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					diag_pulse(&quench_time, r_span, r_zero);
					paint_diag(diag_to_show);
					options_changed = true;
					break;
														
				case KEY_LEFT_S7:		
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					diag_to_show++;
					if(diag_to_show == 3) {diag_to_show = 1;}
						paint_diag(diag_to_show);
						break;
														
				case KEY_RIGHT_S8:	
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					diag_to_show--;
					if(diag_to_show == 0) {diag_to_show = 2;}
						paint_diag(diag_to_show);
						break;
														
														
				case KEY_UP_S6:	
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					ex_mode = ex_mode_calibration;
					break;
														
														
				case KEY_TOP_S5:	
					execute_pressed_key = 0;
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					//if(LCD_Dialog("Diagnostic","Do you really\nwant to stop\ndiagnostic procedure?", D_FGC, D_BGC))
					//{
						if(CHECK_MEASURE_PIN) MEASURE_PIN_OFF
						ex_mode = ex_mode_options_window;
						active_page = 5;
						paint_options(active_page);
						active_option = 1;
						active_value = 0;
						diag_to_show = 1;
						paint_opt_values_page5(r_span, r_zero, total_volume);
					//}
					//else paint_diag(diag_to_show);
					break;
				default:		
					switch(diag_to_show)
					{
						case 1:	
							diag_page1(r_zero, r_span, batt_min, batt_max, res_min, res_max, zero, span);
							break;
						case 2:
							diag_page2(r_zero);
							break;
						default:
							break;
					}
					break;
				}
			break;
/*		case MODE_OFFLINE:				// Use device offline
			switch(keyhit())
			{
				case KEY_MEASURE:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					
					paint_measure(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, 0);
					// Switch on current supply board
					MEASURE_PIN_ON
					// Get levels
					he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 1);
					batt_level = get_batt_level(batt_min, batt_max, r_zero);
					pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
					// Switch off current supply board
					MEASURE_PIN_OFF
					
					if(force_measurement)
					{
						forced_measurement_done = true;
					}
//					else if(err_code == OFFLINE_errCode)
					else
					{
						paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,0);
						running_mode = MODE_OFFLINE;
					}
// 					else
// 					{
// 						paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
// 						running_mode = MODE_MAIN_WINDOW;
// 					}
					break;
				case KEY_UP_S6:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_LEFT_S7:		
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_RIGHT_S8:		
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_DOWN_S9:	
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					break;
				case KEY_BOT_S10:	
					// Go to options pages...
					if (!display_on()) 
					// Display was off, turn it on
					{
						#ifdef ALLOW_DOUBLE_CLICK 
							;		// Double click allowed, execute following code
						#else
							break;	// Double click not allowed, break here and wait for a pressed key 
						#endif
					}
					
					uint8_t status;	// Pressed "cancel" or "ok"
					entered_options_pw = get_number(&status, PASSWORD);
					
					if(status || (entered_options_pw != options_pw))
					{
						// Cancelled or wrong password, stay in offline mode
						paint_offline(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
						break;
					}
					
					// Set options mode	
					running_mode = MODE_OPTIONS_WINDOW;
					// Init. options flow control
					active_option = 1; active_value = 0; active_page = 1;
					// Display options page 1
					paint_options(active_page);
					// Show current option values
					paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, quench_time, meas_cycles, fill_timeout);
					break;
				default:
					// Do anything
					break;
				}		// End of switch
			break;	// End of offline mode
*/
		case ex_mode_error:				// Manage errors
			LCD_Cls(BGC);
			switch(ex_errorCode)
			{
				case ex_errorCode_LoginFailed:	
					LCD_Print("Connection", 5, 40, 2, 1, 1, FGC, BGC);
					LCD_Print("to server failed!", 5, 60, 2, 1, 1, FGC, BGC);
					LCD_Print("Restart device to try again", 2, 90, 1, 1, 1, FGC, BGC);
					while(1);	//no login, no game
				case ex_errorCode_Shutdown:
					LCD_Print("Failed", 5, 40, 2, 1, 1, FGC, BGC);
					LCD_Print("to shutdown!", 5, 60, 2, 1, 1, FGC, BGC);
					LCD_Print("Probably hardware issue...", 2, 90, 1, 1, 1, FGC, BGC);
					while(1);	// Stay here
				default:		
					LCD_Print("unknown error", 5, 40, 2, 1, 1, FGC, BGC);
					LCD_Print("(probably hardware)", 5, 60, 2, 1, 1, FGC, BGC);
					char bufferStr[20];
					sprintf(bufferStr, "code = %u", ex_errorCode);
					LCD_Print(bufferStr, 2, 90, 1, 1, 1, FGC, BGC);
					break;
			}
			break;									

		case ex_mode_calibration:			
			LCD_Cls(BGC);
			if(LCD_Dialog("Calibration", "Set current span/\nzero to default?", D_FGC, D_BGC))
			{
				// User answered "Yes"
				r_zero = R_ZERO_DEF;
				r_span = R_SPAN_DEF;
			}
			else
			{
				// User answered "No, I want to calibrate the levelmeter"
				if(!CHECK_MEASURE_PIN)
				{
					// Start PWM
					TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
					TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
					DDRD |= (1 << PD7);			// Set PORTD.7 as output

					// Set current output to measurement value (user defined)
					OCR2A = (uint8_t)round(meas_current*(double)(255/238));

					// Close relay
					MEASURE_PIN_ON
				}
				
//				if(!LCD_Dialog("Calibration", "Set resistance\nto 0 ohm", D_FGC, D_BGC))
				sprintf(temp," Set resistance\nto %d ohm", (int)res_min);
				if(!LCD_Dialog("Calibration", temp, D_FGC, D_BGC))
				{
					// User answered "No"
					_delay_ms(300);
					execute_pressed_key = 0;
					pressed_key = 0;				//sonst key sofort im diag aktiv

					// Open relay
					MEASURE_PIN_OFF

					// Set current output to measurement value (user defined)
					OCR2A = 0;

					// Stop PWM
					DDRD &= (0 << PD7);			// Set PORTD.7 as input

					execute_pressed_key = 0; pressed_key = 0;	// Otherwise key active immediately in dialog
					options_changed = false;
					ex_mode = ex_mode_diagnotic_action;

					break;
				}
				else
				{				
					// User answered "Yes"
					temp_r_zero = (double)((((res_min + 10.0)/r_span)*(r_zero + (readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS)))) - readChannel(VOLT_PROBE_MEAS, ADC_LOOPS));
			
//					LCD_Dialog("Calibration", "Set resistance\nto 340 ohm", D_FGC, D_BGC);
					sprintf(temp," Set resistance\nto %d ohm", (int)res_max);
					LCD_Dialog("Calibration", temp, D_FGC, D_BGC);
			
					uint16_t adc_v = readChannel(VOLT_PROBE_MEAS, ADC_LOOPS);
					uint16_t adc_i = readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS);
					double temp_double1 = (double)(adc_v + temp_r_zero)/(adc_i + temp_r_zero);
//					temp_r_span = (double)((350)/(temp_double1));
					temp_r_span = (double)((res_max + 10.0)/(temp_double1));
			
					// Open relay
					MEASURE_PIN_OFF

					// Set current output to measurement value (user defined)
					OCR2A = 0;

					// Stop PWM
					DDRD &= (0 << PD7);			// Set PORTD.7 as input
			
//					sprintf(temp," Span value: %f \n\n Zero value: %f \n\n Would you like to save?", temp_r_span, temp_r_zero);
					sprintf(temp," Span value: %d \n\n Zero value: %d \n\n Would you like to save?", (int)temp_r_span, (int)temp_r_zero);
					if(LCD_Dialog("Calibration", temp, D_FGC, D_BGC))
					{
						r_zero = temp_r_zero;
						r_span = temp_r_span;
						options_changed = true;
					}
					paint_diag(diag_to_show);
					execute_pressed_key = 0; pressed_key = 0;	// Otherwise key active immediately in dialog
					ex_mode = ex_mode_diagnotic_action;
				}
			}
		
		break;

		
		default:					
			//should never happen
			LCD_Cls(BGC);
			LCD_Print("Fatal error: unknown mode", 5, 40, 1, 1, 1, FGC, BGC);
			sprintf(temp,"mode: %i \n", ex_mode);
			LCD_Print(temp, 5, 60, 1, 1, 1, FGC, BGC);
//			while(1);
			ex_mode = ex_mode_main_window;
			break;
		}

		//=========================================================================
		// "User bugs on the keyboard"
		//=========================================================================
		// If it is not running dialog action
		if(ex_mode != ex_mode_diagnotic_action)
		{
			// Waits until the user stop pressing a key	
			while(!(keyhit() == 0));
				_delay_ms(10);
		}
		// If any saved key to execute
		if(!execute_pressed_key)
			pressed_key = 0;	// Clear saved key (PCI_int)
		
		//=========================================================================
		// Manage XBee module sleep cycles
		//=========================================================================
		// If XBee is not busy and it is not offline
		if(!xbee_busy && (ex_errorCode != ex_errorCode_Offline))
		{
			if(xbee_sleeping)
			{
				// XBee module is sleeping
				if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
				{
					xbee_wake_up();
					
					// Define frame
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					buffer[2] = status_byte;
					
					// Try to reconnect to the network
					xbee_reconnect(&dest_low, &dest_high);
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(XBEE_ACTIVE_MSG, buffer, 3, dest_high, dest_low);
					xbee_send(buffer);
					
					// Clear then set the timeout for awake time
					set_timeout(0, TIMER_5, RESET_TIMER);
					set_timeout(XBEE_AWAKE_TIME, TIMER_5, USE_TIMER);		// Stay active for 30 sec
					xbee_sleeping = false;			// Xbee module is not sleeping anymore
					#ifdef ALLOW_DEBUG
						LCD_Print("XBee awake", 5, 20, 2, 1, 1, FGC, BGC);
					#endif
				}
			}	
			else 
			{	// XBee module is awake
				if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
				{	
					xbee_sleep();
					// Clear then set the timeout for sleeping time
					set_timeout(0, TIMER_5, RESET_TIMER);
					set_timeout(xbee_sleep_period*60, TIMER_5, USE_TIMER);
					xbee_sleeping = true;			// Xbee module is sleeping
					#ifdef ALLOW_DEBUG
						LCD_Print("XBee is sleeping", 5, 20, 2, 1, 1, FGC, BGC);
					#endif
				}
			}
		}
		
		//=========================================================================
		// Forced measurement (button pressed)
		//=========================================================================
		// DO_MEASUREMENT is true when "Measure" button is pressed
		// Allowed only in main window and filling modes
		if(DO_MEASUREMENT && (ex_mode == ex_mode_main_window || ex_mode == ex_mode_fill_action))
		{
			DISPLAY_TURN_ON;			// Wakes up display
			force_measurement = true;	// Set forced measurement
			saved_mode = ex_mode;			// Saves the running mode before to change it
			ex_mode = ex_mode_measure_action;	// Set measure action for running mode
		}
		else if (forced_measurement_done)
		{
			// User request for forced measured is done
			DISPLAY_TURN_ON;			// Wakes up display
			ex_mode = saved_mode;	// Reset the previous mode (either main or filling mode)
			// Paint screen according to running mode
			switch(ex_mode)
			{
				case ex_mode_main_window:	
					paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
					break;
				case ex_mode_fill_action:	
					paint_filling(device_id, device_pos, he_level, total_volume, batt_level,critical_batt, 0);
					LCD_Print("Waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
					sprintf(temp,FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
					LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
					break;
				default:				
					ex_mode = ex_mode_main_window;	// Set main mode as default
					break;
			}
			// Clear flags
			forced_measurement_done = false;
			force_measurement = false;
		}
		
		//=========================================================================
		// Display standby
		//=========================================================================
		#ifdef ALLOW_DISPLAY_TIMER
			if(!set_timeout(0, TIMER_2, USE_TIMER)) 
			{
				if((ex_mode == ex_mode_main_window) || (ex_mode == ex_mode_offline))
					DISPLAY_TURN_OFF		// Turn off display
				
				#ifdef ALLOW_DEBUG
					LCD_Print("off", 5, 20, 2, 1, 1, FGC, BGC);
				#endif	
			}
		#endif
		
		//=========================================================================
		// Interval slow changed (Time interval between regular network connections)
		//=========================================================================
		// If time interval between regular network connections has been changed
		// and new option settings have been successfully sent to the database server
		if(transmit_slow_changed && !options_changed)
		{	
			// Reset timer dedicated to interval slow
			set_timeout(0, TIMER_1, RESET_TIMER);
			
			// Then set a new timer value in seconds
			(transmit_slow_min)? 
				set_timeout(ceil(transmit_slow*60), TIMER_1, USE_TIMER)	// with transmit_slow in minutes
			:	set_timeout(transmit_slow*3600, TIMER_1, USE_TIMER);	// with transmit_slow in hours
			
			transmit_slow_changed = false;		// Reset marker
		}
		
		//=========================================================================
		// Auto filling
		//=========================================================================
// 		if((running_mode == MODE_MAIN_WINDOW) && auto_fill_pin_on() && auto_fill_enabled)
// 		{//auto fill	
// 			//Kanne zu leer, back to main, no filling
// 			if(very_low_He_level)
// 			{
// 				LCD_Dialog("HE LEVEL", "Filling is disabled.\nHe Level is too low.", D_FGC, D_BGC);
// 				running_mode = MODE_MAIN_WINDOW;
// 				paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
// 			}
// 			else {
// 				_delay_ms(100);
// 				//ext. Fuellen nicht mehr an?
// 				if(!auto_fill_pin_on())
// 				{
// 					running_mode = MODE_MAIN_WINDOW;
// 					paint_main(device_id,device_pos,he_level,batt_level,critical_batt,PAINT_ALL);
// 				}
// 				else {
// 					LCD_Cls(BGC);
// 					LCD_Print("auto fill init.", 5, 40, 2, 1, 1, FGC, BGC);
// 					LCD_Print("sending data...", 5, 60, 2, 1, 1, FGC, BGC);
// 					
// 					xbee_wake_up();
// 					xbee_busy = true;
// 					
// 					uint8_t indx = 0;
// 					buffer[0] = device_id>>8;
// 					buffer[1] = device_id;
// 					indx = devicePos_to_buffer(device_pos, 2, buffer);
// 					buffer[indx++] = status_byte;
// 					
// 					//send
// 					#ifdef ALLOW_COM
// 						xbee_send_request(FILLING_BEGIN_MSG, buffer, indx, &dest_high, &dest_low);
// 					#else
// 						// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
// 						sending_cmd = xbee_pack_tx64_frame(FILLING_BEGIN_MSG, buffer, indx, dest_high, dest_low);
// 						xbee_send(buffer);
// 					#endif
// 					
// 					paint_filling(device_id, device_pos, he_level, batt_level,critical_batt,0);
// 					fill_meas_counter = fill_timeout/transmit_fast;
// 					running_mode = MODE_FILL_ACTION;
// 					//set output fill pin
// 					START_AUTO_FILL
// 					auto_fill_started = true;
// 				}
// 			}
// 		}
// 		
		
		//=========================================================================
		// Battery checking
		//=========================================================================
/*		if(batt_min >= map_to_batt(readChannel_calib(BATTERY, ADC_LOOPS, r_zero)))
		{	
			// Last measurement before shutting down
			MEASURE_PIN_ON
//			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
			batt_level = get_batt_level(batt_min, batt_max, r_zero);
			pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
			MEASURE_PIN_OFF
			
			sprintf(temp,"The battery is\ncritically low (%d%%).\nSystem will shut down!\nLast He Level:\n%d%%",((uint8_t) batt_level),((uint8_t) he_level));
			timed_dialog("Shutting down...", temp, 10, D_FGC, D_BGC); 			
			
			// Send levels to the database server
			xbee_wake_up();
			
			buffer[0] = device_id>>8;
			buffer[1] = device_id;
			
			if(he_level < 0)	// If negative He level, send error code
			{
				buffer[2] = HE_LEVEL_ERROR>>8;
				buffer[3] = (uint8_t) HE_LEVEL_ERROR;
			}
			else
			{
				buffer[2] = ((uint16_t)(he_level*10))>>8;
				buffer[3] = ((uint16_t)(he_level*10));
			}
			
			buffer[4] = batt_level;
			buffer[5] = pressure_level >> 8;
			buffer[6] = pressure_level;
			buffer[7] = status_byte;
			
			#ifdef ALLOW_COM
				// Send request and wait for answer
				xbee_send_request(LOGOUT_MSG, buffer, 8, &dest_high, &dest_low);
			#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				sending_cmd = xbee_pack_tx64_frame(LOGOUT_MSG, buffer, 8, dest_high, dest_low);
				xbee_send(buffer);
			#endif

			// Switch off the power supply
			SHUTDOWN
			_delay_ms(2000);
	
			// If pin doesn't work go to mysterious error
			ex_mode = ex_mode_error;
			ex_errorCode = ex_errorCode_Shutdown;
			
		}
*/		
		//=========================================================================
		// Check receive buffer for commands from database server
		//=========================================================================
		uint8_t reply_Id = xbee_hasReply(LAST_NON_CMD_MSG, GREATER_THAN);
		if(reply_Id != 0xFF) 		// Always check for command in buffer and do one at a time
		{
			switch(frameBuffer[reply_Id].type) 
			{
				case STATUS_MSG:	// (#11) Send levels and status to the database server.
					paint_wait_screen();
					
					// Enable measures and get levels
					MEASURE_PIN_ON
//					he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
					he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, quench_current, wait_time, meas_current, 0);
					batt_level = get_batt_level(batt_min, batt_max, r_zero);
					pressure_level = (enable_pressure)? map_to_pres(readChannel_calib(PRESSURE, ADC_LOOPS, r_zero), zero, span) : 0;
					MEASURE_PIN_OFF
					
					// Build data frame
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					
					if(he_level < 0)  // If negative He level, send corresponding error code
					{
						buffer[2] = ((uint16_t)(HE_LEVEL_ERROR))>>8;
						buffer[3] = (uint8_t) HE_LEVEL_ERROR;
					}
					else	// If no error, send He level
					{
						buffer[2] = ((uint16_t)(he_level*10))>>8;
						buffer[3] = ((uint16_t)(he_level*10));
					}
					buffer[4] = batt_level;
					buffer[5] = pressure_level >> 8;
					buffer[6] = pressure_level;
					buffer[7] = status_byte;
					
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(STATUS_MSG, buffer, 8, dest_high, dest_low);
					xbee_send(buffer);
					
					// Update display
					switch(ex_mode)
					{
						case ex_mode_main_window:
							paint_main(device_id, device_pos, he_level, total_volume, batt_level, critical_batt, PAINT_ALL);
							break;
						case ex_mode_fill_action:
							paint_filling(device_id, device_pos, he_level, total_volume, batt_level,critical_batt, 0);
							LCD_Print("waiting  ", 2, 20, 2, 1, 1, FGC, BGC);
							sprintf(temp,FILL_WAITING_LABEL, fill_meas_counter*transmit_fast);
							LCD_Print(temp, 5, 105, 1, 1, 1, FGC, BGC);
							break;
						case ex_mode_options_window:
							paint_options(active_page);
							active_option = 1;
							active_value = 0;
							switch(active_page)
							{
								case 1:
									paint_opt_values_page1(transmit_slow_min, transmit_slow, transmit_fast, quench_time, meas_cycles, fill_timeout);
									break;
								case 2:
									paint_opt_values_page2(res_min, res_max, batt_min, batt_max, critical_batt);
									break;
								case 3:
									paint_opt_values_page3(device_pos, auto_fill_enabled, he_min, fill_timeout, wait_time);
									break;
								case 4:
									paint_opt_values_page4(span, zero, enable_pressure, quench_current, meas_current);
									break;
								case 5:		
									paint_options(5);
									break;
								default:
									LCD_Print("opt def", 5, 5, 2, 1, 1, ERR, BGC);
									break;
							}
							break;
						case ex_mode_diagnotic_action:
							paint_diag(diag_to_show);
							break;
						default:				
							break;
					}	// End switch
					break;
				case GET_OPTIONS_CMD:	// (#12) Send device settings to the database server.
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					
					if(transmit_slow_min) 
					{
						buffer[2] = transmit_slow>>8;
						buffer[3] = transmit_slow;
					}
					else
					{
						buffer[2] = (transmit_slow*60)>>8;
						buffer[3] = transmit_slow*60;
					}
					if(transmit_fast_sec) 
					{
						buffer[4] = transmit_fast>>8;
						buffer[5] = transmit_fast;
					}
					else
					{
						buffer[4] = (transmit_fast*60)>>8;
						buffer[5] = transmit_fast*60;
					}
					buffer[6] = ((uint16_t)(res_min*10))>>8;
					buffer[7] = res_min*10;
					buffer[8] = ((uint16_t)(res_max*10))>>8;
					buffer[9] = res_max*10;
				
					buffer[10] = ((uint16_t)(quench_time*1000))>>8;
					buffer[11] = (quench_time*1000);
					buffer[12] = ((uint16_t)quench_current)>>8;
					buffer[13] = quench_current;
					buffer[14] = ((uint16_t)(wait_time*1000))>>8;
					buffer[15] = (wait_time*1000);
					buffer[16] = ((uint16_t)meas_current)>>8;
					buffer[17] = meas_current;

					buffer[18] = meas_cycles;
					buffer[19] = fill_timeout;
					buffer[20] = ((uint16_t)(span*10))>>8;
					buffer[21] = span*10;
					buffer[22] = ((uint16_t)((ZERO_NULLPUNKT+zero)*10))>>8;
					buffer[23] = (ZERO_NULLPUNKT+zero)*10;
					buffer[24] = ((uint16_t)(batt_min*10))>>8;
					buffer[25] = batt_min*10;
					buffer[26] = ((uint16_t)(batt_max*10))>>8;
					buffer[27] = batt_max*10;
					buffer[28] = critical_batt;
					buffer[29] = status_byte;
					
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(GET_OPTIONS_CMD, buffer, 30, dest_high, dest_low);
					xbee_send(buffer);
					break;
				case SET_OPTIONS_CMD:	// (#13) Set device settings received from the database server.
					if (NUMBER_OPTIONS_BYTES == frameBuffer[reply_Id].data_len)
					{
						uint16_t temp_transmit_slow = (frameBuffer[reply_Id].data[0]<<8) + frameBuffer[reply_Id].data[1];
						if(transmit_slow != temp_transmit_slow)
						{
							transmit_slow_changed = true;
							transmit_slow = temp_transmit_slow;
							transmit_slow_min = true;
						}
						
						if(transmit_slow <= 0) transmit_slow = TRANSMIT_SLOW_DEF;
						if(transmit_slow > 60)
						{
							transmit_slow /= 60;
							transmit_slow_min = false;
						}
						else transmit_slow_min = true;
						
						transmit_fast = (frameBuffer[reply_Id].data[2]<<8) + frameBuffer[reply_Id].data[3];
						if(transmit_fast <= 0) transmit_fast = TRANSMIT_FAST_DEF;
						if(transmit_fast > 60)
						{
							transmit_fast /= 60;
							transmit_fast_sec = false;
						}
						else transmit_fast_sec = true;
						
						res_min = ((frameBuffer[reply_Id].data[4]<<8) + frameBuffer[reply_Id].data[5])/10.0;
						if(res_min <= 0) res_min = RES_MIN_DEF;
						
						res_max = ((frameBuffer[reply_Id].data[6]<<8) + frameBuffer[reply_Id].data[7])/10.0;
						if(res_max <= 0) res_max = RES_MAX_DEF;
						
						quench_time = ((frameBuffer[reply_Id].data[8]<<8) + frameBuffer[reply_Id].data[9])/1000.0;
						if(quench_time <= 0) quench_time = QUENCH_TIME_DEF;
						
						quench_current = (frameBuffer[reply_Id].data[10]<<8) + frameBuffer[reply_Id].data[11];
						if(quench_current <= 0) quench_current = QUENCH_CURRENT_DEF;
						
						wait_time = ((frameBuffer[reply_Id].data[12]<<8) + frameBuffer[reply_Id].data[13])/1000.0;
						if(wait_time <= 0) wait_time = WAIT_TIME_DEF;
						
						meas_current = (frameBuffer[reply_Id].data[14]<<8) + frameBuffer[reply_Id].data[15];
//						meas_current = (meas_current <= 0)? MEAS_CURRENT_DEF;
						if(meas_current <= 0) meas_current = MEAS_CURRENT_DEF;
						
						meas_cycles = (!frameBuffer[reply_Id].data[16])? MEASUREMENT_CYCLES_DEF : frameBuffer[reply_Id].data[16];
						fill_timeout = (!frameBuffer[reply_Id].data[17])? FILLING_TIMEOUT_DEF : frameBuffer[reply_Id].data[17];
						
						span = ((frameBuffer[reply_Id].data[18]<<8) + frameBuffer[reply_Id].data[19])/10.0;
						if(span <= 0) span = SPAN_DEF;
						
						zero = (double)((((frameBuffer[reply_Id].data[20]<<8) + frameBuffer[reply_Id].data[21])/10.0)-ZERO_NULLPUNKT);
						//zero = ZERO_NULLPUNKT - zero;
						
						total_volume = ((frameBuffer[reply_Id].data[22]<<8) + frameBuffer[reply_Id].data[23])/10.0;
						if(total_volume <= TOTAL_VOL_MIN) total_volume = TOTAL_VOL_DEF;
						
						batt_min = ((frameBuffer[reply_Id].data[24]<<8) + frameBuffer[reply_Id].data[25])/10.0;
						if(batt_min <= 0) batt_min = BATT_MIN_DEF;
						
						batt_max = ((frameBuffer[reply_Id].data[26]<<8) + frameBuffer[reply_Id].data[27])/10.0;
						if(batt_max <= 0) batt_max = BATT_MAX_DEF;
						
						critical_batt = (!frameBuffer[reply_Id].data[28])? CRITICAL_BATT_DEF : frameBuffer[reply_Id].data[28];
					
						// Save settings in EEPROM
						#ifdef ALLOW_EEPROM_SAVING
							//eeprom_write_word(&ee_r_zero, (uint16_t)(r_zero*10));
							//eeprom_write_word(&ee_r_span, (uint16_t)(r_span*10));
							
							eeprom_write_word(&ee_transmit_slow, transmit_slow);
							eeprom_write_byte(&ee_transmit_slow_min, transmit_slow_min);
							eeprom_write_word(&ee_transmit_fast, transmit_fast);	
							eeprom_write_byte(&ee_transmit_fast_sec, transmit_fast_sec);
							eeprom_write_word(&ee_quench_time, (uint16_t) (quench_time*1000));
							eeprom_write_word(&ee_quench_current, (uint16_t) quench_current);
							eeprom_write_word(&ee_wait_time, (uint16_t) (wait_time*1000));
							eeprom_write_word(&ee_meas_current, (uint16_t) meas_current);
							
							eeprom_write_byte(&ee_meas_cycles, meas_cycles);
							eeprom_write_byte(&ee_fill_timeout, fill_timeout);
							//eeprom_write_byte(&ee_he_min, he_min);
							eeprom_write_word(&ee_res_min, (uint16_t) (res_min*10));
							eeprom_write_word(&ee_res_max, (uint16_t) (res_max*10));
							eeprom_write_word(&ee_span, (uint16_t) (span*10));
							eeprom_write_word(&ee_zero, (uint16_t) ((ZERO_NULLPUNKT+zero)*10));
							//eeprom_write_byte(&ee_enable_pressure, enable_pressure);
							eeprom_write_word(&ee_batt_min, (uint16_t) (batt_min*10));
							eeprom_write_word(&ee_batt_max, (uint16_t) (batt_max*10));
							eeprom_write_byte(&ee_critical_batt, critical_batt);

							eeprom_write_word(&ee_total_volume, (uint16_t)total_volume);
						#endif
					}
					else
					{	// Set default settings
						transmit_slow = TRANSMIT_SLOW_DEF;
						transmit_slow_min = false;
						transmit_fast = TRANSMIT_FAST_DEF;
						quench_time = QUENCH_TIME_DEF;
						quench_current = QUENCH_CURRENT_DEF;
						wait_time = WAIT_TIME_DEF;
						meas_current = MEAS_CURRENT_DEF;
						meas_cycles = MEASUREMENT_CYCLES_DEF;
						fill_timeout = FILLING_TIMEOUT_DEF;
						res_min = RES_MIN_DEF;
						res_max = RES_MAX_DEF;
						span = SPAN_DEF;
						zero = ZERO_DEF;
						batt_min = BATT_MIN_DEF;
						batt_max = BATT_MAX_DEF;
						critical_batt = CRITICAL_BATT_DEF;
						total_volume = TOTAL_VOL_DEF;
					}
					break;
				case GET_LETTERS_CMD:	// (#14) Send list of available device positions to the database server.
				{				
					char *ptr;
					uint8_t index = 1;
					uint8_t i = 2;
					ptr = &fill_pos_letters[index][0];
					
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					
					while(*ptr != '\r')
					{
						//ptr = fill_pos_letters[index][0];
						while(*ptr != '\0')
						{
							buffer[i++] = *ptr++;
						}
						buffer[i++] = ';';
						ptr = ((char*) &fill_pos_ranges[index][1]);
						
						while(*ptr != END)
						{
							buffer[i++] = ((char)(*ptr++));
						}
						buffer[i++] = SEP;
						
						index++;
						ptr = &fill_pos_letters[index][0];
					}
					buffer[i++] = status_byte;
					
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(GET_LETTERS_CMD, buffer, i, dest_high, dest_low);
					xbee_send(buffer);
					break;
				}
				case SET_LETTERS_CMD:	// (#15) Set list of available device positions received from the database server.
				{
					// Get data length and data
					uint8_t byte_number = frameBuffer[reply_Id].data_len;
					uint8_t *ptr = frameBuffer[reply_Id].data;
					
					// Init. variables
					uint8_t i_letters = 1;
					uint8_t j_letters = 0;
					uint8_t j_ranges = 1;
					
					// Init. arrays
					fill_pos_letters[0][0] ='\r'; 
					fill_pos_letters[0][1] ='\0';
					fill_pos_ranges[0][0] = END;
					
					while(byte_number)
					{
						fill_pos_letters[i_letters][j_letters] = ((char)(*ptr));
						j_letters++;
						ptr++;
						byte_number--;
						
						if(*ptr == ';') //ranges 
						{
							fill_pos_letters[i_letters][j_letters] = '\0';
							fill_pos_ranges[i_letters][0] = END;
							ptr++;
							byte_number--;
							
							while(*ptr != SEP)
							{
								fill_pos_ranges[i_letters][j_ranges] = *ptr;
								j_ranges++;
								ptr++;
								byte_number--;
								// If ranges index is higher than the maximum value, break
								if(j_ranges == FILL_RANGES_LENGTH-1)
									break;
							}
							fill_pos_ranges[i_letters][j_ranges] = END;
							
							ptr++;
							byte_number--;
							i_letters++;
							// If letters index is higher than the maximum value, break
							if(i_letters == FILL_RANGES_COUNT-1) 
								break;
							j_letters = 0;
							j_ranges = 1;
						}
					}
					fill_pos_letters[i_letters][0] ='\r';
					fill_pos_ranges[i_letters][0] = END; 
					break;				
				}
				case GET_PASSWORD_CMD:	// (#16) Send options password to the database server. This password is required to access to settings pages.
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					buffer[2] = options_pw >> 8;
					buffer[3] = options_pw;
					buffer[4] = status_byte;
					
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(GET_PASSWORD_CMD, buffer, 5, dest_high, dest_low);
					xbee_send(buffer);
					break;
				case SET_PASSWORD_CMD:	// (#17) Set options password received from the database server. This password is required to access to settings pages.
					options_pw = (frameBuffer[reply_Id].data[0]<<8) + frameBuffer[reply_Id].data[1];
					break;
				case GET_XBEE_SLEEP_TIME_CMD:	// (#18) Send XBee sleeping period to the database server
					buffer[0] = device_id>>8;
					buffer[1] = device_id;
					buffer[2] = xbee_sleep_period;		// Sleeping period in minute
					buffer[3] = status_byte;
					
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(GET_XBEE_SLEEP_TIME_CMD, buffer, 4, dest_high, dest_low);
					xbee_send(buffer);
					break;
				case SET_XBEE_SLEEP_TIME_CMD:	// (#19) Set XBee sleeping period received from the database server
					xbee_sleep_period = frameBuffer[reply_Id].data[0];
					#ifdef ALLOW_DEBUG
						draw_int(xbee_sleep_period, 5,  190, "", blue);
					#endif
					break;
				default:	// (#10) Unknown command, send error code to the database server
					buffer[0] = status_byte;
					// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
					sending_cmd = xbee_pack_tx64_frame(UNKNOWN_MSG, buffer, 1, dest_high, dest_low);
					xbee_send(buffer);
					break;
			}//switch(frameBuffer[reply_Id].type)
			buffer_removeData(reply_Id);	//mark cmd as done !
		}//if buffersize
		
		//=========================================================================
		// CPU goes to sleep mode
		//=========================================================================
		// Set the desired sleep mode using set_sleep_mode(). It usually defaults to idle mode where the CPU is put on sleep but all peripheral clocks are still running.
		// This instruction makes the MCU enter Idle mode, stopping the CPU but allowing the SPI, USART, Analog Comparator, ADC, two-wire Serial Interface, Timer/Counters, Watchdog, and the interrupt system to continue operating.
		// This sleep mode basically halts clkCPU and clkFLASH, while allowing the other clocks to run.
		// Idle mode enables the MCU to wake up from external triggered interrupts as well as internal ones like the Timer Overflow and USART Transmit Complete interrupts.
		// If wake-up from the Analog Comparator interrupt is not required, the Analog Comparator can be powered down by setting the ACD bit in the Analog Comparator Control and Status Register Ð ACSR.
		// This will reduce power consumption in Idle mode. If the ADC is enabled, a conversion starts automatically when this mode is entered.
		set_sleep_mode(SLEEP_MODE_IDLE);
		// Then call sleep_mode(). This macro automatically sets the sleep enable bit, goes to sleep, and clears the sleep enable bit.
		sleep_mode();
		
	}	// End of infinite loop
}
