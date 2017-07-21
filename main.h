// Main.h - Copyright 2016, HZB, ILL/SANE & ISIS

#ifndef MAIN_H
#define MAIN_H

#define arr_size(x) 			(sizeof(x)/sizeof(x[0]))

// Device position constants
#define END 					0x7E		// '~' Start and end marker for ranges array
#define SEP						0x2F		// '/' Separator marker

#define FILL_RANGES_COUNT 		15
#define FILL_RANGES_LENGTH 		21
#define FILL_LETTERS_LENGTH 	4						//3 chars + \0 !


#define AUTO_FILL_TURNED_ON 	!(PINC & (1<<PC1)) 		// PC1 = VCC means auto-fill OFF
#define START_AUTO_FILL 		PORTB |=(1<<PB0);		// Set Pin B0
#define STOP_AUTO_FILL 			PORTB &=~(1<<PB0);		// Clear Pin B0

#define DO_MEASUREMENT			!(PINA & (1<<PA5))		// Low trigger on PA5

#define SHUTDOWN 				PORTB &=~(1<<PB1); 		// Shutdown - set PB1 at 0



#define COM_TIMEOUT_TIME 		25						//s
#define WAIT_TIMEOUT_TIME 		500						//ms
#define DISP_TIMEOUT_TIME 		20						//s

//#define ST_OVERFLOW_HRS 		791
//#define ST_OVERFLOW_MIN 		(13.2)
#define OPTIONS_PASSWORD		1
#define NUMBER_OPTIONS_BYTES	29

// Database server errors
#define HE_LEVEL_ERROR			1111

// XBee network constants
#define TRANSMIT_SLOW_DEF 		5			// Time interval in hour(s) between regular network connections
#define TRANSMIT_SLOW_MIN 		1			// Minimum allowed value for time interval
#define TRANSMIT_SLOW_MAX 		24			// Maximum allowed value for time interval
#define TRANSMIT_FAST_DEF 		1			// Reduced time interval in minute(s) between network connections while filling
#define TRANSMIT_FAST_MIN 		1			// Minimum allowed value for reduced time interval
#define TRANSMIT_FAST_MAX 		10			// Maximum allowed value for reduced time interval

// Helium probe constants
//#define RES_MIN_DEF 			(2.1)		// Minimum resistance of He probe (user-defined)
#define RES_MIN_DEF 			(63)		// Minimum resistance of He probe (user-defined) for ILL
#define RES_MIN_MIN 			(10)		// Minimum allowed value for min resistance of He probe
//#define RES_MIN_MAX 			(9.9)		// Maximum allowed value for min resistance of He probe
#define RES_MIN_MAX 			(160)		// Maximum allowed value for min resistance of He probe for ILL

//#define RES_MAX_DEF 			(350.1)		// Maximum resistance of He probe (user-defined)
#define RES_MAX_DEF 			(163)		// Maximum resistance of He probe (user-defined) for ILL
#define RES_MAX_MIN 			(0.1)		// Minimum allowed value for max resistance of He probe
#define RES_MAX_MAX 			(999.9)		// Maximum allowed value for max resistance of He probe

#define QUENCH_TIME_DEF 		(4)			// 
#define QUENCH_TIME_MIN 		(0.1)		//
#define QUENCH_TIME_MAX 		(9.9)		//

#define QUENCH_CURRENT_DEF 		(150)		// Current in mA
#define QUENCH_CURRENT_MIN 		(120)		// Minimum allowed value
#define QUENCH_CURRENT_MAX 		(160)		// Maximum allowed value

#define WAIT_TIME_DEF 			(3)			// Waiting time between end of quench and beginning of measurement of He probe (JG)
#define WAIT_TIME_MIN 			(0.1)		// Minimum allowed value
#define WAIT_TIME_MAX 			(9.9)		// Maximum allowed value

#define MEAS_CURRENT_DEF 		(80)		// Current in mA
#define MEAS_CURRENT_MIN 		(60)		// Minimum allowed value
#define MEAS_CURRENT_MAX 		(150)		// Maximum allowed value

#define TOTAL_VOL_DEF 			(250)		// Total volume in liter
#define TOTAL_VOL_MIN	 		(1)			// Minimum allowed value
#define TOTAL_VOL_MAX 			(150)		// Maximum allowed value

#define MEASUREMENT_CYCLES_DEF 	20			// Number of measuring cycles for He probe
#define MEASUREMENT_CYCLES_MIN 	1			// Minimum allowed value for number of measuring cycles for He probe
#define MEASUREMENT_CYCLES_MAX 	50			// Maximum allowed value for number of measuring cycles for He probe

// He probe calibration
#define R_SPAN_DEF				350			// 
#define R_ZERO_DEF				0			// 

// Helium filling constants
#define FILLING_TIMEOUT_DEF 	30			// Timeout while filling
#define MIN_FILLING_TIMEOUT 	(2*TRANSMIT_FAST_MIN)
#define MAX_FILLING_TIMEOUT 	30
#define AUTO_FILL_HE_DEF 		10
#define MIN_AUTO_FILL_HE 		0
#define MAX_AUTO_FILL_HE 		80
#define FILL_WAITING_LABEL		"time left: %d min"

// Pressure constants
#define ZERO_DEF				0			// Used to calculate pressure level (see map_to_pres())
#define MIN_ZERO                (-2000)
#define MAX_ZERO                2000
#define ZERO_NULLPUNKT			5000		// nur positive zahlen! oder code aendern
#define SPAN_DEF				(1.5)		// 
#define MIN_SPAN				1
#define	MAX_SPAN				2000

// Battery constants
#define BATT_MIN_DEF			10			// Minimum voltage of battery (user-defined)
#define BATT_MIN_MIN			0			// Minimum allowed value for minimum voltage of battery
#define BATT_MIN_MAX			12			// Maximum allowed value for minimum voltage of battery
#define BATT_MAX_DEF			(12.61)		// Maximum voltage of battery (user-defined)
#define BATT_MAX_MIN			12			// Minimum allowed value for maximum voltage of battery
#define BATT_MAX_MAX			13			// Maximum allowed value for maximum voltage of battery
#define CRITICAL_BATT_DEF		20			// Critical percentage of battery (user-defined)
//#define CRITICAL_BATT_MIN		0
//#define CRITICAL_BATT_MAX		100





#define ALLOW_COM					// Allow the possibility to receive answer from XBee module
//#define ALLOW_DEBUG
#define ALLOW_DISPLAY_TIMER
#define ALLOW_LOGIN
#define ALLOW_XBEE_SLEEP			// Allow sleeping mode for XBee module
#define ALLOW_EEPROM_SAVING
//#define ALLOW_DOUBLE_CLICK		// Define to allow double click: wake up display and execute the pressed key



#define  ex_mode_main_window		1
#define  ex_mode_fill_action		2
#define  ex_mode_measure_action		4
#define  ex_mode_options_window		8
#define  ex_mode_shutdown_action	16
#define  ex_mode_diagnotic_action	32
#define  ex_mode_offline			64
#define  ex_mode_error				128
#define  ex_mode_calibration		256


#define ex_errorCode_NoError		0
#define ex_errorCode_LoginFailed	1100
#define ex_errorCode_Shutdown		1200
#define ex_errorCode_Offline		1300



extern volatile uint8_t cmd_line;					// RS232 command ready for processing indicator
extern volatile uint8_t *send_str_reader;			// Pointer to the next byte to send via USART
extern volatile uint8_t sending_cmd;				// Number of bytes to send via USART

// Variables for keyboard
//extern volatile uint8_t pressed_key;			// Pressed key, checked in PCI_int
//extern volatile uint8_t execute_pressed_key;	// Remember to execute the pressed key

//==============================================================
// List of error codes
//==============================================================

#define errCode_TooHighRes			2000



//==============================================================
// Device status
//==============================================================
extern volatile uint8_t status_byte;

#define SET_NETWORK_ERROR 				status_byte|=(1<<0);
#define SET_NO_REPLY_ERROR 				status_byte|=(1<<1);
#define SET_STARTED_FILLING_ERROR 		status_byte|=(1<<2);
#define SET_STOPED_FILLING_ERROR 		status_byte|=(1<<3);
#define SET_CHANGED_OPTIONS_ERROR 		status_byte|=(1<<4);
#define SET_SLOW_TRANSMISSION_ERROR 	status_byte|=(1<<5);

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


//==============================================================
// Add the following commands
//==============================================================
//_Bool auto_fill_pin_on(void);	// Check auto fill pin
//void diag_pulse(double *heat_time, double r_span, double r_zero);
//void diag_page1(double r_zero, double r_span, double batt_min, double batt_max, double res_min, double res_max, double zero, double span);
//void diag_page2(double r_zero);

//int main(void);


#endif  // main.h
