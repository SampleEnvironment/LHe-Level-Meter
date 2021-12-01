/*
* base_controller.c
*
* Created: 23.02.2021 17:56:21
*  Author: Weges
*/


#include "base_controller.h"
#include "pulse_select_mode.h"
#include "../keyboard.h"
#include "../timer_utilities.h"
#include "../display_utilities.h"
#include "../main.h"
#include "../HoneywellSSC.h"
#include "adwandler.h"
#include "../diag_pulse.h"


#include "xbee.h"
#include "xbee_utilities.h"
#include "I2C_utilities.h"
#include "status.h"
#include "DS3231M.h"


#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <time.h>

globalModesType global_mode = {
	.netstat = online,
	.curr = ex_main,
	.next= ex_main,
	.error = no_error,
	.Key = 0
};


void pressedFILL(Controller_Model *Model){
	Model->vtable->pressedFILL(Model);
}

void pressedMeasure(Controller_Model *Model){
	Model->vtable->pressedMeasure(Model);
}

void pressedHIDDEN(Controller_Model *Model){
	Model->vtable->pressedHIDDEN(Model);
}

void pressedTOP(Controller_Model *Model){
	Model->vtable->pressedTOP(Model);
}

void pressedUP(Controller_Model *Model){
	Model->vtable->pressedUP(Model);
}

void pressedLEFT(Controller_Model *Model){
	Model->vtable->pressedLEFT(Model);
}

void pressedRIGHT(Controller_Model *Model){
	Model->vtable->pressedRIGHT(Model);
}

void pressedDOWN(Controller_Model *Model){
	Model->vtable->pressedDOWN(Model);
}

void pressedBOT(Controller_Model *Model){
	Model->vtable->pressedBOT(Model);
}

void pressedNONE(Controller_Model *Model){
	Model->vtable->pressedNONE(Model);
}

void display_on_pressed(Controller_Model *Model){
	
	if (Model->mode->Key != 0)
	{
		if (!Model->vtable->display_on_pressed())
		{
			//--> Display was off
			//--> ignore kepress
			Model->mode->Key = 0;
			_delay_ms(200);
			keyhit_block();
		}
		
	}

}

void display_off_pressed(Controller_Model *Model){
	Model->vtable->display_off_pressed();
}


void pre_Switch_case_Tasks(Controller_Model *Model){
	Model->vtable->pre_Switch_case_Tasks(Model);
}

void post_Switch_case_Tasks(Controller_Model *Model){
	Model->vtable->post_Switch_case_Tasks(Model);
}





Controller_Model * Controller(Controller_Model *Model){
	
	Model->mode->Key = keyhit_block();
	
	display_on_pressed(Model);
	
	
	
	pre_Switch_case_Tasks(Model);
	
	switch(Model->mode->Key)
	{
		case KEY_FILL:
		{
			pressedFILL(Model);
			break;
		}
		case KEY_MEASURE:
		{
			pressedMeasure(Model);
			break;
		}
		case HIDDEN_FUNCTION:
		{
			pressedHIDDEN(Model);
			break;
		}
		case KEY_TOP_S5:
		{
			pressedTOP(Model);
			break;
		}
		case KEY_UP_S6:
		{
			pressedUP(Model);
			break;
		}
		case KEY_LEFT_S7:
		{
			pressedLEFT(Model);
			break;
		}
		case KEY_RIGHT_S8:
		{
			pressedRIGHT(Model);
			break;
		}
		case KEY_DOWN_S9:
		{
			pressedDOWN(Model);
			break;
		}
		case KEY_BOT_S10:
		{
			pressedBOT(Model);
			break;
		}
		default:
		{
			pressedNONE(Model);
			break;
		}
		
	}
	
	ready_for_new_key();
	
	display_off_pressed(Model);
	
	post_Switch_case_Tasks(Model);
	
	Battery_check(Model);
	
	return Model;
}


_Bool base_display_always_on(void){
	DISPLAY_TURN_ON;
	return true;
}




_Bool base_display_on_pressed(void){
	if (DISPLAY_ON)
	{
		// Display is on, extend display_on time
		set_timeout(0, TIMER_2, RESET_TIMER);
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		return true;
	}
	else
	{
		// Turn display backlight on
		DISPLAY_TURN_ON
		set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
		#ifdef ALLOW_DEBUG
		LCD_Print("on ", xoff+5, 20, 2, 1, 1, ERR, BGC);
		
		#endif
		return false;
	}
	
	
}

_Bool base_display_off_pressed(void){

	if(!set_timeout(0, TIMER_2, USE_TIMER))
	{
		DISPLAY_TURN_OFF;		// Turn off display
	}
	return false;

}

_Bool base_display_DONOTHING(void) {
	return true;
}

void base_pressedDONOTHING(Controller_Model *Model){
	
}

void base_display_extend_onTime(void){
	DISPLAY_TURN_ON;
	// Display is on, extend display_on time
	set_timeout(0, TIMER_2, RESET_TIMER);
	set_timeout(DISP_TIMEOUT_TIME, TIMER_2, USE_TIMER);
}

void measure(Controller_Model * Model){
	paint_info_line(STR_MEASURING, 0);



	// Switch on current supply board
	MEASURE_PIN_ON
	// Get levels
	//			he_level = get_he_level(res_min, res_max, r_span, r_zero, meas_cycles, quench_time, 0);
	LVM.vars->he_level = get_he_level(LVM.options->res_min, LVM.options->res_max, LVM.options->r_span, LVM.options->r_zero, LVM.options->meas_cycles, LVM.options->quench_time, LVM.options->quench_current, LVM.options->wait_time, LVM.options->meas_current, 1);
	LVM.vars->batt_level = get_batt_level(LVM.options->batt_min, LVM.options->batt_max);
	//			pressure_level = (enable_pressure)? map_to_pres(readChannel(PRESSURE, ADC_LOOPS), zero, span) : 0;
	// Switch off current supply board
	MEASURE_PIN_OFF

	// enter time
	if(connected.DS3231M)
	{
		DS3231M_read_time();

	}
	else
	{
		Time.tm_sec  = 0;
		Time.tm_min  = 0;
		Time.tm_hour = 0;
		Time.tm_mday = 0;
		Time.tm_mon  = 0;
		Time.tm_year = 0;
	}

	// enter pressure
	LVM.vars->pressure_level = 0;
	if (HoneywellSSC_status.connected)
	{
		HoneywellSSC_read_pressure();
		if (HoneywellSSC_status.status < 4) LVM.vars->pressure_level = HoneywellSSC_Pressure;
	}



}

void shutdown_LVM(Controller_Model *Model){
	// if not in offline mode make last measurement and send it to server
	if (Model->mode->netstat == online)
	{
		// Wake up XBee module
		xbee_wake_up_plus();
		

		measure(Model);


		if(!(CHECK_ERROR(NETWORK_ERROR) || CHECK_ERROR(NO_REPLY_ERROR))){
			xbee_reconnect();
		}

		// transmit old measurements that could not be sent (if available) to database server
		if ((LVM.measbuff->numberStored) > 0)
		{

			paint_info_line(STR_TRANSMIT_STORED,0);
			_delay_ms(1000);
			paint_info_line(STR_MEASUREMENTS,0);
			_delay_ms(1000);
			sprintf(LVM.temp->infostr,STR_REMAINING,LVM.measbuff->numberStored);
			paint_info_line(LVM.temp->infostr,0);
			_delay_ms(1000);

			while (LVM.measbuff->numberStored > 0)
			{
				#ifdef ALLOW_COM
				if (xbee_send_request(LVM.measbuff->measurements[LVM.measbuff->firstMeas].type, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data_len) != 0xFF)
				{  // there was an answer from the database server, so the data was transmitted
					--LVM.measbuff->numberStored;
					if (LVM.measbuff->firstMeas < (MEASBUFFER_LENGTH-1)) LVM.measbuff->firstMeas = LVM.measbuff->firstMeas + 1;
					else LVM.measbuff->firstMeas = 0;
					paint_info_line(STR_SUCCESSFUL,0);
				}
				else paint_info_line(STR_UN_SUCCESSFUL,0);


				#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				if (xbee_send_message(LVM.measbuff->measurements[LVM.measbuff->firstMeas].type, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data_len))
				{
					// there is no way to know if the data was transmitted correctly, so let's hope for the best....
					CLEAR_ALL(); // Clear all errors
					--LVM.measbuff->numberStored;
					if (LVM.measbuff->firstMeas < (MEASBUFFER_LENGTH-1)) LVM.measbuff->firstMeas = LVM.measbuff->firstMeas + 1;
					else LVM.measbuff->firstMeas = 0;

				}
				#endif
				if((CHECK_ERROR(NETWORK_ERROR)) || (CHECK_ERROR(NO_REPLY_ERROR)))
				break;
				sprintf(LVM.temp->infostr,STR_REMAINING,LVM.measbuff->numberStored);
				paint_info_line(LVM.temp->infostr,0);
				_delay_ms(1000);

			}
		}





		if (LVM.measbuff->numberStored)
		{
			sprintf(LVM.temp->string,"Do you really\nwant to shut down\nthe system?\nThere are still %i\nunsent Messages!",LVM.measbuff->numberStored);
			if (!LCD_Dialog("Unsent Meas.",LVM.temp->string,D_FGC, D_BGC,SHUTDOWN_TIMEOUT_TIME))
			{
				//user pressed no OR timer ran out
					Model->mode->next = ex_main;
					paint_main(Time, 1, PAINT_ALL);
					
					xbee_sleep_plus();
					return;
				
			}
		}

		if (!(CHECK_ERROR(NETWORK_ERROR) || CHECK_ERROR(NO_REPLY_ERROR)))
		{
			collect_and_send_MeasData(LVM.temp->buffer,LOGOUT_MSG);
		}





		// Sleep XBee module
		
		xbee_sleep_plus();
	}

	// Set shutdown pin
	SHUTDOWN
	// PROGRAM ENDS HERE
	_delay_ms(2000);
	//while(1);	// If shutting down does not work, program will stay here
	Model->mode->next = ex_error;
	Model->mode->error = shutdown_failed;
	return;
}

void manage_Xbee_sleep_cycles(Controller_Model *Model){
	//=========================================================================
	// Manage XBee module sleep cycles
	//=========================================================================
	// If XBee is not busy and it is not offline
	if(Model->mode->netstat == online)
	{
		if(xbee_get_sleeping())
		{
			// XBee module is sleeping
			if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
			{


				// enter pressure
				LVM.vars->pressure_level = 0;
				if (HoneywellSSC_status.connected)
				{
					HoneywellSSC_read_pressure();
					if (HoneywellSSC_status.status < 4) LVM.vars->pressure_level = HoneywellSSC_Pressure;
				}

				// wakes up the xbee and sets the timer
				xbee_wake_up_plus();
				

				// Define frame

				uint8_t index =  0;


				LVM.temp->buffer[index++] = (uint16_t) LVM.vars->pressure_level >> 8;
				LVM.temp->buffer[index++] = (uint16_t) LVM.vars->pressure_level;
				LVM.temp->buffer[index++] =  get_status_byte_levelmeter();

				// Try to reconnect to the network
				xbee_reconnect();

				//uint8_t status = 0;
				struct tm  newtime2;

				#ifdef ALLOW_COM
				paint_info_line(STR_SEND_AWAKE_MSG,0);
				_delay_ms(1000);

				if(xbee_send_request(XBEE_ACTIVE_MSG, LVM.temp->buffer, index)!= 0xFF){


					// set time
					newtime2.tm_sec =	LVM.temp->buffer[0];
					newtime2.tm_min =	LVM.temp->buffer[1];
					newtime2.tm_hour =		LVM.temp->buffer[2];
					newtime2.tm_mday =		LVM.temp->buffer[3];
					newtime2.tm_mon =	LVM.temp->buffer[4];
					newtime2.tm_year =		LVM.temp->buffer[5];

					if (connected.DS3231M)
					{
						DS3231M_set_time(&newtime2);
					}

				}
				#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				if (xbee_send_message(XBEE_ACTIVE_MSG, LVM.temp->buffer, index))
				{
					CLEAR_ALL(); // Clear all errors
				}
				#endif


				// transmit old measurements that could not be sent (if available) to database server
				if (!((CHECK_ERROR(NETWORK_ERROR)) || (CHECK_ERROR(NO_REPLY_ERROR))) && ((LVM.measbuff->numberStored) > 0))
				{

					paint_info_line(STR_TRANSMIT_STORED,0);
					_delay_ms(1000);
					paint_info_line(STR_MEASUREMENTS,0);
					_delay_ms(1000);
					sprintf(LVM.temp->infostr,STR_REMAINING,LVM.measbuff->numberStored);
					paint_info_line(LVM.temp->infostr,0);
					_delay_ms(1000);

					while (!((CHECK_ERROR(NETWORK_ERROR)) || (CHECK_ERROR(NO_REPLY_ERROR))) && ((LVM.measbuff->numberStored) > 0))
					{
						#ifdef ALLOW_COM
						if (xbee_send_request(LVM.measbuff->measurements[LVM.measbuff->firstMeas].type, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data_len) != 0xFF)
						{  // there was an answer from the database server, so the data was transmitted
							--LVM.measbuff->numberStored;
							if (LVM.measbuff->firstMeas < (MEASBUFFER_LENGTH-1)) LVM.measbuff->firstMeas = LVM.measbuff->firstMeas + 1;
							else LVM.measbuff->firstMeas = 0;
							paint_info_line(STR_SUCCESSFUL,0);
						}
						else paint_info_line(STR_UN_SUCCESSFUL,0);
						_delay_ms(1000);

						#else
						// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
						if (xbee_send_message(LVM.measbuff->measurements[LVM.measbuff->firstMeas].type, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data_len))
						{
							// there is no way to know if the data was transmitted correctly, so let's hope for the best....
							CLEAR_ALL(); // Clear all errors
							--LVM.measbuff->numberStored;
							if (LVM.measbuff->firstMeas < (MEASBUFFER_LENGTH-1)) LVM.measbuff->firstMeas = LVM.measbuff->firstMeas + 1;
							else LVM.measbuff->firstMeas = 0;

						}
						#endif

						sprintf(LVM.temp->infostr,STR_REMAINING,LVM.measbuff->numberStored);
						paint_info_line(LVM.temp->infostr,0);
						_delay_ms(1000);

					}
				}


				// reset the timeout for awake time
				set_timeout(0, TIMER_5, RESET_TIMER);
				set_timeout(xbee_get_awake_period(), TIMER_5, USE_TIMER);		// Stay active for xbee_awake_period

				
				#ifdef ALLOW_DEBUG
				paint_info_line(STR_XBEE_AWAKE, 0);
				_delay_ms(1000);
				#endif


				paint_info_line("",0);  // Clear info line
			}
		}
		else
		{	// XBee module is awake
			if(!set_timeout(0, TIMER_5, USE_TIMER))		//timer5 not running and returns 0
			{
				// set xbee to sleep, sets the timers and the xbee status variables
				xbee_sleep_plus();
				//					LCD_Print("timeout sleep", 5, 20, 2, 1, 1, FGC, BGC);
			}
		}
	}
}

void interval_slow_changed(Controller_Model *Model){
	//=========================================================================
	// Interval slow changed (Time interval between regular network connections)
	//=========================================================================
	// If time interval between regular network connections has been changed
	// and new option settings have been successfully sent to the database server or it was decided not to send the new settings to the database server
	if(LVM.vars->transmit_slow_changed)
	{
		// Reset timer dedicated to interval slow
		set_timeout(0, TIMER_1, RESET_TIMER);

		// Then set a new timer value in seconds
		(LVM.options->transmit_slow_min)?
		set_timeout(ceil(LVM.options->transmit_slow*60), TIMER_1, USE_TIMER)	// with transmit_slow in minutes
		:	set_timeout(LVM.options->transmit_slow*3600, TIMER_1, USE_TIMER);	// with transmit_slow in hours

		LVM.vars->transmit_slow_changed = false;		// Reset marker
		
	}

}

void Battery_check(Controller_Model *Model)
 {
	//=========================================================================
	// Battery checking
	//=========================================================================
	if(LVM.options->batt_min >= map_to_batt(readChannel(BATTERY, 10*ADC_LOOPS)))
	{
		
		if(Model->mode->netstat == online)
		{
			measure(Model);
			// Send levels to the database server
			xbee_wake_up_plus();
			

			collect_and_send_MeasData(LVM.temp->buffer,LOGOUT_MSG);

			
		}


		sprintf(LVM.temp->string,STR_THE_BATTERY_IS_CRITTICALLY_LOW_SYSTEM_WILL_SHUT_DOWN ,((uint8_t) LVM.vars->batt_level),((uint8_t) LVM.vars->he_level));
		timed_dialog(STR_SHUTTING_DOWN , LVM.temp->string, 10, D_FGC, D_BGC);



		// Switch off the power supply
		SHUTDOWN
		_delay_ms(2000);

		// If pin doesn't work go to mysterious error
		Model->mode->next = ex_error;
		Model->mode->error = shutdown_failed;

	}

}


// Measure liquid He level
double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double quench_time, double quench_current, double wait_time, double meas_current, uint8_t show_progress)
{

	double u_val = 0, i_val = 0;				// Initialize voltage and current variables
	//	uint16_t u_val_temp = 0, i_val_temp = 0;	// Initialize voltage and current buffer variables
	
	uint16_t quench_time_ms = quench_time*1000;				// Converts seconds to ms
	uint16_t wait_time_ms	= wait_time*1000;				// Converts seconds to ms
	//	uint8_t steps_sum 		= floor(((double) ((quench_time_ms+wait_time)/OVERFLOW_IN_MS_8_BIT)));	//118.x floor: 118
	//	uint8_t	step 			= floor(steps_sum/9.0);			//13
	//	uint16_t current_count = 0;
	

	//=========================================================================
	// Display progress bar
	//=========================================================================

	if(show_progress)
	{
		paint_progress_bar(xoff+X_GHL_5, Y_GHL_105, 1);
	}

	
	//=========================================================================
	//PWM-Configuration to control measuring current
	//=========================================================================
	
	// Start PWM
	TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (0 << WGM22) | (1 << CS22) | (0 << CS21) | (1 << CS20);
	DDRD |= (1 << PD7);			// Set PORTD.7 as output

	// Switch on current supply to quench_current to quench the He probe
	// Calculation of the corrected set value for the current with the constants given in main.h
	OCR2A = (uint8_t)round(quench_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));


	//init_0_timer8();	// Enable Timer interrupt
	set_timeout(quench_time_ms, TIMER_0, USE_TIMER);
	while(set_timeout(0, TIMER_0, USE_TIMER))
	{
	}

	if(show_progress)
	{
		paint_progress_bar(xoff+X_GHL_5, Y_GHL_105, floor(10*quench_time/(quench_time+wait_time)));
	}
	
	// Set current supply to meas_current for the measurement
	// Calculation of the corrected set value for the current with the constants given in main.h
	OCR2A = (uint8_t)round(meas_current*(double)(SET_CURRENT_FACTOR)+(double)(SET_CURRENT_OFFSET));
	set_timeout(wait_time_ms, TIMER_0, USE_TIMER);
	while(set_timeout(0, TIMER_0, USE_TIMER))
	{
	}

	if(show_progress)
	{
		paint_progress_bar(xoff+X_GHL_5, Y_GHL_105, 9);
	}

	for(uint8_t i=0;i < count; ++i)
	{
		
		u_val += readChannel(VOLT_PROBE_MEAS, ADC_LOOPS); // VOLT_PROBE_MEAS =2 / ADC_LOOPS=10 /
		i_val += readChannel(CURRENT_PROBE_MEAS, ADC_LOOPS); // CURRENT_PROBE_MEAS = 1
		//		u_val  += readChannel_calib(VOLT_PROBE_MEAS, ADC_LOOPS, r_zero); // VOLT_PROBE_MEAS =2 / ADC_LOOPS=10 /
		//		i_val  += readChannel_calib(CURRENT_PROBE_MEAS, ADC_LOOPS, r_zero); // CURRENT_PROBE_MEAS = 1
		
		
	}
	// Stop PWM
	DDRD &= (0 << PD7);			// Set PORTD.7 as input

	if(show_progress)	paint_progress_bar(xoff+X_GHL_5, Y_GHL_105, 10);

	u_val = (double) u_val/count;		// Calculate the averaged voltage
	i_val = (double) i_val/count;		// Calculate the averaged current
	
	if(i_val <= 0) i_val=1;
	
	//double r_val = ((double) u_val/i_val)*r_span-10;
	
	
	// Voltage reference 3.34 V measured on the PCB
	// On PA1, R18 & R19 divide the voltage probe (82500+3740)/3740=23.06
	// On PA2, we measured 952 mV for 81,36 mA => 81.36/952 = 0.08546 mA/mV
	// double r_val = ((u_val*3.34/1024)*23.06)/((i_val*3.34/1024)*0.08546);
	
	double r_val = 0;
	

	// Calculate resistance with correction
	if (map_to_current(i_val) > 10) // > current should be > 10 mA, else broken cable?
	r_val = (r_span * map_to_volt(u_val) * 1000 / map_to_current(i_val)) + r_zero;  // factor 1000:  mA to A
	else
	r_val = 6000;  // Any cable connected
	
	
	LVM.vars->r_val_last_Meas = r_val;
	
	// Added by JG for debugging
	#ifdef ALLOW_DEBUG
	char str_temp[50];
	sprintf(str_temp,"i: %d / u: %d / res: %d\n", (int)i_val, (int)u_val, (int)r_val);
	LCD_Print(str_temp, xoff+X_GHL_5, Y_GHL_90, 1, 1, 1, FGC, black);
	_delay_ms(3000);
	#endif

	
	return calc_he_level(r_val, res_min, res_max);

}


void addline_number(char * text,uint32_t number){
	sprintf(LVM.temp->string,"%s: %lu",text,number);
	
	InitScreen_AddLine(LVM.temp->string,0);
}

void set_Options(uint8_t * optBuffer,uint8_t OpCode){

	optionsType OptBuff;
	// set time
	if (connected.DS3231M)
	{
		struct tm newtime;

		newtime.tm_sec=   optBuffer[0];
		newtime.tm_min =  optBuffer[1];
		newtime.tm_hour = optBuffer[2];
		newtime.tm_mday = optBuffer[3];
		newtime.tm_mon =  optBuffer[4];
		newtime.tm_year = optBuffer[5];

		DS3231M_set_time(&newtime);
	}
	/*
	addline_number("seconds",optBuffer[0]);
	addline_number("min",optBuffer[1]);
	addline_number("hour",optBuffer[2]);
	addline_number("day",optBuffer[3]);
	addline_number("mon",optBuffer[4]);
	addline_number("year",optBuffer[5]);
	*/
	
	OptBuff.transmit_slow    =   (optBuffer[6]<<8)  + optBuffer[7];
	OptBuff.transmit_fast    =   (optBuffer[8]<<8)  + optBuffer[9];
	OptBuff.res_min          =  ((optBuffer[10]<<8) + optBuffer[11])/10.0;
	OptBuff.res_max          =  ((optBuffer[12]<<8) + optBuffer[13])/10.0;
	OptBuff.quench_time      =  ((optBuffer[14]<<8) + optBuffer[15])/1000.0;
	OptBuff.quench_current   =   (optBuffer[16]<<8) + optBuffer[17];
	OptBuff.wait_time        =  ((optBuffer[18]<<8) + optBuffer[19])/1000.0;
	OptBuff.meas_current     =   (optBuffer[20]<<8) + optBuffer[21];
	OptBuff.meas_cycles      =   optBuffer[22];
	OptBuff.fill_timeout     =  optBuffer[23];
	OptBuff.span             = ((optBuffer[24]<<8) + optBuffer[25])/10.0;
	OptBuff.zero             = ((double)(((int16_t) optBuffer[26] << 8) | optBuffer[27]))/10;
	OptBuff.total_volume     = ((optBuffer[28]<<8) + optBuffer[29])/10.0;
	OptBuff.he_min           =  optBuffer[30];
	OptBuff.display_reversed = optBuffer[31];
	OptBuff.batt_min         = ((optBuffer[32]<<8) + optBuffer[33])/10.0;
	OptBuff.batt_max         = ((optBuffer[34]<<8) + optBuffer[35])/10.0;
	OptBuff.critical_batt    =  optBuffer[36];
	
	
	// duplicate options that are only in eeprom
	OptBuff.r_span          = LVM.options->r_span;
	OptBuff.r_zero          = LVM.options->r_zero;
	OptBuff.enable_pressure = LVM.options->enable_pressure;
	OptBuff.Dev_ID_alpahnum = LVM.options->Dev_ID_alpahnum;
	OptBuff.Dev_ID_Max_len  = LVM.options->Dev_ID_Max_len;
	
	
	uint8_t out_of_bounds =  Options_Buonds_Check(&OptBuff);
	
	OptBuff.transmit_slow_min = true;
	OptBuff.transmit_fast_sec = true;
	
	if(OptBuff.transmit_slow > 60)
	{
		OptBuff.transmit_slow_min = false;
		OptBuff.transmit_slow /=60;
	}
	
	if (OptBuff.transmit_fast > 60)
	{
		OptBuff.transmit_fast_sec = false;
		OptBuff.transmit_fast /=60;
	}
	

	
	switch (OpCode)
	{
		case LOGIN_MSG:
		case FORCE_LOGIN_MSG:
		
		OptBuff.options_pw =  (optBuffer[37]<<8) + optBuffer[38];
		
		xbee_set_sleep_period(optBuffer[39]);
		xbee_set_awake_period(optBuffer[40]);
		
		
		break;
		
		case SET_OPTIONS_CMD:
		
		LVM.temp->buffer[0] = out_of_bounds;
		
		if (OptBuff.transmit_slow != LVM.options->transmit_slow)
		{
			LVM.vars->transmit_slow_changed = true;
		}
		
		if (OptBuff.transmit_fast!= LVM.options->transmit_fast)
		{
			LVM.vars->transmit_fast_changed = true;
		}
		
		xbee_send_message(SET_OPTIONS_CMD,LVM.temp->buffer,1);
		
		if(out_of_bounds){
			// device keeps running with its original options
			return;
		}
		
		
		break;
		
		default:
		break;
	}
	
	// copy newly received and tested  options into operational options struct
	
	DISPLAY_CONFIG
	xoff = (!LVM.options->display_reversed)? 0 : XOffset;
	memcpy(LVM.options,&OptBuff,sizeof(optionsType));


}

uint8_t write_Opts_to_Buffer(uint8_t * sendbuff){
	
	uint8_t index = 0;


	if(LVM.options->transmit_slow_min)
	{
		sendbuff[index++] = LVM.options->transmit_slow>>8;
		sendbuff[index++] = LVM.options->transmit_slow;
	}
	else
	{
		sendbuff[index++] = (LVM.options->transmit_slow*60)>>8;
		sendbuff[index++] = LVM.options->transmit_slow*60;
	}
	if(LVM.options->transmit_fast_sec)
	{
		sendbuff[index++] = LVM.options->transmit_fast>>8;
		sendbuff[index++] = LVM.options->transmit_fast;
	}
	else
	{
		sendbuff[index++] = (LVM.options->transmit_fast*60)>>8;
		sendbuff[index++] = LVM.options->transmit_fast*60;
	}
	sendbuff[index++] = ((uint16_t)(LVM.options->res_min*10))>>8;
	sendbuff[index++] = round(LVM.options->res_min*10);
	sendbuff[index++] = ((uint16_t)(LVM.options->res_max*10))>>8;
	sendbuff[index++] = round(LVM.options->res_max*10);

	sendbuff[index++] = ((uint16_t)(LVM.options->quench_time*1000))>>8;
	sendbuff[index++] = round(LVM.options->quench_time*1000);
	sendbuff[index++] = ((uint16_t)LVM.options->quench_current)>>8;
	sendbuff[index++] = round(LVM.options->quench_current);
	sendbuff[index++] = ((uint16_t)(LVM.options->wait_time*1000))>>8;
	sendbuff[index++] = round(LVM.options->wait_time*1000);
	sendbuff[index++] = ((uint16_t)LVM.options->meas_current)>>8;
	sendbuff[index++] = round(LVM.options->meas_current);

	sendbuff[index++] = LVM.options->meas_cycles;
	sendbuff[index++] = LVM.options->fill_timeout;
	

	
	sendbuff[index++] = ((uint16_t)(LVM.options->span*10))>>8;
	sendbuff[index++] = round(LVM.options->span*10);

	
	int16_t zero16 = (int16_t)(LVM.options->zero*10);
	sendbuff[index++] = zero16 >> 8;
	sendbuff[index++] = (uint8_t) round(zero16) ;

	sendbuff[index++] = LVM.options->he_min;
	sendbuff[index++] = LVM.options->display_reversed;

	sendbuff[index++] = ((uint16_t)(LVM.options->batt_min*10))>>8;
	sendbuff[index++] = round(LVM.options->batt_min*10);
	sendbuff[index++] = ((uint16_t)(LVM.options->batt_max*10))>>8;
	sendbuff[index++] = round(LVM.options->batt_max*10);
	sendbuff[index++] = LVM.options->critical_batt;
	sendbuff[index++] =  get_status_byte_levelmeter();
	
	return index;
}

uint8_t Options_Buonds_Check(optionsType * optBuff){
	uint8_t Val_outof_Bounds = 0;
	CHECK_BOUNDS(optBuff->transmit_slow,TRANSMIT_SLOW_MIN,TRANSMIT_SLOW_MAX*60,TRANSMIT_SLOW_DEF*60,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->transmit_fast,TRANSMIT_FAST_MIN,TRANSMIT_FAST_MAX*60,TRANSMIT_FAST_DEF*60,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->quench_time,QUENCH_TIME_MIN,QUENCH_TIME_MAX,QUENCH_TIME_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->quench_current,QUENCH_CURRENT_MIN,QUENCH_CURRENT_MAX,QUENCH_CURRENT_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->wait_time,WAIT_TIME_MIN,WAIT_TIME_MAX,WAIT_TIME_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->meas_current,MEAS_CURRENT_MIN,MEAS_CURRENT_MAX,MEAS_CURRENT_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->meas_cycles,MEASUREMENT_CYCLES_MIN,MEASUREMENT_CYCLES_MAX,MEASUREMENT_CYCLES_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->fill_timeout,MIN_FILLING_TIMEOUT,MAX_FILLING_TIMEOUT,FILLING_TIMEOUT_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->he_min,MIN_AUTO_FILL_HE,MAX_AUTO_FILL_HE,AUTO_FILL_HE_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->res_min,RES_MIN_MIN,RES_MIN_MAX,RES_MIN_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->res_max,RES_MAX_MIN,RES_MAX_MAX,RES_MAX_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->span,MIN_SPAN,MAX_SPAN,SPAN_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->zero,MIN_ZERO,MAX_ZERO,ZERO_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->batt_min,BATT_MIN_MIN,BATT_MIN_MAX,BATT_MIN_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->batt_max,BATT_MAX_MIN,BATT_MAX_MAX,BATT_MAX_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->critical_batt,CRITICAL_BATT_MIN,CRITICAL_BATT_MAX,CRITICAL_BATT_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->total_volume,TOTAL_VOL_MIN,TOTAL_VOL_MAX,TOTAL_VOL_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->Dev_ID_Max_len,DEV_ID_CHARS_MIN,DEV_ID_CHARS_MAX,DEV_ID_CHARS_DEF,Val_outof_Bounds)
	
	CHECK_BOUNDS(optBuff->Dev_ID_Max_len,DEV_ID_CHARS_MIN,DEV_ID_CHARS_MAX,DEV_ID_CHARS_DEF,Val_outof_Bounds)
	CHECK_BOUNDS(optBuff->options_pw,OPTIONS_PW_MIN,OPTIONS_PW_MAX,OPTIONS_PW_DEF,Val_outof_Bounds)
	
	if(optBuff->r_span <= 0){
		optBuff->r_span = R_SPAN_DEF;
		Val_outof_Bounds = 1;
	}
	
	

	return Val_outof_Bounds;
}

//TODO --> clean up
void handle_received_Messages(Controller_Model *Model){
	//=========================================================================
	// Check receive buffer for commands from database server
	//=========================================================================

	// XBee is  offline do nothing
	if(Model->mode->netstat == offline)
	{
		return;
	}
	


	uint8_t reply_Id = xbee_hasReply(LAST_NON_CMD_MSG, GREATER_THAN);
	
	if(reply_Id != 0xFF) 		// Always check for command in buffer and do one at a time
	{
		

		switch(frameBuffer[reply_Id].type)
		{
			case TRIGGER_MEAS_CMD:	// (#24) Send levels and status to the database server.


			// Wake up XBee module
			xbee_wake_up_plus();
			

			// Turn display on
			display_on();

			measure(Model);

			collect_and_send_MeasData(LVM.temp->buffer,TRIGGER_MEAS_CMD);

			

			// Update display
			switch(Model->mode->curr)
			{
				case ex_main:
				paint_main( Time, Model->mode->netstat, PAINT_ALL);
				break;
				case ex_filling:
				paint_filling( Time, Model->mode->netstat, 0, 1);
				break;
				default:
				break;
			}	// End switch

			break;
			case GET_OPTIONS_CMD: ;	// (#12) Send device settings to the database server.

			uint8_t index = write_Opts_to_Buffer(LVM.temp->buffer);


			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(GET_OPTIONS_CMD, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}
			break;
			case SET_OPTIONS_CMD:	// (#13) Set device settings received from the database server.
			if (NUMBER_OPTIONS_BYTES == frameBuffer[reply_Id].data_len)
			{
				set_Options((uint8_t*)frameBuffer[reply_Id].data,SET_OPTIONS_CMD);
				// Save settings in EEPROM
				#ifdef ALLOW_EEPROM_SAVING
				write_opts_to_EEPROM();

				#endif

				//GOOD_OPTIONS:
				InitScreen_AddLine(STR_GOOD_OPTIONS_RECEIVED, 1);
				InitScreen_AddLine(STR_PRESS_MEASURE_X2, 0);
				InitScreen_AddLine(STR_TO_CONTINUE, 0);
				_delay_ms(1000);
				LVM.message->Received = true;
				break;
			}

			//BAD_OPTIONS:
			InitScreen_AddLine(STR_BAD_OPTS_RECEIVED, 1);
			InitScreen_AddLine(STR_DEFAULT_OPTS_SET, 0);
			InitScreen_AddLine(STR_PRESS_MEASURE_X2, 0);
			InitScreen_AddLine(STR_TO_CONTINUE, 0);
			LVM.message->Received = true;
			_delay_ms(1000);
			
			break;
			case GET_LETTERS_CMD:	// (#14) Send list of available device positions to the database server.
			{
				char *ptr;
				uint8_t index = 1;
				uint8_t i = 0;
				ptr = &LVM.pos->Strings[index][0];



				while(*ptr != '\r')
				{
					//ptr = fill_pos_letters[index][0];
					while(*ptr != '\0')
					{
						LVM.temp->buffer[i++] = *ptr++;
					}
					LVM.temp->buffer[i++] = ';';
					ptr = ((char*) &LVM.pos->RangeNums[index][1]);

					while(*ptr != END)
					{
						LVM.temp->buffer[i++] = ((char)(*ptr++));
					}
					LVM.temp->buffer[i++] = SEP;

					index++;
					ptr = &LVM.pos->Strings[index][0];
				}
				LVM.temp->buffer[i++] =  get_status_byte_levelmeter();

				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				if (xbee_send_message(GET_LETTERS_CMD, LVM.temp->buffer, i))
				{
					CLEAR_ALL(); // Clear all errors
				}

				#ifdef ALLOW_DEBUG
				paint_info_line(STR_POSITIONS_SENT, 0);
				_delay_ms(1000);
				#endif

				break;
			}
			case SET_LETTERS_CMD:	// (#15) Set list of available device positions received from the database server.
			{
				
	
				// Get data length and data
				uint8_t byte_number = frameBuffer[reply_Id].data_len;
				uint8_t *ptr = (uint8_t*)frameBuffer[reply_Id].data;

				// Init. variables
				uint8_t i_letters = 1;
				uint8_t j_letters = 0;
				uint8_t j_ranges = 1;

				// Init. arrays
				LVM.pos->Strings[0][0] ='\r';
				LVM.pos->Strings[0][1] ='\0';
				LVM.pos->RangeNums[0][0] = END;
				LVM.pos->StrLen[0] = 0;

				// Example for a position string: 'A;'#1#2#3'/AB;'#1#2'/ABC;/ISI;/P;'#1#2#5'/PPMS;/QWR;/R;'#2#$17#3#4'/T;'#1#$A#$B#2#4'/'

				while(byte_number)
				{
					LVM.pos->Strings[i_letters][j_letters] = ((char)(*ptr));
					j_letters++;
					ptr++;
					byte_number--;
					LVM.pos->StrLen[i_letters] = 0;    // no composed position string until now
					//							draw_int(fill_pos_tonumbersindex[i_letters], 65, 20, "", ERR);
					//							_delay_ms(200);

					if(*ptr == ';') //ranges
					{
						LVM.pos->Strings[i_letters][j_letters] = '\0';
						LVM.pos->RangeNums[i_letters][0] = END;
						ptr++;
						byte_number--;

						while(*ptr != SEP)
						{
							LVM.pos->RangeNums[i_letters][j_ranges] = *ptr;
							LVM.pos->StrLen[i_letters] = j_letters;   // composed position string detected, enter number of letters in the composed position string
							//									draw_int(fill_pos_tonumbersindex[i_letters], 65, 20, "", ERR);
							//									_delay_ms(200);
							j_ranges++;
							ptr++;
							byte_number--;
							// If ranges index is higher than the maximum value, break
							if(j_ranges == FILL_RANGES_LENGTH-1)
							break;
						}
						LVM.pos->RangeNums[i_letters][j_ranges] = END;

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
				LVM.pos->Strings[i_letters][0] ='\r';
				LVM.pos->RangeNums[i_letters][0] = END;
				LVM.pos->StrLen[i_letters] = 0;

				CLEAR_ERROR(LETTERS_ERROR);
				
				//Send Status Ack
				LVM.temp->buffer[0] = 0;
				xbee_send_message(SET_LETTERS_CMD,LVM.temp->buffer,1);

				#ifdef ALLOW_DEBUG
				paint_info_line(STR_POS_WRITTEN , 0);
				_delay_ms(1000);
				#endif

				break;
			}
			case GET_PASSWORD_CMD: ;	// (#16) Send options password to the database server. This password is required to access to settings pages.

			index = 0;
			LVM.temp->buffer[index++] = LVM.options->options_pw >> 8;
			LVM.temp->buffer[index++] = LVM.options->options_pw;
			LVM.temp->buffer[index++] =  get_status_byte_levelmeter();

			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(GET_PASSWORD_CMD, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}
			#ifdef ALLOW_DEBUG
			paint_info_line(STR_PASSW_SENT , 0);
			_delay_ms(1000);
			#endif
			break;
			case SET_PASSWORD_CMD:	// (#17) Set options password received from the database server. This password is required to access to settings pages.
			LVM.options->options_pw = (frameBuffer[reply_Id].data[0]<<8) + frameBuffer[reply_Id].data[1];
			
			//Send Status Ack
			LVM.temp->buffer[0] = 0;
			xbee_send_message(SET_PASSWORD_CMD,LVM.temp->buffer,1);
			
			#ifdef ALLOW_DEBUG
			sprintf(LVM.temp->infostr,STR_NEW_PASSW ,LVM.options->options_pw);
			paint_info_line(LVM.temp->infostr, 0);
			_delay_ms(1000);
			#endif
			break;
			case GET_XBEE_SLEEP_TIME_CMD: ;	// (#18) Send XBee sleeping period to the database server

			index = 0;

			LVM.temp->buffer[index++] = xbee_get_sleep_period();		// Sleeping period in minute
			LVM.temp->buffer[index++] =  get_status_byte_levelmeter();

			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(GET_XBEE_SLEEP_TIME_CMD, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}
			#ifdef ALLOW_DEBUG
			paint_info_line(STR_SLEEP_TIME_SENT, 0);
			_delay_ms(1000);
			#endif
			break;

			case SET_XBEE_SLEEP_TIME_CMD:	// (#19) Set XBee sleeping period received from the database server
			xbee_set_sleep_period(frameBuffer[reply_Id].data[0]);
			
			
			//Send Status Ack
			LVM.temp->buffer[0] = 0;
			xbee_send_message(SET_XBEE_SLEEP_TIME_CMD,LVM.temp->buffer,1);
			
			#ifdef ALLOW_DEBUG
			sprintf(LVM.temp->infostr,STR_XBEE_SLEEP ,xbee_get_sleep_period());
			paint_info_line(LVM.temp->infostr, 0);
			_delay_ms(1000);
			#endif
			break;


			case DISPLAY_MSG_CMD:	// (#20) Display message received from the database server
			LVM.message->Received = true;
			memcpy(LVM.message->Str,(uint8_t*)&frameBuffer[reply_Id].data[1],19);
			LVM.message->type = frameBuffer[reply_Id].data[0];
			paint_Notification(LVM.message);
			break;


			case GET_XBEE_AWAKE_TIME_CMD:	;// (#21) Send XBee awake period to the database server

			index = 0;

			LVM.temp->buffer[index++] = xbee_get_awake_period();		// Awake period in seconds
			LVM.temp->buffer[index++] =  get_status_byte_levelmeter();

			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(GET_XBEE_AWAKE_TIME_CMD, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}

			#ifdef ALLOW_DEBUG
			paint_info_line(STR_AWAKE_TIME_SENT, 0);
			_delay_ms(1000);
			#endif
			break;


			case SET_XBEE_AWAKE_TIME_CMD:	// (#22) Set XBee awake period received from the database server
			xbee_set_awake_period((frameBuffer[reply_Id].data[0]>10) ? frameBuffer[reply_Id].data[0] : 10);//minimum 10s
			
			
			//Send Status Ack
			LVM.temp->buffer[0] = 0;
			xbee_send_message(SET_XBEE_AWAKE_TIME_CMD,LVM.temp->buffer,1);

			#ifdef ALLOW_DEBUG
			sprintf(LVM.temp->infostr,STR_AWAKE_s,xbee_get_awake_period());
			paint_info_line(LVM.temp->infostr, 0);
			_delay_ms(1000);
			#endif
			break;

			case TRIGGER_REMOTE_PULSE:
			{
				diag_pulseType dp;
				
				pulse_select_Init();
				
				
				
				diag_pulse_init(&dp ,1, NORMAL);
				diag_pulse_Measure(&dp);
				diag_pulse_send(&dp);


				// Update display
				switch(Model->mode->curr)
				{
					case ex_main:
					paint_main( Time, Model->mode->netstat, PAINT_ALL);
					break;
					case ex_filling:
					paint_filling( Time, Model->mode->netstat, 0, 1);
					break;
					default:
					break;
				}	// End switch

				break;
			}
			case  TRIGGER_REMOTE_U_OVER_I:
			{
				
				
				
				_delay_ms(1000);
				diag_pulseType dp_u_i;
				
				pulse_select_Init();
				uint8_t i_min = frameBuffer[reply_Id].data[0];
				uint8_t i_max = frameBuffer[reply_Id].data[1];
				uint8_t delta_i = frameBuffer[reply_Id].data[2];
				uint8_t delta_t = frameBuffer[reply_Id].data[3];
				
				uint16_t pulse_duration = ((frameBuffer[reply_Id].data[2]<<8) + frameBuffer[reply_Id].data[3]);
				
				if (!pulse_seclect_set_linear_params(&dp_u_i,i_min,i_max,delta_i, delta_t, pulse_duration))
				{

					break;
				}
				

				diag_pulse_Measure(&dp_u_i);
				diag_pulse_send(&dp_u_i);


				// Update display
				switch(Model->mode->curr)
				{
					case ex_main:
					paint_main(Time, Model->mode->netstat, PAINT_ALL);
					break;
					case ex_filling:
					paint_filling(Time, Model->mode->netstat, 0, 1);
					break;
					default:
					break;
				}	// End switch
				
				break;
			}
			case DEPRECATED_ILM_BROADCAST: // ILM messages are sent in broadcast mode and are ignored by other devices
			break;

			default:	// (#10) Unknown command, send error code to the database server
			//					buffer[0] = status_byte;
			LVM.temp->buffer[0] =  frameBuffer[reply_Id].type;
			memcpy(&LVM.temp->buffer[1],(uint8_t*)frameBuffer[reply_Id].data,frameBuffer[reply_Id].data_len);
			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(UNKNOWN_MSG, LVM.temp->buffer, 1+frameBuffer[reply_Id].data_len))
			{
				CLEAR_ALL(); // Clear all errors
			}
		}//switch(frameBuffer[reply_Id].type)
		buffer_removeData(reply_Id);	//mark cmd as done !
	}//if buffersize

}