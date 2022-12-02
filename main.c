// Main.c - Copyright 2011-2019, HZB, ILL/SANE & ISIS
#define RELEASE_MAIN 2.00

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016
// 2.00 - Second release January 2018

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

#include "main.h"

#include "adwandler.h"
#include "disp/display_lib.h"

#include "display_utilities.h"
#include "keyboard.h"
#include "timer_utilities.h"

#include "HoneywellSSC.h"
#include "Controller/option_mode.h"
#include "Controller/filling_mode.h"
#include "Controller/error_mode.h"
#include "Controller/diagnostic_mode.h"
#include "Controller/main_mode.h"
#include "Controller/getCode_mode.h"
#include "Controller/pulse_select_mode.h"


#include "diag_pulse.h"
#include "i2cmaster.h"
#include "usart.h"
#include "xbee.h"
#include "xbee_utilities.h"
#include "xbee_AT_comm.h"
#include "DS3231M.h"
#include "I2C_utilities.h"
#include "module_globals.h"
#include "status.h"



#ifdef DISP_3000
#include "StringPixelCoordTable.h"
#endif
#ifdef ili9341
#include "StringPixelCoordTable_ili9341.h"
//#include "StringPixelCoordTable.h"
#endif





/**
* @brief Ringbuffer for unsent Measurements
*
*	Ringbuffer containing all Messages that could not be sent due to a Network error.
*
*/

MeasBufferType measbuff = {
	.firstMeas = 0,
	.nextfreeEntry = 0,
	.numberStored = 0
};

//EEPROM vars
#ifdef ALLOW_EEPROM_SAVING
//control
//calibration

eememType  EEMEM eeVars = {
	.eeOptions.r_span = R_SPAN_DEF,
	.eeOptions.r_zero = R_ZERO_DEF,
	.eeOptions.transmit_slow = TRANSMIT_SLOW_DEF,		// Define time interval between regular network connections (5 hours as default)
	.eeOptions.transmit_slow_min = false,					// Time interval between regular network connections defined in minutes
	.eeOptions.transmit_fast = TRANSMIT_FAST_DEF,			// Define reduced time interval when Helium is filling (1 minute as default)
	.eeOptions.transmit_fast_sec = false,					// Reduced time interval between network s defined in seconds
	.eeOptions.quench_time = QUENCH_TIME_DEF,				// Quench duration in seconds
	.eeOptions.quench_current = QUENCH_CURRENT_DEF,		// Quench current in mA
	.eeOptions.wait_time = WAIT_TIME_DEF,					// Stabilization duration in seconds
	.eeOptions.meas_current = MEAS_CURRENT_DEF,			// Measurement current in mA
	.eeOptions.meas_cycles = MEASUREMENT_CYCLES_DEF,
	.eeOptions.fill_timeout = FILLING_TIMEOUT_DEF,
	.eeOptions.he_min = AUTO_FILL_HE_DEF,
	.eeOptions.res_min = RES_MIN_DEF,
	.eeOptions.res_max = RES_MAX_DEF,
	.eeOptions.span = SPAN_DEF,
	.eeOptions.zero = ZERO_DEF,
	.eeOptions.enable_pressure = false,
	.eeOptions.batt_min = BATT_MIN_DEF,
	.eeOptions.batt_max = BATT_MAX_DEF,
	.eeOptions.critical_batt = CRITICAL_BATT_DEF,
	.eeOptions.total_volume = TOTAL_VOL_DEF,
	.eeOptions.display_reversed = false,
	.eeOptions.Dev_ID_alpahnum = DEVICE_ID_ALPHANUMERIC ,
	.eeOptions.Dev_ID_Max_len = DEVICE_ID_UI_MAX_LEN,
	.eeOptions.SC_mask = SC_MASK_DEFAULT,
	.eeOptions.SC_mask_alerady_received = 0,
	.eeprom_changed = EEPROM_CHANGED_DEF
	
};

#endif

LCD_MessageType TextMessage = {
	.Str = "",
	.type = 1,
	.Received = 0
};


optionsType Options ={
	.r_span = R_SPAN_DEF,
	.r_zero = R_ZERO_DEF,
	.transmit_slow = TRANSMIT_SLOW_DEF,		// Define time interval between regular network connections (5 hours as default)
	.transmit_slow_min = false,					// Time interval between regular network connections defined in minutes
	.transmit_fast = TRANSMIT_FAST_DEF,			// Define reduced time interval when Helium is filling (1 minute as default)
	.transmit_fast_sec = false,					// Reduced time interval between network s defined in seconds
	.quench_time = QUENCH_TIME_DEF,				// Quench duration in seconds
	.quench_current = QUENCH_CURRENT_DEF,		// Quench current in mA
	.wait_time = WAIT_TIME_DEF,					// Stabilization duration in seconds
	.meas_current = MEAS_CURRENT_DEF,			// Measurement current in mA
	.meas_cycles = MEASUREMENT_CYCLES_DEF,
	.fill_timeout = FILLING_TIMEOUT_DEF,
	.he_min = AUTO_FILL_HE_DEF,
	.res_min = RES_MIN_DEF,
	.res_max = RES_MAX_DEF,
	.span = SPAN_DEF,
	.zero = ZERO_DEF,
	.enable_pressure = false,
	.batt_min = BATT_MIN_DEF,
	.batt_max = BATT_MAX_DEF,
	.critical_batt = CRITICAL_BATT_DEF,
	.total_volume = TOTAL_VOL_DEF,
	.display_reversed = false,
	.Dev_ID_alpahnum = DEVICE_ID_ALPHANUMERIC ,
	.Dev_ID_Max_len = DEVICE_ID_UI_MAX_LEN,
	.SC_mask = SC_MASK_DEFAULT,
	.SC_mask_alerady_received = 0
};



// Default positions, examples
posType PosModel = {
	.Strings ={"\r","none", "\r"},
	.RangeNums = {{END},{END,1,END}, {END}},
	.StrLen={0,0,0},
	.active_Pos_Str = 1,
	.active_range_Number=1,
	.letters_and_numbers = false,
	.digit_on = false,
};



Temp_buffersType temp;









VarsType vars = {
	.r_val_last_Meas = 0,

	.he_level = 0,
	.last_he_level = 200,
	.batt_level = 0,
	.pressure_level = 0,
	.eeprom_changed = 0,
	.very_low_He_level = 0,
	.auto_fill_enabled = 0,
	.auto_fill_started = 0,
	.device_pos ="none",
	.entered_options_pw = 0,
	.transmit_fast_changed = 0,
	.transmit_slow_changed = 0,
	.fill_meas_counter =0,
	.Device_ID_Str="",
	.n_pulse_wakes = 0
};



//==================================================================
//Filling related
//========================================================






LVM_ModelType LVM ={
	.vars =     &vars,
	.options =  &Options,
	.pos =      &PosModel,
	.message =  &TextMessage,
	.measbuff = &measbuff,
	.temp =     &temp,
	.version =  &version,
	.eeprom = & eeVars
};



// store undelivered measurements to the measurement buffer
void store_measurement(uint8_t db_cmd_type, uint8_t *buffer, uint8_t length)
{


	// Variables
	char infostr[20];

	// write information to measurement buffer
	LVM.measbuff->measurements[LVM.measbuff->nextfreeEntry].type  = db_cmd_type;
	//	measBuffer[nextfreeMeasBuff].data  = buffer;

	//	for(uint8_t i = 0; i<length;i++) measBuffer[nextfreeMeasBuff].data[i] = buffer[i];
	memcpy(LVM.measbuff->measurements[LVM.measbuff->nextfreeEntry].data,buffer,length);
	LVM.measbuff->measurements[LVM.measbuff->nextfreeEntry].data_len = length;

	// increase the number of stored measurements
	LVM.measbuff->numberStored = LVM.measbuff->numberStored + 1;
	if (LVM.measbuff->numberStored > MEASBUFFER_LENGTH) LVM.measbuff->numberStored = MEASBUFFER_LENGTH;

	// adapt the index for the next measurement to be stored
	LVM.measbuff->nextfreeEntry = LVM.measbuff->nextfreeEntry + 1;
	if (LVM.measbuff->nextfreeEntry > MEASBUFFER_LENGTH-1) LVM.measbuff->nextfreeEntry = 0;

	// if the number of stored measurements reaches the maximum number, the oldest measurements will be overwritten
	if (LVM.measbuff->numberStored == MEASBUFFER_LENGTH) LVM.measbuff->firstMeas = LVM.measbuff->nextfreeEntry;

	/*	#ifdef ALLOW_DEBUG
	LCD_Cls(BGC);
	LCD_Print("storing...  ", 5, 20, 2, 1, 1, FGC, BGC);
	draw_int(MEASBUFFER_LENGTH, 5, 40, " max", FGC);
	draw_int(numberMeasBuff, 5, 60, " nr", FGC);
	draw_int(firstMeasBuff, 5, 80, " first", FGC);
	draw_int(nextfreeMeasBuff, 5, 100, " next", FGC);
	delay_ms(2000);
	#endif
	*/

	sprintf(infostr,STR_DATA_STORED_N,LVM.measbuff->numberStored);
	paint_info_line(infostr,0);
	_delay_ms(1000);

}





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




uint8_t collect_and_send_MeasData(uint8_t *meas_buffer,uint8_t Message_Code){
	// enter time
	if (connected.DS3231M)
	{
		DS3231M_read_time();

	}
	else
	{
		Time.tm_sec = 0;
		Time.tm_min = 0;
		Time.tm_mday  = 0;
		Time.tm_mon  = 0;
		Time.tm_year   = 0;
	}

	// enter pressure
	LVM.vars->pressure_level = 0;
	if (HoneywellSSC_status.connected)
	{
		HoneywellSSC_read_pressure();
		if (HoneywellSSC_status.status < 4) LVM.vars->pressure_level = HoneywellSSC_Pressure;
	}


	uint8_t index =   0;



	// enter time
	meas_buffer[index++] = Time.tm_sec;   //0
	meas_buffer[index++] = Time.tm_min;   //1
	meas_buffer[index++] = Time.tm_hour;  //2
	meas_buffer[index++] = Time.tm_mday;  //3
	meas_buffer[index++] = Time.tm_mon;   //4
	meas_buffer[index++] = Time.tm_year;  //5


	if(LVM.vars->he_level < 0)	// If negative He level, send error code
	{
		meas_buffer[index++] = HE_LEVEL_ERROR>>8;
		meas_buffer[index++] = (uint8_t) HE_LEVEL_ERROR;
	}
	else
	{
		meas_buffer[index++] = ((uint16_t)(LVM.vars->he_level*10))>>8;  //6
		meas_buffer[index++] = ((uint16_t)(LVM.vars->he_level*10));     //7
	}

	meas_buffer[index++] = LVM.vars->batt_level;           //8

	meas_buffer[index++] = (uint16_t) LVM.vars->pressure_level >> 8;  //9
	meas_buffer[index++] = (uint16_t) LVM.vars->pressure_level;       //10

	// enter temperature
	if (connected.DS3231M)
	{
		DS3231M_read_temperature();

		/*					// Message on screen
		LCD_Cls(BGC);
		LCD_Print("temperature", 5, 20, 2, 1, 1, FGC, BGC);
		draw_int(temp_MSB, 5, 40, "", ERR);
		draw_int(temp_LSB, 5, 60, "", ERR);
		draw_double(DS3231M_Temperature, 5, 80, 2, "K", ERR);
		_delay_ms(2000);
		*/

		meas_buffer[index++] = (uint16_t) (DS3231M_Temperature * 100) >> 8;  //11
		meas_buffer[index++] = (uint16_t) (DS3231M_Temperature * 100);       //12
	}
	else
	{
		meas_buffer[index++] = 0;
		meas_buffer[index++] = 0;




	}

	// position
	index =  devicePos_to_buffer(LVM.vars->device_pos, index, LVM.temp->buffer);  // Positions are 4 letters  13..16

	if (LVM.vars->r_val_last_Meas > 650){
		LVM.vars->r_val_last_Meas = 650;
	}

		meas_buffer[index++] = ((uint16_t)(LVM.vars->r_val_last_Meas*10))>>8;  //18
		meas_buffer[index++] = ((uint16_t)(LVM.vars->r_val_last_Meas*10));     //19

		meas_buffer[index++] =  get_status_byte_levelmeter(); //17
		








	memcpy(LVM.temp->databuffer,meas_buffer,index);


	if(Message_Code == TRIGGER_MEAS_CMD){
		// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
		if (xbee_send_message(Message_Code, meas_buffer, index))
		{
			CLEAR_ALL(); // Clear all errors
		}

	}
	else
	{
		#ifdef ALLOW_COM
		if (xbee_send_request(Message_Code, meas_buffer, index) == 0xFF) // no conn to Helium Management
		{
			LVM.temp->databuffer[index - 1] = 0;  // delete status_byte before storing
			if ((Message_Code != LOGOUT_MSG)||(Message_Code != FILLING_BEGIN_MSG) )
			{
				store_measurement(Message_Code,LVM.temp->databuffer, index);
			}
			return 1;
		}

		#else
		// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
		if (xbee_send_message(Message_Code, meas_buffer, index))
		{
			CLEAR_ALL(); // Clear all errors
		}
		#endif
	}

	return 0;
}

//=========================================================================
// Login transaction
//=========================================================================

uint8_t xbee_send_login_msg(uint8_t db_cmd_type, uint8_t *buffer)
{
	#ifdef ALLOW_LOGIN
	uint8_t reply_Id = 0;

	xbee_wake_up_plus();

	InitScreen_AddLine(STR_CONNECTING_TO_SERVER,1);



	// Try to send login message "number_trials" times
	uint8_t number_trials = 2;

	while(number_trials)
	{
		reply_Id = xbee_send_request(db_cmd_type, buffer, 0);
		
		if(reply_Id != 0xFF)
		{
			//				sprintf(ltemp, "...data received: %i", frameBuffer[reply_Id].data_len);
			InitScreen_AddLine(STR_DATE_RECEIVED,0);
			memcpy(LVM.temp->buffer,(uint8_t *)frameBuffer[reply_Id].data,NUMBER_LOGIN_BYTES);

			if(frameBuffer[reply_Id].data_len == NUMBER_LOGIN_BYTES) 	return 0;	//good options
			else {  switch (frameBuffer[reply_Id].data[0])
				{
					case 0x56: return 2; break; //message 0x56="V" vessel parameters not valid
					case 0x57: return 4; break; //message 0x56="W" vessel not on campus
					case 0x53: return 5; break; //message 0x53="S" vessel already connected
					case 0x54: return 6; break; //message 0x53="T" vessel unknown, not in data base
					case 0x4D: return 7; break; //message 0x4D="M" module not known
					default: return 1;   break; //bad options
				}
				//						if(frameBuffer[reply_Id].data[0] == 0x56)	return 2;	//message 0x56="V" vessel unknown
				//						else										return 1;	//bad options
			}
		}


		if(!(--number_trials))
		{
			//stop trying and go in error mode; no functionality available from here on
			xbee_sleep(); // Stop XBee module
			return 0xFF;
		}
		InitScreen_AddLine(STR_LOGIN_FAILED,1);
		InitScreen_AddLine(STR_PRESS_KEY_TO_TRY_AGAIN,0);

		// waiting for the user to press a key and giving the
		// possibility to go to OFFLINE MODE if there is no connection
		// shutdown after number_trials (in main.c)

		// set timer until shutdown
		set_timeout(0, TIMER_0, RESET_TIMER);
		set_timeout(10000, TIMER_0, USE_TIMER);
		//			LCD_Print("timer was set", 5,80,2,1,1, FGC, BGC);
		// if key is pressed retry to connect or goto OFFLINE mode
		while (set_timeout(0, TIMER_0, USE_TIMER))
		{
			//				LCD_Print("timer running", 5, 20, 2, 1, 1, FGC, BGC);
			if (keyhit_cont() != 0) // key pressed
			{
				break;
			}
		}

		// if timer is not completely run down check if key pressed is HIDDEN_FUNCTION then goto OFFLINE mode
		// or in the other case try to reconnect
		if(set_timeout(0,TIMER_0, USE_TIMER))
		{
			//			    LCD_Print("timer running 2", 5, 20, 2, 1, 1, FGC, BGC);
			if(keyhit_cont() == HIDDEN_FUNCTION)
			{
				InitScreen_AddLine(STR_OFFLINE_MODE,1);
				_delay_ms(2000);
				return 3;
			}
		}

	}
	return 0xFF;
	#else
	return 1;	//bad options /main will set default
	#endif
}


inline void xbee_wake_up_plus(void)
{
	if (xbee.sleeping)
	{
		xbee_wake_up();
		
	}

	// Clear then set the timeout for awake time
	set_timeout(0, TIMER_5, RESET_TIMER);
	set_timeout(xbee.awake_period, TIMER_5, USE_TIMER);		// Stay active for xbee_awake_period

	xbee.sleeping = false;									// Xbee module is not sleeping anymore

}





inline void xbee_sleep_plus(void)
{
	xbee_sleep();
	xbee.sleeping = true;
	// Clear then set the timeout for sleeping time
	set_timeout(0, TIMER_5, RESET_TIMER);
	
	if (LVM.vars->n_pulse_wakes)
	{
		set_timeout(POST_PULSE_SLEEP_TIME, TIMER_5, USE_TIMER);
		--LVM.vars->n_pulse_wakes;
	}else{
		set_timeout(xbee.sleep_period*60, TIMER_5, USE_TIMER);
	}
	
	
}





void write_opts_to_EEPROM(void){
	
	optionsType OptBuff;
	
	memcpy(&OptBuff,LVM.options,sizeof(optionsType));
	
	
	//transmit_ slow is saved in minutes in eeprom
	if(!OptBuff.transmit_slow_min){
		OptBuff.transmit_slow *= 60;
	}
	
	//transmit_ slow is saved in seconds in eeprom
	if(!OptBuff.transmit_fast_sec){
		OptBuff.transmit_fast *= 60;
	}
	
	eeprom_update_block(LVM.options,&LVM.eeprom->eeOptions,sizeof(optionsType));

}

void write_DEFS_to_EEPROM(void){
	
	optionsType OptBuff = {
		.r_zero = R_ZERO_DEF,
		.r_span = R_SPAN_DEF,
		.transmit_slow = TRANSMIT_SLOW_DEF,
		.transmit_fast = TRANSMIT_FAST_DEF,
		.transmit_slow_min = false,
		.transmit_fast_sec = false,
		.quench_time       = QUENCH_TIME_DEF,
		.quench_current    = QUENCH_CURRENT_DEF,
		.wait_time         = WAIT_TIME_DEF,
		.meas_current      = MEAS_CURRENT_DEF,
		.meas_cycles       = MEASUREMENT_CYCLES_DEF,
		.fill_timeout      = FILLING_TIMEOUT_DEF,
		.he_min            = AUTO_FILL_HE_DEF,
		.res_min           = RES_MIN_DEF,
		.res_max           = RES_MAX_DEF,
		.span              = SPAN_DEF,
		.zero              = ZERO_DEF,
		.enable_pressure   = 0,
		.batt_min          = BATT_MIN_DEF,
		.batt_max          = BATT_MAX_DEF,
		.critical_batt     = CRITICAL_BATT_DEF,
		.display_reversed  = false,
		.Dev_ID_alpahnum   = false,
		.Dev_ID_Max_len    = DEV_ID_CHARS_DEF,
		.options_pw        =  OPTIONS_PW_DEF,
		.SC_mask = SC_MASK_DEFAULT,
		.SC_mask_alerady_received = 0		
		
	};
	
	eeprom_update_block(&OptBuff,&LVM.eeprom->eeOptions,sizeof(optionsType));
	

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



	// Measurement pin (PORTC.0)
	DDRC |= (1<<DDC0);			// Set Pin C0 as output
	MEASURE_PIN_OFF				// Open the relay K1



	// Display light control pin
	DDRB |= (1<<DDB3);			// Set Pin B3 as output
	#ifdef DISP_3000
	DISPLAY_TURN_ON				// Turn on the display
	#endif





	// Auto fill pin
	DDRC &= ~(1<<DDC1);			// Set Pin C1 as input
	PORTC |= (1<<PC1);			// Activate pull-up

	// Start_Auto_fill pin
	DDRB |= (1<<DDB0);			// Set Pin B0 as output
	PORTB &=~(1<<PB0);			// Clear Pin B0 // Added by JG on March 2017



	//=========================================================================
	// Initialization
	//=========================================================================
	#ifdef ALLOW_EEPROM_SAVING
	LVM.options->display_reversed 	= eeprom_read_byte(&eeVars.eeOptions.display_reversed);
	uint8_t SC_already_received 	= eeprom_read_byte(&eeVars.eeOptions.SC_mask_alerady_received);
	uint8_t SC_EEPROM				= eeprom_read_word(&eeVars.eeOptions.SC_mask);

	#endif

	version_INIT(FIRMWARE_VERSION,BRANCH_ID,LAST_FIRMWARE_EEPROM_CHANGED);
	
	if (SC_already_received == SC_already_received_Pattern)
	{
		xbee_init(&paint_info_line,LVM.vars->Device_ID_Str,DEV_ID_CHARS_MAX,SC_EEPROM);
		LVM.options->SC_mask = SC_EEPROM;
	}else
	{
		xbee_init(&paint_info_line,LVM.vars->Device_ID_Str,DEV_ID_CHARS_MAX,SC_MASK_DEFAULT);
		LVM.options->SC_mask = SC_MASK_DEFAULT;
	}


	LCD_Init();
	



	// XBee module (PORTA.6)
	DDRA |= (1<<DDA6);			// Set Pin A6 as output
	xbee_wake_up();				// Switch on Zigbee module

	//=========================================================================
	// Display Orientation;
	//=========================================================================
	DISPLAY_CONFIG
	xoff = (!LVM.options->display_reversed)? 0 : XOffset;


	#ifdef ili9341
	//_delay_ms(1000);
	LCD_Cls(white);

	LCD_LOGO(15,60,white);

	sprintf(LVM.temp->string,"%s ", STR_HZB_LEVELMETER);
	glcd_led_on();
	_delay_ms(1000);
	LCD_Print(LVM.temp->string,20,160,2,1,1,BGC,white);
	sprintf(LVM.temp->string,"%s %iv%i", STR_FIRMWARE_VERSION,LVM.version->Branch_id,LVM.version->Fw_version);
	LCD_Print(LVM.temp->string,20,185,2,1,1,BGC,white);
	_delay_ms(2000);
	#endif

	#ifdef DISP_3000
	LCD_Cls(white);

	LCD_LOGO(5,20,white);

	sprintf(LVM.temp->string,"%s ", STR_HZB_LEVELMETER);
	_delay_ms(1000);
	LCD_Print(LVM.temp->string,10,90,2,1,1,BGC,white);

	sprintf(LVM.temp->string,"Firmware %iv%i",LVM.version->Branch_id,LVM.version->Fw_version);
	LCD_Print(LVM.temp->string,10,105,2,1,1,BGC,white);
	_delay_ms(2000);
	#endif // DISP_3000



	


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
	init_2_timer8();				// Initialize Timer2 (8-bit)
	i2c_init();
	OptentrysInit();


	

	Controller_Model * const main_model= (Controller_Model*) get_main_model();
	Controller_Model * const diag_model= (Controller_Model*) get_diag_model();
	Controller_Model * const error_model= (Controller_Model*) get_error_model();
	Controller_Model * const filling_model= (Controller_Model*) get_fill_model();
	Controller_Model * const option_model= (Controller_Model*) get_option_model();
	Controller_Model * const pselect_model= (Controller_Model*) get_pulse_select_model();
	
	Controller_Model * Model = main_model;
	
	//write_DEFS_to_EEPROM();

	#ifdef ALLOW_EEPROM_SAVING

	
	eeprom_read_block(LVM.options,&LVM.eeprom->eeOptions,sizeof(optionsType));
	
	Options_Buonds_Check(LVM.options);
	
	LVM.options->transmit_slow_min = true;
	LVM.options->transmit_fast_sec = true;
	
	if(LVM.options->transmit_slow > 60)
	{
		LVM.options->transmit_slow_min = false;
		LVM.options->transmit_slow /=60;
	}
	
	if (LVM.options->transmit_fast > 60)
	{
		LVM.options->transmit_fast_sec = false;
		LVM.options->transmit_fast /=60;
	}
	
	LVM.vars->eeprom_changed              = eeprom_read_word(&LVM.eeprom->eeprom_changed);
	
	
	
	diag_set_temp_r_span(LVM.options->r_span);
	
	diag_set_temp_r_zero(LVM.options->r_zero);

	
	
	#endif



	DISPLAY_CONFIG
	xoff = (!LVM.options->display_reversed)? 0 : XOffset;

	
	
	// Letters error is set on startup --> gets cleared as soon as device receives Letters from Server
	// It is ent with everymeasurement as part of the status byte, so server can resend if po letters have not been set yet.
	SET_ERROR(LETTERS_ERROR);
	


	//=========================================================================
	// Enable global interrupts
	//=========================================================================
	sei();
	
	
	// array --> firmwares where the eeprom was changed
	// if value read from eeprom is smaller than any of the stored versions --> show message to recalibrate
	
	
	if (((LVM.vars->eeprom_changed != LVM.version->Fw_version) && LVM.version->FW_eeprom_changed > LVM.vars->eeprom_changed) || LVM.vars->eeprom_changed > LVM.version->Fw_version ){
		// DIALOG --> eeprom values might be undefined pls recalibrate
		timed_dialog(STR_EEPROM_UNDEFINED,STR_VALUES_MIGHT_BE_UNDEFINED,15,D_FGC,D_BGC);
		//if(LCD_Dialog(STR_EEPROM_UNDEFINED,STR_VALUES_MIGHT_BE_UNDEFINED,D_BGC,D_FGC,30)){
			
		//}
		
		ready_for_new_key();
	}
	





	//=========================================================================
	// Check if offline modus is activated (HIDDEN FUNCTION keys pressed)
	//=========================================================================

	//=========================================================================
	// Initialization I2C
	//=========================================================================

	// Timer
	InitScreen_AddLine(STR_INIT_I2C_TIMER,1); //	LCD_Print("Init I2C Timer", 5, 40, 2, 1, 1, FGC, BGC);

	if (init_DS3231M(&paint_info_line) == 0) // trying to connect with DS3231M (time)
	{
		connected.DS3231M = 1;
		InitScreen_AddLine(STR_SUCCESSFUL,0);  //LCD_Print("successful", 5, 60, 2, 1, 1, FGC, BGC);
	}
	else
	{
		connected.DS3231M = 0;
		InitScreen_AddLine(STR_UN_SUCCESSFUL,0);  //LCD_Print("not successful", 5, 60, 2, 1, 1, FGC, BGC);
	}

	// Pressure Sensor
	InitScreen_AddLine(STR_INIT_I2C_PRESSURE,0); //LCD_Print("Init I2C Press", 5, 40, 2, 1, 1, FGC, BGC);

	if (init_HoneywellSSC() == 0) // trying to connect with HoneywellSSC (pressure)
	{
		InitScreen_AddLine(STR_SUCCESSFUL,0);  //LCD_Print("successful", 5, 60, 2, 1, 1, FGC, BGC);
	}
	else
	{
		InitScreen_AddLine(STR_UN_SUCCESSFUL,0);  //LCD_Print("not successful", 5, 60, 2, 1, 1, FGC, BGC);
	}

	sprintf(LVM.temp->string,"Status %d",HoneywellSSC_status.status);
	InitScreen_AddLine(LVM.temp->string,0);  //LCD_Print("Status ", 5, 80, 2, 1, 1, ERR, BGC); draw_int(HoneywellSSC_status.status, 65, 80, "", ERR);
	switch (HoneywellSSC_status.status)
	{
		case 3: // Sensor is defect
		InitScreen_AddLine(STR_DEFECT_PRESSURE_SENSOR,0);
		break;
		case 4: // No sensor connected
		InitScreen_AddLine(STR_NO_PRESSURE_SENSOR,0);
		break;
	}

	xbee_hardware_version();
	
	
	#ifdef ili9341

	if(LVM.version->hw_version_xbee == XBEE_V_S1){
		InitScreen_AddLine("Xbee HW: Xbee S1",0);
	}

	if(LVM.version->hw_version_xbee == XBEE_V_SC2){
		InitScreen_AddLine("Xbee HW: Xbee S2C",0);
	}
	#endif // ili9341

	xbee_Set_Scan_Channels(xbee.ScanChannels);
	xbee_WR();

	#if 1
	unsigned char i;
	for (i = 0; i < 20; ++i)
	{
		_delay_ms(100);
		if (keyhit_cont() == HIDDEN_FUNCTION)
		{
			InitScreen_AddLine(STR_OFFLINE_MODE,1);
			_delay_ms(2000);
			global_mode.netstat = offline;

			break;
		}
	}
	#else
	_delay_ms(2000);
	if (keyhit_cont() == HIDDEN_FUNCTION)
	{
		InitScreen_AddLine(STR_OFFLINE_MODE,1);
		_delay_ms(2000);
		global_mode.netstat = offline;
	}
	#endif
	
	


	//=========================================================================
	// TRY TO CONNECT TO BASE STATION
	// if not in offline mode
	//=========================================================================
	

	if(global_mode.netstat == online)
	{
		//=========================================================================
		// Display connection is in progress
		//=========================================================================

		InitScreen_AddLine(STR_NETWORK_CONN,0);
		InitScreen_AddLine(STR_IN_PROGRESS,0);

		//=========================================================================
		// Try to establish connection to the network
		//=========================================================================
		#ifdef ALLOW_LOGIN
		int loop = 1;

		xbee_Set_Scan_Channels(xbee.ScanChannels);

		while(loop)
		{
			if(xbee_reset_connection(0))
			{
				if(xbee_get_server_adrr())
				{
					global_mode.netstat = online;
					break;
				}
			}
			InitScreen_AddLine(STR_FAILED_DOT,0);
			InitScreen_AddLine(STR_PRESS_KEY_TO_TRY_AGAIN,0);


			// waiting for the user to press a key and giving the
			// possibility to go to OFFLINE MODE if there is no connection
			// shutdown if no key is pressed

			// set timer until shutdown
			set_timeout(10000, TIMER_0, USE_TIMER);
			// if key is pressed retry to connect or goto OFFLINE mode
			while (set_timeout(0, TIMER_0, USE_TIMER))
			{
				//					LCD_Print("timer running", 5, 20, 2, 1, 1, FGC, BGC);
				if (keyhit_cont() != 0) // key pressed
				{
					break;
				}
			}

			// if timer is not completely run down check if key pressed is HIDDEN_FUNCTION then goto OFFLINE mode
			// or in the other case try to reconnect
			if(set_timeout(0,TIMER_0, USE_TIMER))
			{
				//					LCD_Print("timer running 2", 5, 20, 2, 1, 1, FGC, BGC);
				if(keyhit_cont() == HIDDEN_FUNCTION)
				{
					InitScreen_AddLine(STR_OFFLINE_MODE,1);
					_delay_ms(2000);
					global_mode.netstat = offline;
					break;
				}
			}
			else SHUTDOWN;


			InitScreen_AddLine(STR_NETWORK_CONN,1);
			InitScreen_AddLine(STR_IN_PROGRESS,0);

			//			loop -=1;   // use this to stop the infinite loop after one run

		}	// end of infinite loop


		//=========================================================================
		// Device Login
		//=========================================================================

		if(global_mode.netstat == online)
		{
			//enter device number
			_Bool login_okay = false;
			uint8_t status = 0;
			_Bool bad_id = false;

			while(!login_okay)
			{
				if(!bad_id)
				{
					get_Device_ID(&status); // User prompt - Return the vessel number

					//device_id = 1; //TODO remove when serverside  alphanumeric devid decode is implemented

					switch (status)  // Pressed cancel (status 2) or hidden mode (status 1)
					{
						case 1:
						InitScreen_AddLine(STR_OFFLINE_MODE,1);
						_delay_ms(2000);
						global_mode.netstat = offline;
						login_okay = true;
						break;
						case 2:
						if(LCD_Dialog(STR_SHUTDOWN,STR_DO_YOU_REALLY_WANT_SHUTDOWN, D_FGC, D_BGC,SHUTDOWN_TIMEOUT_TIME))
						{
							// Confirmed shutting down, set shutdown action
							SHUTDOWN;
						}
						else
						{
							// Canceled, go back to the main mode
							continue;
						}
						break;
						default:
						break;
					}
					
					if (status == 1)
					{
						// OFFLINE MODE
						break;
					}

					// TODO  test auf NULL ID if(!device_id) continue;   // skip null ID
					sprintf(LVM.temp->string,STR_DO_YOU_WANT_TO_CONNECT_VESSEL_TO_SERVER,LVM.vars->Device_ID_Str);
				}
				else ;  // text was created in the preceding round... see cases 2, 4, 5 //sprintf(temp,"No vessel with ID\n%d found,\nstill login?",device_id);

				if (LCD_Dialog(STR_SERVER, LVM.temp->string, D_FGC, D_BGC,LOGIN_CONNECT_TIMEOUT_TIME))
				{
					//if answer "yes" then send
					status = (bad_id)?
					xbee_send_login_msg(FORCE_LOGIN_MSG, LVM.temp->buffer)
					:	xbee_send_login_msg(LOGIN_MSG, LVM.temp->buffer);

					/*					Debug
					LCD_Cls(BGC);
					draw_int(status, 85, 60, "status", ERR);
					_delay_ms(1000);
					*/
					

					switch(status)
					{
						case 0:
						//GOOD_OPTIONS:

						//Message on screen
						InitScreen_AddLine(STR_GOOD_OPTIONS,0);

						set_Options(LVM.temp->buffer,LOGIN_MSG);
						
						
						DS3231M_read_time();
						// Message on screen
						sprintf(LVM.temp->string,STR_NEW_DATE, Time.tm_mday, Time.tm_mon, Time.tm_year+2000);
						InitScreen_AddLine(LVM.temp->string,0);
						sprintf(LVM.temp->string,STR_NEW_TIME, Time.tm_hour, Time.tm_min, Time.tm_sec);
						InitScreen_AddLine(LVM.temp->string,0);
						
						xbee_coordIdentifier();
						InitScreen_AddLine(xbee_get_coordID(),0);

						// Save settings in EEPROM
						#ifdef ALLOW_EEPROM_SAVING
						write_opts_to_EEPROM();
						#endif



						login_okay = true;
						break;
						case 1:
						//BAD_OPTIONS:

						InitScreen_AddLine(STR_BAD_OPTS_RECEIVED,0);
						InitScreen_AddLine(STR_DEFAILT_OPTS_SET,0);
						_delay_ms(1000);

						LVM.options->transmit_slow = TRANSMIT_SLOW_DEF;
						LVM.options->transmit_slow_min = false;
						LVM.options->transmit_fast = TRANSMIT_FAST_DEF;
						LVM.options->transmit_fast_sec = false;
						LVM.options->res_min = RES_MIN_DEF;
						LVM.options->res_max = RES_MAX_DEF;
						LVM.options->quench_time = QUENCH_TIME_DEF;
						LVM.options->quench_current = QUENCH_CURRENT_DEF;
						LVM.options->wait_time = WAIT_TIME_DEF;
						LVM.options->meas_current = MEAS_CURRENT_DEF;
						LVM.options->meas_cycles = MEASUREMENT_CYCLES_DEF;
						LVM.options->fill_timeout = FILLING_TIMEOUT_DEF;
						LVM.options->span = SPAN_DEF;
						LVM.options->zero = ZERO_DEF;
						LVM.options->total_volume = TOTAL_VOL_DEF;
						LVM.options->he_min = AUTO_FILL_HE_DEF;

						LVM.options->display_reversed = 0;
						DISPLAY_CONFIG
						xoff = (!LVM.options->display_reversed)? 0 : XOffset;

						LVM.options->batt_min = BATT_MIN_DEF;
						LVM.options->batt_max = BATT_MAX_DEF;
						LVM.options->critical_batt = CRITICAL_BATT_DEF;

						login_okay = true;
						break;
						case 2:
						//Vessel parameters not valid, message "V":
						bad_id = true;
						sprintf(LVM.temp->string,STR_VESSEL_WITH_ID_NOT_VALID ,LVM.vars->Device_ID_Str);
						break;
						case 3:
						//TOP_SECRET: hidden function
						global_mode.netstat = offline;
						login_okay = true;
						break;
						case 4:
						//Vessel not on campus, message "W":
						bad_id = true;
						sprintf(LVM.temp->string,STR_VESSEL_NOT_ON_CAMPUS,LVM.vars->Device_ID_Str);
						break;
						case 5:
						//Vessel already connected, message "S":
						bad_id = true;
						sprintf(LVM.temp->string,STR_VESSEL_ALREADY_CONNECTED,LVM.vars->Device_ID_Str);
						break;
						case 6:
						//Vessel not known, message "T":
						bad_id = true;
						sprintf(LVM.temp->string,STR_VESSEL_WITH_ID_NOT_FOUND,LVM.vars->Device_ID_Str);
						break;
						case 7:
						//Module not known, message "M":
						InitScreen_AddLine(STR_LEVEL_METER_NOT_KNOWN,1);
						InitScreen_AddLine(STR_OFFLINE_MODE,0);
						_delay_ms(2000);
						global_mode.netstat = offline;
						login_okay = true;
						break;
						case 0xFF:
						//FAILED_LOGIN:
						
						_delay_ms(5000);
						global_mode.curr = ex_error;
						global_mode.next = ex_error;
						global_mode.error =login_failed;
						Model = error_model;
						login_okay = true;
						break;
						default:
						//FAILED_LOGIN:
						global_mode.curr = ex_error;
						global_mode.next = ex_error;
						global_mode.error =login_failed;
						Model = error_model;
						login_okay = true;
						break;
					}//switch status login request



				}else bad_id = false;  //if response "no" :enter vessel number again
			}//while !login_okay
		}
		#else

		//TODO  set d_id to default if no login allowed
		#endif

	} // end of if(ex_errorCode != ex_errorCode_Offline)
	DISPLAY_CONFIG
	xoff = (!LVM.options->display_reversed)? 0 : XOffset;
	

	//=========================================================================
	//Initial setup
	//=========================================================================
	switch(global_mode.error)
	{
		case no_error:
		//1. measurement

		InitScreen_AddLine(STR_1ST_MEASUREMENT,1);
		

		measure(Model);

		if (global_mode.netstat == online)
		{
			//Wake up xbee
			xbee_wake_up_plus();
			
			xbee_coordIdentifier();
			
			collect_and_send_MeasData(LVM.temp->buffer,LONG_INTERVAL_MSG);
			
			
			//set XBEE SLEEP and start waiting to wake it up
			xbee_sleep_plus();


			// Free XBee module
			
			
			//set slow transmission; transmit_slow_min indicates whether its in min or hours
			(LVM.options->transmit_slow_min)?
			set_timeout(ceil(LVM.options->transmit_slow*60), TIMER_1, USE_TIMER)
			:	set_timeout(LVM.options->transmit_slow*3600, TIMER_1, USE_TIMER);
			}else{
			xbee_sleep();
		}
		// Batterycheck offline mode if battery low --> go to optioins 
		if(global_mode.netstat == offline && LVM.options->batt_min >= map_to_batt(readChannel(BATTERY, 10*ADC_LOOPS))){
				// Set options mode
				global_mode.next = ex_options;
				global_mode.curr = ex_options;
				
				Model = option_model;
				
				if (Model->mode->netstat == offline)
				{
					make_he_vol_changable();


				}

				sprintf(LVM.temp->string,STR_THE_BATTERY_WARNING_CHANGE_PARAMS ,((uint8_t) LVM.vars->batt_level));
				timed_dialog(STR_BATTERY_WARNING , LVM.temp->string, 10, D_FGC, D_BGC);


				set_timeout(0, TIMER_3, RESET_TIMER);
				set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);

				
				set_OptionModel(4,3,0);
				set_bufferVars(false);	
				opt_drawPage();
							
				keyhit_block();
				
				option_model->batt_check = false;
			
			
		}else{
			//paint screen
			paint_main(Time, global_mode.netstat, PAINT_ALL);
		}



		break;

		default:
		break;
	}


	set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
	



	
	//=========================================================================
	// Main loop
	//=========================================================================
	while(1)
	{

		

		if (!connected.DS3231M)
		{
			if (!I2C_ClearBus())
			{
				connected.DS3231M = 1;
				}else{
				paint_info_line("I2C recovery failed",1);
			}


		}

		
		Controller(Model);

		//TODO more elegant switcher

		if (global_mode.curr != global_mode.next)
		{
			switch (global_mode.next)
			{
				case ex_main:
				Model = main_model;
				break;
				case ex_filling:
				Model = filling_model;
				break;
				case ex_options:
				Model = option_model;
				break;
				case ex_diagnostic:
				Model = diag_model;
				break;
				case ex_error:
				Model = error_model;
				break;
				case ex_diag_pulse:
				//TODO
				break;
				case ex_pulse_select:
				Model = pselect_model;
				default:
				//TODO
				break;
				
			}
			global_mode.curr = global_mode.next;
		}



		//=========================================================================
		// CPU goes to sleep mode
		//=========================================================================
		// Set the desired sleep mode using set_sleep_mode(). It usually defaults to idle mode where the CPU is put on sleep but all peripheral clocks are still running.
		// This instruction makes the MCU enter Idle mode, stopping the CPU but allowing the SPI, USART, Analog Comparator, ADC, two-wire Serial Interface, Timer/Counters, Watchdog, and the interrupt system to continue operating.
		// This sleep mode basically halts clkCPU and clkFLASH, while allowing the other clocks to run.
		// Idle mode enables the MCU to wake up from external triggered interrupts as well as internal ones like the Timer Overflow and USART Transmit Complete interrupts.
		// If wake-up from the Analog Comparator interrupt is not required, the Analog Comparator can be powered down by setting the ACD bit in the Analog Comparator Control and Status Register � ACSR.
		// This will reduce power consumption in Idle mode. If the ADC is enabled, a conversion starts automatically when this mode is entered.
		set_sleep_mode(SLEEP_MODE_IDLE);
		// Then call sleep_mode(). This macro automatically sets the sleep enable bit, goes to sleep, and clears the sleep enable bit.
		sleep_mode();

	}	// End of infinite loop
}
