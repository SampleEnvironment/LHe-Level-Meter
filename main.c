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
#include "display_driver.h"
#include "adwandler.h"
#include "display.h"

#include "display_utilities.h"
#include "keyboard.h"
#include "timer_utilities.h"
#include "usart.h"
#include "xbee.h"
#include "xbee_utilities.h"
#include "DS3231M_helevel.h"
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

	.r_span = R_SPAN_DEF,
	.r_zero = R_ZERO_DEF,

	//options
	.transmit_slow  = TRANSMIT_SLOW_DEF,
	.transmit_slow_min = 0,
	.transmit_fast  = TRANSMIT_FAST_DEF,
	.transmit_fast_sec= 0,
	.quench_time  = QUENCH_TIME_DEF * 1000,
	.quench_current = QUENCH_CURRENT_DEF,
	.wait_time = WAIT_TIME_DEF * 1000,
	.meas_current  = MEAS_CURRENT_DEF,
	.meas_cycles  = MEASUREMENT_CYCLES_DEF,
	.fill_timeout  = FILLING_TIMEOUT_DEF,
	.he_min  = AUTO_FILL_HE_DEF,
	.res_min  = RES_MIN_DEF * 10,
	.res_max  = RES_MAX_DEF * 10,
	.span  = SPAN_DEF * 10,
	.zero  = ZERO_DEF * 10,
	//	uint8_t		ee_enable_pressure = 0;
	.enable_pressure  = 0,  //???
	.batt_min  = BATT_MIN_DEF * 10,
	.batt_max  = BATT_MAX_DEF * 10,
	.critical_batt = CRITICAL_BATT_DEF,
	.total_volume  = TOTAL_VOL_DEF*10,
	.display_reversed = 0,
	.alphanum = 0,
	.dev_id_char_num= DEV_ID_CHARS_DEF,
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
	.Dev_ID_Max_len = DEVICE_ID_UI_MAX_LEN

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


VersionType version = {
	.Fw_version = FIRMWARE_VERSION,
	.Branch_id = BRANCH_ID,
	.FW_eeprom_changed = LAST_FIRMWARE_EEPROM_CHANGED,
	.hw_version_xbee = 0
};






VarsType vars = {
	.r_val_last_Meas = 0,

	.he_level = 0,
	.last_he_level = 200,
	.batt_level = 0,
	.pressure_level = 0,
	.eeprom_changed = 0,
	.very_low_He_level = false,
	.auto_fill_enabled = false,
	.auto_fill_started = false,
	.device_pos ="none",
	.options_pw = OPTIONS_PASSWORD,
	.entered_options_pw = 0,
	.transmit_fast_changed = false,
	.transmit_slow_changed = false,
	.fill_meas_counter =false,
	.Device_ID_Str=""
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


int I2C_ClearBus(void) {



	TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly


	INPUT_PULLUP(SDA);; // Make SDA (data) and SCL (clock) pins Inputs with pullup.
	INPUT_PULLUP(SCL);

	_Bool SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
	if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master.
		return 1; //I2C bus error. Could not clear SCL clock line held low
	}

	_Bool SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
	int clockCount = 20; // > 2x9 clock

	while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
		clockCount--;
		// Note: I2C bus is open collector so do NOT drive SCL or SDA high.
		INPUT_NO_PULLUP(SCL); // release SCL pullup so that when made output it will be LOW
		pinMode(SCL, OUTPUT); // then clock SCL Low
		_delay_us(10); //  for >5uS
		pinMode(SCL, INPUT); // release SCL LOW
		INPUT_PULLUP(SCL); // turn on pullup resistors again
		// do not force high as slave may be holding it low for clock stretching.
		_delay_us(10); //  for >5uS
		// The >5uS is so that even the slowest I2C devices are handled.
		SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
		int counter = 20;
		while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
			counter--;
			_delay_ms(100);
			SCL_LOW = (digitalRead(SCL) == LOW);
		}
		if (SCL_LOW) { // still low after 2 sec error
			return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
		}
		SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
	}
	if (SDA_LOW) { // still low
		return 3; // I2C bus error. Could not clear. SDA data line held low
	}

	// else pull SDA line low for Start or Repeated Start
	INPUT_NO_PULLUP(SDA); // remove pullup.
	pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
	// When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
	/// A Repeat Start is a Start occurring after a Start with no intervening Stop.
	_delay_us(10); // wait >5uS
	pinMode(SDA, INPUT); // remove output low
	INPUT_PULLUP(SDA);; // and make SDA high i.e. send I2C STOP control.
	_delay_us(10); // x. wait >5uS

	i2c_init();

	return 0; // all ok
}














uint8_t collect_and_send_MeasData(uint8_t *meas_buffer,uint8_t Message_Code){
	// enter time
	if (DS3231M_status.connected)
	{
		DS3231M_read_time();

	}
	else
	{
		Time.second = 0;
		Time.minute = 0;
		Time.hour   = 0;
		Time.date   = 0;
		Time.month  = 0;
		Time.year   = 0;
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
	meas_buffer[index++] = Time.second;
	meas_buffer[index++] = Time.minute;
	meas_buffer[index++] = Time.hour;
	meas_buffer[index++] = Time.date;
	meas_buffer[index++] = Time.month;
	meas_buffer[index++] = Time.year;


	if(LVM.vars->he_level < 0)	// If negative He level, send error code
	{
		meas_buffer[index++] = HE_LEVEL_ERROR>>8;
		meas_buffer[index++] = (uint8_t) HE_LEVEL_ERROR;
	}
	else
	{
		meas_buffer[index++] = ((uint16_t)(LVM.vars->he_level*10))>>8;
		meas_buffer[index++] = ((uint16_t)(LVM.vars->he_level*10));
	}

	meas_buffer[index++] = LVM.vars->batt_level;

	meas_buffer[index++] = (uint16_t) LVM.vars->pressure_level >> 8;
	meas_buffer[index++] = (uint16_t) LVM.vars->pressure_level;

	// enter temperature
	if (DS3231M_status.connected)
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

		meas_buffer[index++] = (uint16_t) (DS3231M_Temperature * 100) >> 8;
		meas_buffer[index++] = (uint16_t) (DS3231M_Temperature * 100);
	}
	else
	{
		meas_buffer[index++] = 0;
		meas_buffer[index++] = 0;




	}

	// position
	index =  devicePos_to_buffer(LVM.vars->device_pos, index, LVM.temp->buffer);  // Positions are 4 letters

	meas_buffer[index++] = xbee_get_status_byte();
	if (LVM.vars->r_val_last_Meas > 6300){
		LVM.vars->r_val_last_Meas = 6300;
	}


	meas_buffer[index++] = ((uint16_t)(LVM.vars->r_val_last_Meas*10))>>8;
	meas_buffer[index++] = ((uint16_t)(LVM.vars->r_val_last_Meas*10));

	memcpy(LVM.temp->databuffer,meas_buffer,index);


	if(Message_Code == TRIGGER_MEAS_MSG){
		// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
		if (xbee_send_message(Message_Code, meas_buffer, index))
		{
			xbee_set_status_byte(0); // Clear all errors
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
			xbee_set_status_byte(0); // Clear all errors
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
		reply_Id = xbee_send_request_only(db_cmd_type, buffer, 0);
		
		if(reply_Id != 0xFF)
		{
			//				sprintf(ltemp, "...data received: %i", frameBuffer[reply_Id].data_len);
			InitScreen_AddLine(STR_DATE_RECEIVED,0);


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




void write_opts_to_EEPROM(void){
	eeprom_write_float(&LVM.eeprom->r_zero, LVM.options->r_zero);
	eeprom_write_float(&LVM.eeprom->r_span, LVM.options->r_span);

	eeprom_write_word(&LVM.eeprom->transmit_slow, LVM.options->transmit_slow);
	eeprom_write_byte(&LVM.eeprom->transmit_slow_min, LVM.options->transmit_slow_min);
	eeprom_write_word(&LVM.eeprom->transmit_fast, LVM.options->transmit_fast);
	eeprom_write_byte(&LVM.eeprom->transmit_fast_sec, LVM.options->transmit_fast_sec);
	eeprom_write_word(&LVM.eeprom->quench_time, (uint16_t) (LVM.options->quench_time*1000));
	eeprom_write_word(&LVM.eeprom->quench_current, (uint16_t) LVM.options->quench_current);
	eeprom_write_word(&LVM.eeprom->wait_time, (uint16_t) (LVM.options->wait_time*1000));
	eeprom_write_word(&LVM.eeprom->meas_current, (uint16_t) LVM.options->meas_current);

	eeprom_write_byte(&LVM.eeprom->meas_cycles, LVM.options->meas_cycles);
	eeprom_write_byte(&LVM.eeprom->fill_timeout, LVM.options->fill_timeout);
	eeprom_write_byte(&LVM.eeprom->he_min, LVM.options->he_min);
	eeprom_write_word(&LVM.eeprom->res_min, (uint16_t) (LVM.options->res_min*10));
	eeprom_write_word(&LVM.eeprom->res_max, (uint16_t) (LVM.options->res_max*10));
	eeprom_write_word(&LVM.eeprom->span, (uint16_t) (LVM.options->span*10));
	eeprom_write_word(&LVM.eeprom->zero, (uint16_t) (LVM.options->zero*10));
	eeprom_write_byte(&LVM.eeprom->enable_pressure, LVM.options->enable_pressure);
	eeprom_write_word(&LVM.eeprom->batt_min, (uint16_t) (LVM.options->batt_min*10));
	eeprom_write_word(&LVM.eeprom->batt_max, (uint16_t) (LVM.options->batt_max*10));
	eeprom_write_byte(&LVM.eeprom->critical_batt, LVM.options->critical_batt);

	eeprom_write_word(&LVM.eeprom->total_volume, (uint16_t) (LVM.options->total_volume*10));
	eeprom_write_byte(&LVM.eeprom->display_reversed, LVM.options->display_reversed);

	eeprom_write_byte(&LVM.eeprom->alphanum,LVM.options->Dev_ID_alpahnum);
	eeprom_write_byte(&LVM.eeprom->dev_id_char_num,LVM.options->Dev_ID_Max_len);

}

void write_DEFS_to_EEPROM(void){
	eeprom_write_float(&LVM.eeprom->r_zero,R_ZERO_DEF );
	eeprom_write_float(&LVM.eeprom->r_span, R_SPAN_DEF);

	eeprom_write_word(&LVM.eeprom->transmit_slow, TRANSMIT_SLOW_DEF);
	eeprom_write_byte(&LVM.eeprom->transmit_slow_min, false);
	eeprom_write_word(&LVM.eeprom->transmit_fast, TRANSMIT_FAST_DEF);
	eeprom_write_byte(&LVM.eeprom->transmit_fast_sec, false);
	eeprom_write_word(&LVM.eeprom->quench_time,QUENCH_TIME_DEF*1000);
	eeprom_write_word(&LVM.eeprom->quench_current, QUENCH_CURRENT_DEF);
	eeprom_write_word(&LVM.eeprom->wait_time, WAIT_TIME_DEF*1000);
	eeprom_write_word(&LVM.eeprom->meas_current, MEAS_CURRENT_DEF);

	eeprom_write_byte(&LVM.eeprom->meas_cycles, MEASUREMENT_CYCLES_DEF);
	eeprom_write_byte(&LVM.eeprom->fill_timeout, FILLING_TIMEOUT_DEF);
	eeprom_write_byte(&LVM.eeprom->he_min, AUTO_FILL_HE_DEF);
	eeprom_write_word(&LVM.eeprom->res_min, RES_MIN_DEF*10);
	eeprom_write_word(&LVM.eeprom->res_max, RES_MAX_DEF*10);
	eeprom_write_word(&LVM.eeprom->span, SPAN_DEF*10);
	eeprom_write_word(&LVM.eeprom->zero, ZERO_DEF*10);
	eeprom_write_byte(&LVM.eeprom->enable_pressure, 0);
	eeprom_write_word(&LVM.eeprom->batt_min, BATT_MIN_DEF*10);
	eeprom_write_word(&LVM.eeprom->batt_max, BATT_MAX_DEF*10);
	eeprom_write_byte(&LVM.eeprom->critical_batt, CRITICAL_BATT_DEF);

	eeprom_write_word(&LVM.eeprom->total_volume, TOTAL_VOL_DEF*10);
	eeprom_write_byte(&LVM.eeprom->display_reversed, false);
	
	eeprom_write_byte(&LVM.eeprom->alphanum,false);
	eeprom_write_byte(&LVM.eeprom->dev_id_char_num,DEV_ID_CHARS_DEF);
	
	

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
	LVM.options->display_reversed 	= eeprom_read_byte(&LVM.eeprom->display_reversed);
	#endif

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
	_delay_ms(3000);
	LCD_Print(LVM.temp->string,20,160,2,1,1,BGC,white);
	sprintf(LVM.temp->string,"%s %iv%i", STR_FIRMWARE_VERSION,LVM.version->Branch_id,LVM.version->Fw_version);
	LCD_Print(LVM.temp->string,20,185,2,1,1,BGC,white);
	_delay_ms(3000);
	#endif

	#ifdef DISP_3000
	LCD_Cls(white);

	LCD_LOGO(5,20,white);

	sprintf(LVM.temp->string,"%s ", STR_HZB_LEVELMETER);
	_delay_ms(3000);
	LCD_Print(LVM.temp->string,10,90,2,1,1,BGC,white);

	sprintf(LVM.temp->string,"Firmware %iv%i",LVM.version->Branch_id,LVM.version->Fw_version);
	LCD_Print(LVM.temp->string,10,105,2,1,1,BGC,white);
	_delay_ms(3000);
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
	LVM.options->r_span 				= (double) (eeprom_read_float(&LVM.eeprom->r_span));
	if(LVM.options->r_span < 0 || isnan(LVM.options->r_span)) LVM.options->r_span = R_SPAN_DEF;
	LVM.options->r_zero 				= (double) (eeprom_read_float(&LVM.eeprom->r_zero));
	if(LVM.options->r_zero < 0 || isnan(LVM.options->r_zero) || LVM.options->r_zero > 100)	LVM.options->r_zero = R_ZERO_DEF;
	LVM.options->transmit_slow 		= eeprom_read_word(&LVM.eeprom->transmit_slow); 					CHECK_BOUNDS(transmit_slow,TRANSMIT_SLOW_MIN,TRANSMIT_SLOW_MAX,TRANSMIT_SLOW_DEF)
	LVM.options->transmit_slow_min 	= eeprom_read_byte(&LVM.eeprom->transmit_slow_min);					if(LVM.options->transmit_slow == TRANSMIT_SLOW_DEF) 	LVM.options->transmit_slow_min = false; //???
	LVM.options->transmit_fast 		= eeprom_read_word(&LVM.eeprom->transmit_fast);						CHECK_BOUNDS(transmit_fast,TRANSMIT_FAST_MIN,TRANSMIT_FAST_MAX,TRANSMIT_FAST_DEF)
	LVM.options->transmit_fast_sec	= eeprom_read_byte(&LVM.eeprom->transmit_fast_sec);					if(LVM.options->transmit_fast == TRANSMIT_FAST_DEF) 	LVM.options->transmit_fast_sec = false;  //???
	LVM.options->quench_time			= (double) (eeprom_read_word(&LVM.eeprom->quench_time)/1000.0);	CHECK_BOUNDS(quench_time,QUENCH_TIME_MIN,QUENCH_TIME_MAX,QUENCH_TIME_DEF)
	LVM.options->quench_current		= (double) (eeprom_read_word(&LVM.eeprom->quench_current));			CHECK_BOUNDS(quench_current,QUENCH_CURRENT_MIN,QUENCH_CURRENT_MAX,QUENCH_CURRENT_DEF)
	LVM.options->wait_time			= (double) (eeprom_read_word(&LVM.eeprom->wait_time)/1000.0);		CHECK_BOUNDS(wait_time,WAIT_TIME_MIN,WAIT_TIME_MAX,WAIT_TIME_DEF)
	LVM.options->meas_current		= (double) (eeprom_read_word(&LVM.eeprom->meas_current));			CHECK_BOUNDS(meas_current,MEAS_CURRENT_MIN,MEAS_CURRENT_MAX,MEAS_CURRENT_DEF)
	LVM.options->meas_cycles 		= eeprom_read_byte(&LVM.eeprom->meas_cycles);						CHECK_BOUNDS(meas_cycles,MEASUREMENT_CYCLES_MIN,MEASUREMENT_CYCLES_MAX,MEASUREMENT_CYCLES_DEF)
	LVM.options->fill_timeout 		= eeprom_read_byte(&LVM.eeprom->fill_timeout);						CHECK_BOUNDS(fill_timeout,MIN_FILLING_TIMEOUT,MAX_FILLING_TIMEOUT,FILLING_TIMEOUT_DEF)
	LVM.options->he_min 				= eeprom_read_byte(&LVM.eeprom->he_min);						CHECK_BOUNDS(he_min,MIN_AUTO_FILL_HE,MAX_AUTO_FILL_HE,AUTO_FILL_HE_DEF)
	LVM.options->res_min 			= (double) (eeprom_read_word(&LVM.eeprom->res_min)/10.0);			CHECK_BOUNDS(res_min,RES_MIN_MIN,RES_MIN_MAX,RES_MIN_DEF)
	LVM.options->res_max 			= (double) (eeprom_read_word(&LVM.eeprom->res_max)/10.0);			CHECK_BOUNDS(res_max,RES_MAX_MIN,RES_MAX_MAX,RES_MAX_DEF)
	LVM.options->span 				= (double) (eeprom_read_word(&LVM.eeprom->span)/10.0);				CHECK_BOUNDS(span,MIN_SPAN,MAX_SPAN,SPAN_DEF)
	LVM.options->zero 				= (double) (eeprom_read_word(&LVM.eeprom->zero)/10.0);				CHECK_BOUNDS(zero,MIN_ZERO,MAX_ZERO,ZERO_DEF)
	LVM.options->enable_pressure 	= eeprom_read_byte(&LVM.eeprom->enable_pressure);					if(LVM.options->enable_pressure > 2) 	LVM.options->enable_pressure= 0;
	LVM.options->batt_min 			= (double) (eeprom_read_word(&LVM.eeprom->batt_min)/10.0);			CHECK_BOUNDS(batt_min,BATT_MIN_MIN,BATT_MIN_MAX,BATT_MIN_DEF)
	LVM.options->batt_max 			= (double) (eeprom_read_word(&LVM.eeprom->batt_max)/10.0);			CHECK_BOUNDS(batt_max,BATT_MAX_MIN,BATT_MAX_MAX,BATT_MAX_DEF)
	LVM.options->critical_batt 		= eeprom_read_byte(&LVM.eeprom->critical_batt);						CHECK_BOUNDS(critical_batt,CRITICAL_BATT_MIN,CRITICAL_BATT_MAX,CRITICAL_BATT_DEF)
	LVM.options->total_volume		= (double) (eeprom_read_word(&LVM.eeprom->total_volume)/10.0);		CHECK_BOUNDS(total_volume,TOTAL_VOL_MIN,TOTAL_VOL_MAX,TOTAL_VOL_DEF)
	LVM.options->Dev_ID_alpahnum     = eeprom_read_byte(&LVM.eeprom->alphanum);
	LVM.options->Dev_ID_Max_len		= eeprom_read_byte(&LVM.eeprom->dev_id_char_num);					CHECK_BOUNDS(Dev_ID_Max_len,DEV_ID_CHARS_MIN,DEV_ID_CHARS_MAX,DEV_ID_CHARS_DEF)
	
	LVM.vars->eeprom_changed              = eeprom_read_word(&LVM.eeprom->eeprom_changed);
	
	diag_set_temp_r_span(LVM.options->r_span);
	
	diag_set_temp_r_zero(LVM.options->r_zero);

	
	
	#endif



	





	//=========================================================================
	// Enable global interrupts
	//=========================================================================
	sei();
	
	
	// array --> firmwares where the eeprom was changed
	// if value read from eeprom is smaller than any of the stored versions --> show message to recalibrate
	
	
	if (((LVM.vars->eeprom_changed != LVM.version->Fw_version) && LVM.version->FW_eeprom_changed > LVM.vars->eeprom_changed) || LVM.vars->eeprom_changed > LVM.version->Fw_version ){
		// DIALOG --> eeprom values might be undefined pls recalibrate
		if(LCD_Dialog(STR_EEPROM_UNDEFINED,STR_VALUES_MIGHT_BE_UNDEFINED,D_BGC,D_FGC,30)){
			eeprom_write_word(&LVM.eeprom->eeprom_changed,LVM.version->Fw_version);
		}
		
		ready_for_new_key();
	}
	

	//=========================================================================
	// Enable global interrupts
	//========================================================================



	//=========================================================================
	// Check if offline modus is activated (HIDDEN FUNCTION keys pressed)
	//=========================================================================

	//=========================================================================
	// Initialization I2C
	//=========================================================================

	// Timer
	InitScreen_AddLine(STR_INIT_I2C_TIMER,1); //	LCD_Print("Init I2C Timer", 5, 40, 2, 1, 1, FGC, BGC);

	if (init_DS3231M() == 0) // trying to connect with DS3231M (time)
	{
		DS3231M_status.connected = 1;
		InitScreen_AddLine(STR_SUCCESSFUL,0);  //LCD_Print("successful", 5, 60, 2, 1, 1, FGC, BGC);
	}
	else
	{
		DS3231M_status.connected = 0;
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

	LVM.version->hw_version_xbee =   (xbee_hardware_version() > 0x2000)? XBEE_V_SC2 : XBEE_V_S1;
	
	
	#ifdef ili9341

	if(LVM.version->hw_version_xbee == XBEE_V_S1){
		InitScreen_AddLine("Xbee HW: Xbee S1",0);
	}

	if(LVM.version->hw_version_xbee == XBEE_V_SC2){
		InitScreen_AddLine("Xbee HW: Xbee S2C",0);
	}
	#endif // ili9341


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

		while(loop)
		{
			if(xbee_reset_connection())
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

						//parse buffer

						// set time

						TimeBuff newtime;
						newtime.second =	LVM.temp->buffer[0];
						newtime.minute =	LVM.temp->buffer[1];
						newtime.hour =		LVM.temp->buffer[2];
						newtime.date =		LVM.temp->buffer[3];
						newtime.month =		LVM.temp->buffer[4];
						newtime.year =		LVM.temp->buffer[5];

						// Message on screen
						sprintf(LVM.temp->string,STR_NEW_DATE, newtime.date, newtime.month, newtime.year+2000);
						InitScreen_AddLine(LVM.temp->string,0);
						sprintf(LVM.temp->string,STR_NEW_TIME, newtime.hour, newtime.minute, newtime.second);
						InitScreen_AddLine(LVM.temp->string,0);

						/*								LCD_Print("new date", 5, 20, 2, 1, 1, FGC, BGC);
						draw_int(newtime.date, 5, 40, ".", ERR);
						draw_int(newtime.month, 35, 40, ".", ERR);
						draw_int(newtime.year+2000, 65, 40, "", ERR);
						LCD_Print("new time", 5, 60, 2, 1, 1, FGC, BGC);
						draw_int(newtime.hour, 5, 80, ":", ERR);
						draw_int(newtime.minute, 35, 80, ":", ERR);
						draw_int(newtime.second, 65, 80, "", ERR);
						delay_ms(2000);
						*/
						if (DS3231M_status.connected)
						{
							DS3231M_set_time(&newtime);
						}


						// Transmit slow in minutes
						LVM.options->transmit_slow = (LVM.temp->buffer[6]<<8) + LVM.temp->buffer[7];
						LVM.options->transmit_slow_min = true;
						if(LVM.options->transmit_slow < TRANSMIT_SLOW_MIN) LVM.options->transmit_slow = TRANSMIT_SLOW_MIN;

						if(LVM.options->transmit_slow > 60)
						{
							LVM.options->transmit_slow /= 60;
							LVM.options->transmit_slow_min = false;
							if (LVM.options->transmit_slow > TRANSMIT_SLOW_MAX) LVM.options->transmit_slow = TRANSMIT_SLOW_MAX;
						}


						// Transmit fast in seconds
						LVM.options->transmit_fast = (LVM.temp->buffer[8]<<8) + LVM.temp->buffer[9];
						LVM.options->transmit_fast_sec = true;

						if(LVM.options->transmit_fast < TRANSMIT_FAST_MIN) LVM.options->transmit_fast = TRANSMIT_FAST_MIN;

						if(LVM.options->transmit_fast > 60)
						{
							LVM.options->transmit_fast /= 60;
							LVM.options->transmit_fast_sec = false;
							if (LVM.options->transmit_fast > TRANSMIT_FAST_MAX) LVM.options->transmit_fast = TRANSMIT_FAST_MAX;
						}

						// Minimum resistance multiplied by factor 10
						LVM.options->res_min = (double)(((LVM.temp->buffer[10]<<8) + LVM.temp->buffer[11])/10.0);
						if(LVM.options->res_min < 0) LVM.options->res_min = RES_MIN_MIN;

						// Maximum resistance multiplied by factor 10
						LVM.options->res_max = (double)(((LVM.temp->buffer[12]<<8) + LVM.temp->buffer[13])/10.0);
						if(LVM.options->res_max <= 0) LVM.options->res_max = RES_MAX_DEF;

						// Quench time in ms - stored in seconds
						LVM.options->quench_time = round_double(((LVM.temp->buffer[14]<<8) + LVM.temp->buffer[15])/1000.0, 1);
						if(LVM.options->quench_time < 0) LVM.options->quench_time = QUENCH_TIME_DEF;

						/*							LCD_Print("quench time", 5, 20, 2, 1, 1, FGC, BGC);
						_delay_ms(1000);
						draw_double(quench_time, 5, 40, 1, "", ERR);
						draw_double(buffer[14], 5, 60, 1, "", ERR);
						draw_double(buffer[15], 5, 80, 1, "", ERR);
						_delay_ms(4000);
						*/
						// Quench current in mA
						LVM.options->quench_current = (double)(((LVM.temp->buffer[16]<<8) + LVM.temp->buffer[17]));
						if(LVM.options->quench_current <= 0) LVM.options->quench_current = QUENCH_CURRENT_DEF;

						/*							LCD_Print("quench current", 5, 20, 2, 1, 1, FGC, BGC);
						_delay_ms(1000);
						draw_double(quench_current, 5, 40, 1, "", ERR);
						draw_double(buffer[16], 5, 60, 1, "", ERR);
						draw_double(buffer[17], 5, 80, 1, "", ERR);
						_delay_ms(4000);
						*/

						// Wait time in ms - stored in seconds
						LVM.options->wait_time = round_double(((LVM.temp->buffer[18]<<8) + LVM.temp->buffer[19])/1000.0, 1);
						if(LVM.options->wait_time <= 0) LVM.options->wait_time = WAIT_TIME_DEF;

						// Measurement current in mA
						LVM.options->meas_current = (double)(((LVM.temp->buffer[20]<<8) + LVM.temp->buffer[21]));
						if(LVM.options->meas_current <= 0) LVM.options->meas_current = MEAS_CURRENT_DEF;

						// Number of measuring cycles of He probe
						LVM.options->meas_cycles = (!LVM.temp->buffer[22])? MEASUREMENT_CYCLES_DEF : LVM.temp->buffer[22];

						// Timeout while filling
						LVM.options->fill_timeout = (!LVM.temp->buffer[23])? FILLING_TIMEOUT_DEF : LVM.temp->buffer[23];

						// Span & zero
						LVM.options->span = ((LVM.temp->buffer[24]<<8) + LVM.temp->buffer[25])/10.0;
						if(LVM.options->span <= 0) LVM.options->span = SPAN_DEF;

						// zero is signed 2 byte integer, if buffer[20] is >=128 then zero is negative
						LVM.options->zero = (LVM.temp->buffer[26] < 128)? ((LVM.temp->buffer[26] * 256) + LVM.temp->buffer[27]) : ((LVM.temp->buffer[26]-256) * 256 + LVM.temp->buffer[27]);
						LVM.options->zero = LVM.options->zero/10;
						// alternative							zero = (double)((((buffer[26]<<8) + buffer[27])/10.0));


						// Total volume of the dewar multiplied by factor 10
						LVM.options->total_volume = round_double(((LVM.temp->buffer[28]<<8) + LVM.temp->buffer[29])/10.0, 1);
						if(LVM.options->total_volume <= 0) LVM.options->total_volume = TOTAL_VOL_DEF;

						// Minimum Helium Level when in Auto Fill Mode (in %)
						LVM.options->he_min = (!LVM.temp->buffer[30])? AUTO_FILL_HE_DEF : LVM.temp->buffer[30];
						if (LVM.options->he_min < MIN_AUTO_FILL_HE) LVM.options->he_min = MIN_AUTO_FILL_HE;
						if (LVM.options->he_min > MAX_AUTO_FILL_HE) LVM.options->he_min = MAX_AUTO_FILL_HE;

						// Orientation of display
						LVM.options->display_reversed = LVM.temp->buffer[31];
						DISPLAY_CONFIG
						xoff = (!LVM.options->display_reversed)? 0 : XOffset;

						// Minimum voltage of the battery multiplied by factor 10
						LVM.options->batt_min = round_double(((LVM.temp->buffer[32]<<8) + LVM.temp->buffer[33])/10.0, 1);
						if(LVM.options->batt_min <= 0) LVM.options->batt_min = BATT_MIN_DEF;

						// Maximum voltage of the battery multiplied by factor 10
						LVM.options->batt_max = round_double(((LVM.temp->buffer[34]<<8) + LVM.temp->buffer[35])/10.0, 1);
						if(LVM.options->batt_max <= 0) LVM.options->batt_max = BATT_MAX_DEF;

						// Critical percentage of battery
						LVM.options->critical_batt = (!LVM.temp->buffer[36])? CRITICAL_BATT_DEF : LVM.temp->buffer[36];


						LVM.vars->options_pw = ((LVM.temp->buffer[37]<<8) + LVM.temp->buffer[38]);

						xbee_set_sleep_period(LVM.temp->buffer[39]);

						xbee_set_awake_period(LVM.temp->buffer[40]);


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
		
		//paint screen
		paint_main(Time, global_mode.netstat, PAINT_ALL);


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
		

		if (!DS3231M_status.connected)
		{
			if (!I2C_ClearBus())
			{
				DS3231M_status.connected = 1;
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
		// If wake-up from the Analog Comparator interrupt is not required, the Analog Comparator can be powered down by setting the ACD bit in the Analog Comparator Control and Status Register Ð ACSR.
		// This will reduce power consumption in Idle mode. If the ADC is enabled, a conversion starts automatically when this mode is entered.
		set_sleep_mode(SLEEP_MODE_IDLE);
		// Then call sleep_mode(). This macro automatically sets the sleep enable bit, goes to sleep, and clears the sleep enable bit.
		sleep_mode();

	}	// End of infinite loop
}
