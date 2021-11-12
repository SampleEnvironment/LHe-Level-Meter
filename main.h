// Main.h - Copyright 2016-2019, HZB, ILL/SANE & ISIS

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "config.h"
#include "display_driver.h"
#include "module_globals.h"



extern EWindowOrientation Orientation;
extern EWindowOrientation ili_Orientation;

#ifdef DISP_3000
#include "StringPixelCoordTable.h"
#endif

#ifdef ili9341
#include "StringPixelCoordTable_ili9341.h"
//#include "StringPixelCoordTable.h"
#endif






#define arr_size(x) 			(sizeof(x)/sizeof(x[0]))


// Proportional Factor to calculate the Set_Current (OCR2A = Set_Current * SET_CURRENT_FACTOR + SET_CURRENT_OFFSET)
#define SET_CURRENT_FACTOR		0.8719
#define SET_CURRENT_OFFSET		21.77


// Device position constants
#define END 					0x7E		// '~' Start and end marker for ranges array
#define SEP						0x2F		// '/' Separator marker

#define FILL_RANGES_COUNT 		40
#define FILL_RANGES_LENGTH 		30
#define FILL_LETTERS_LENGTH 	5						//4 chars + \0 !


#define AUTO_FILL_TURNED_ON 	!(PINC & (1<<PC1)) 		// PC1 = VCC means auto-fill OFF
#define START_AUTO_FILL 		PORTB |=(1<<PB0);  // Set Pin B0


#define STOP_AUTO_FILL 			PORTB &=~(1<<PB0); // Clear Pin B0

//#define DO_MEASUREMENT			!(PINA & (1<<PA5))		// Low trigger on PA5

#define SHUTDOWN 				PORTB &=~(1<<PB1); 		// Shutdown - set PB1 at 0


#define MEASBUFFER_LENGTH      25

#define COM_TIMEOUT_TIME 		10						//s
#define WAIT_TIMEOUT_TIME 		500						//ms
#define DISP_TIMEOUT_TIME 		20						//s
#define OPT_TIMEOUT_TIME         60						//s
#define POS_TIMEOUT_TIME        20
#define SAVE_PLUS_PARAMS_TIMEOUT_TIME   20
#define OPT_CHANGED_TIMEOUT_TIME 20
#define SHUTDOWN_TIMEOUT_TIME 20
#define LOGIN_CONNECT_TIMEOUT_TIME 20
#define STOP_FILL_TIMEOUT_TIME 20
#define CALIBRATE_TIMEOUT_TIME 20


//#define ST_OVERFLOW_HRS 		791
//#define ST_OVERFLOW_MIN 		(13.2)
#define OPTIONS_PASSWORD		0
#define NUMBER_OPTIONS_BYTES	37
#define NUMBER_LOGIN_BYTES	    41

// Database server errors
#define HE_LEVEL_ERROR			1111

// XBee network constants
#define TRANSMIT_SLOW_DEF 		5			// Time interval in hour(s) between regular network connections
#define TRANSMIT_SLOW_MIN 		10			// Minimum allowed value for time interval (minutes)
#define TRANSMIT_SLOW_MAX 		24			// Maximum allowed value for time interval (hours)
#define TRANSMIT_FAST_DEF 		1			// Reduced time interval in minute(s) between network connections while filling
#define TRANSMIT_FAST_MIN 		30			// Minimum allowed value for reduced time interval (seconds)
#define TRANSMIT_FAST_MAX 		10			// Maximum allowed value for reduced time interval (minutes)

// Helium probe constants
#define RES_MIN_DEF 			(0)		    // Minimum resistance of He probe (user-defined)
#define RES_MIN_MIN 			(0)			// Minimum allowed value for min resistance of He probe
#define RES_MIN_MAX 			(200)		// Maximum allowed value for min resistance of He probe

#define RES_MAX_DEF 			(100)		// Maximum resistance of He probe (user-defined)
#define RES_MAX_MIN 			(1)	    	// Minimum allowed value for max resistance of He probe
#define RES_MAX_MAX 			(999.9)		// Maximum allowed value for max resistance of He probe

#define QUENCH_TIME_DEF 		(2)			//
#define QUENCH_TIME_MIN 		(0.0)		//
#define QUENCH_TIME_MAX 		(9.9)		//

#define QUENCH_CURRENT_DEF 		(120)		// Current in mA
#define QUENCH_CURRENT_MIN 		(60)		// Minimum allowed value
#define QUENCH_CURRENT_MAX 		(250)		// Maximum allowed value

#define WAIT_TIME_DEF 			(2)			// Waiting time between end of quench and beginning of measurement of He probe (JG)
#define WAIT_TIME_MIN 			(0.1)		// Minimum allowed value
#define WAIT_TIME_MAX 			(30)		// Maximum allowed value

#define MEAS_CURRENT_DEF 		(80)		// Current in mA
#define MEAS_CURRENT_MIN 		(60)		// Minimum allowed value
#define MEAS_CURRENT_MAX 		(150)		// Maximum allowed value

#define TOTAL_VOL_DEF 			(100)		// Total volume in liter
#define TOTAL_VOL_MIN	 		(1)			// Minimum allowed value
#define TOTAL_VOL_MAX 			(1000)		// Maximum allowed value

#define MEASUREMENT_CYCLES_DEF 	20			// Number of measuring cycles for He probe
#define MEASUREMENT_CYCLES_MIN 	1			// Minimum allowed value for number of measuring cycles for He probe
#define MEASUREMENT_CYCLES_MAX 	50			// Maximum allowed value for number of measuring cycles for He probe

// He probe calibration
#define R_SPAN_DEF				1			// ???
#define R_ZERO_DEF				0			//

// Helium filling constants
#define FILLING_TIMEOUT_DEF 	30			// Timeout while filling in minutes
#define MIN_FILLING_TIMEOUT 	10
#define MAX_FILLING_TIMEOUT 	90
#define AUTO_FILL_HE_DEF 		10			// in %
#define MIN_AUTO_FILL_HE 		5
#define MAX_AUTO_FILL_HE 		80
#define FILL_WAITING_LABEL		"Filling time: %d min"

// Pressure constants
#define ZERO_DEF				0			// Used to calculate pressure level (see map_to_pres())
#define MIN_ZERO                (-2000)
#define MAX_ZERO                2000
// #define ZERO_NULLPUNKT			5000		// nur positive zahlen! oder code aendern
#define SPAN_DEF				(1.5)		//
#define MIN_SPAN				0.1
#define	MAX_SPAN				2000

// Battery constants
#define BATT_MIN_DEF			7			// Minimum voltage of battery (user-defined)
#define BATT_MIN_MIN			6			// Minimum allowed value for minimum voltage of battery
#define BATT_MIN_MAX			12			// Maximum allowed value for minimum voltage of battery
#define BATT_MAX_DEF			8.3			// Maximum voltage of battery (user-defined)
#define BATT_MAX_MIN			7			// Minimum allowed value for maximum voltage of battery
#define BATT_MAX_MAX			13			// Maximum allowed value for maximum voltage of battery
#define CRITICAL_BATT_DEF		10			// Critical percentage of battery (user-defined)
#define CRITICAL_BATT_MIN		0
#define CRITICAL_BATT_MAX		100

#define DEV_ID_CHARS_DEF		3
#define DEV_ID_CHARS_MAX        6
#define DEV_ID_CHARS_MIN        3

#define DEVICE_ID_UI_MAX_LEN 3
#define DEVICE_ID_ALPHANUMERIC false
#define MESSAGEFOPRMAT_IDENTIFIER 128	// Identifier used, to differentiate between old and new messageformat

#define OPTIONS_PW_DEF 0
#define OPTIONS_PW_MAX 999
#define OPTIONS_PW_MIN 0

// ILL Display Orientation
#ifdef ILL
#define DISPLAY_CONFIG Orientation = (LVM.options->display_reversed)? Landscape : Landscape180;
#endif // ILL


// HZB Display Orientation
#ifdef HZB
#define DISPLAY_CONFIG Orientation = (!LVM.options->display_reversed)? Landscape : Landscape180;
#endif // HZB


#define EEPROM_CHANGED_DEF 0

// Configuration
#define BUFFER_LENGTH	10        		    // maximum frames
#define DATA_LENGTH		256-16				// maximum data in frame   ATTENTION if changed then change SINGLE_FRAME_LENGTH in xbee.h as well
#define DATA_LENGTH_MEAS 100					// maximum data in frame for level measurement (#3 #6 #7 #8 #10)
#define SINGLE_FRAME_LENGTH 	256		// Full length of one frame  ATTENTION if changed then change DATA_LENGTH in xbee_utilities.h as well
//#define BYTE 			unsigned char

#define DEVICE_ID_STRING_LEN 6

#define ALLOW_COM					// Allow the possibility to receive answer from XBee module
// #define ALLOW_DEBUG
#define ALLOW_DISPLAY_TIMER
#define ALLOW_LOGIN
#define ALLOW_XBEE_SLEEP			// Allow sleeping mode for XBee module
#define ALLOW_EEPROM_SAVING
//#define ALLOW_DOUBLE_CLICK		// Define to allow double click: wake up display and execute the pressed key


//==============================================================
// List of error codes
//==============================================================



#define CHECK_BOUNDS(VAR,MIN,MAX,DEF,FLAG) if((VAR < MIN) || (VAR > MAX || isnan(VAR))){VAR = DEF; FLAG = 1;};

//====================================================================
//			OPTIONS
//====================================================================
typedef struct{
	double r_span;
	double r_zero;
	uint16_t transmit_slow;
	_Bool transmit_slow_min;
	uint16_t transmit_fast;
	_Bool transmit_fast_sec;
	double quench_time;
	double quench_current;
	double wait_time;
	double meas_current;
	uint8_t meas_cycles;
	uint8_t fill_timeout;
	uint8_t he_min;
	double res_min;
	double res_max;
	double span;
	double zero;
	uint8_t enable_pressure;
	double batt_min;
	double batt_max;
	uint8_t critical_batt;
	double total_volume;
	uint8_t display_reversed;
	_Bool Dev_ID_alpahnum;
	uint8_t Dev_ID_Max_len;
	uint16_t	options_pw;
}optionsType;


//====================================================================
//			POSITION
//====================================================================
typedef struct {
	char    Strings[FILL_RANGES_COUNT][FILL_LETTERS_LENGTH];
	uint8_t RangeNums[FILL_RANGES_COUNT][FILL_RANGES_LENGTH];
	uint8_t StrLen[FILL_RANGES_COUNT];
	int8_t  active_Pos_Str;
	int8_t  active_Pos_Str_last;
	int8_t  active_range_Number;
	int8_t  active_range_Number_last;
	_Bool	digit_on;
	_Bool	digit_on_last;
	_Bool	letters_and_numbers ;
	
}posType;



//====================================================================
//			Message from Server
//====================================================================
typedef struct
{
	char Str[19];
	uint8_t type;
	_Bool Received;
}LCD_MessageType;





//====================================================================
//			unsent MEASUREMENT
//====================================================================
// Buffer for old measurements that could not be transmitted, see also typedef "frame" in xbee_utilities
typedef struct {
	uint8_t type;
	uint8_t data_len;
	uint8_t data[DATA_LENGTH_MEAS];
} savedMeasType;

//====================================================================
//			SAVED MEASUREMENTS RINGBUFFER
//====================================================================
typedef struct {
	savedMeasType measurements[MEASBUFFER_LENGTH];
	uint8_t firstMeas;     // index of the first entry
	uint8_t nextfreeEntry;  // index of the next free entry
	uint8_t numberStored;	// number of stored measurements
	
}MeasBufferType;

//====================================================================
//			TEMPORARY BUFFERS
//====================================================================
typedef struct {
	char 		string[SINGLE_FRAME_LENGTH];
	uint8_t 	buffer[SINGLE_FRAME_LENGTH];
	uint8_t		databuffer[DATA_LENGTH_MEAS];
	char		infostr[30];

}Temp_buffersType;


//====================================================================
//			VARS
//====================================================================
typedef struct{
	double		r_val_last_Meas;
	
	double 		he_level;						// Init and clear Helium level
	double		last_he_level;
	double	 	batt_level;						// Init and clear battery level
	double	 	pressure_level;					// Init and clear pressure value

	uint16_t    eeprom_changed;

	_Bool		very_low_He_level;			// Flag for very low Helium level (< level min)
	_Bool 		auto_fill_enabled;
	_Bool 		auto_fill_started;
	_Bool 		transmit_slow_changed;				// Set true if time interval between regular network connections has been changed
	_Bool		transmit_fast_changed;

	// Device position (instruments or labs)
	char 		device_pos[5];


	//diag pages

	

	uint16_t	entered_options_pw;
	uint8_t     fill_meas_counter;
	
	char  Device_ID_Str[DEVICE_ID_STRING_LEN+1];
	
	
	
}VarsType;



typedef struct
{
    optionsType  eeOptions;
	uint16_t     eeprom_changed;
}eememType;



typedef struct{
	VarsType         *const vars;
	optionsType      *const options;
	posType          *const pos;
	LCD_MessageType  *const message;
	MeasBufferType   *const measbuff;
	Temp_buffersType *const temp;
	VersionType      *const version;
	eememType        *const eeprom;
}LVM_ModelType;






// diag pulse Model
typedef struct
{
	
	uint32_t elapsed_t; //elapsed time  in timer overflows, --> conversion is needed to get ms
	uint32_t elapsed_t_last;
	
	uint32_t delta_t_timer_steps;
	
	uint32_t t_end_quench;
	uint32_t t_end_wait;
	uint32_t t_end_meas;
	
	uint8_t  meastime_factor;
	
	_Bool draw_all;
	_Bool headless;
	_Bool quench_on;
	
	uint16_t * top_unit;
	
	double heat_time;
	double quench_current;
	double meas_current;
	double wait_time;
	double meas_time;
	double r_span;
	double r_zero;
	
	
	
	
	int16_t active_point;
	
	
	uint16_t u[DP_NUMBER_OF_POINTS_140];
	uint16_t i[DP_NUMBER_OF_POINTS_140];
	uint16_t r[DP_NUMBER_OF_POINTS_140];
	
	uint16_t u_point[DP_NUMBER_OF_POINTS_140];
	uint16_t i_point[DP_NUMBER_OF_POINTS_140];
	uint16_t r_point[DP_NUMBER_OF_POINTS_140];
	uint16_t color[DP_NUMBER_OF_POINTS_140];
	
	uint8_t  y_maxpixels;
	uint16_t cursor_len;
	uint8_t  x_fact;
	uint16_t quench_end_position;
	
	double   delta_t_points;

	
	uint16_t   top_zero;
	uint16_t   bot_zero;
	
	uint16_t   u_max;
	uint16_t   i_max;
	uint16_t  r_max;
	

	double u_avg;
	double i_avg;
	uint16_t n_for_avg;
	
	double I_increment;
	
	
	
	double diag_res;
	
	uint8_t pulse_type;
	uint16_t points_in_plot;
	uint16_t halfway_point;
	
	
	
}diag_pulseType;




#define DRAW_U 1
#define DRAW_R 2



//#define CHECK_ERROR(LETTERS_ERROR) 			(status_byte & (1<<6))





//==============================================================
// Add the following commands
//==============================================================
//_Bool auto_fill_pin_on(void);	// Check auto fill pin

void diag_pulse_move_cursor(diag_pulseType *dp,int8_t direction);
//int main(void);
int I2C_ClearBus(void);
uint8_t collect_and_send_MeasData(uint8_t *meas_buffer,uint8_t Message_Code);
_Bool auto_fill_pin_on(void);
void write_opts_to_EEPROM(void);


#endif  // main.h
