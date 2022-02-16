/*
* CFile1.c
*
* Created: 23.02.2021 16:40:21
*  Author: Weges
*/
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "base_controller.h"
#include "main_mode.h"
#include "option_mode.h"
#include "getCode_mode.h"
#include "../display_driver.h"
#include "../display_utilities.h"
#include "../HoneywellSSC.h"
#include "../main.h"
#include "../keyboard.h"
#include "../timer_utilities.h"

#include "xbee.h"
#include "xbee_utilities.h"
#include "I2C_utilities.h"
#include "status.h"
#include "DS3231M.h"


#ifdef DISP_3000
#include "../StringPixelCoordTable.h"
#endif

#ifdef ili9341
#include "../StringPixelCoordTable_ili9341.h"
#endif


struct Controller_Model_Vtable main_Vtable ={
	&main_pressedFILL,
	&main_pressedMeasure,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&main_pressedLEFT,
	&main_pressedRIGHT,
	&main_pressedDOWN,
	&main_pressedBOT,
	&main_pressedNONE,
	&base_display_on_pressed,
	&base_display_off_pressed,
	&main_pre_Switch_case_Tasks,
	&main_post_Switch_case_Tasks
};

Controller_Model_main main_model ={
	.super.mode = &global_mode,
	.super.vtable = &main_Vtable
};


Controller_Model_main* get_main_model(){
	return &main_model;
}


// start Filling
void main_pressedFILL(Controller_Model *Model){
	

	
	
	if (Model->mode->netstat== online)
	{
		
		if (LVM.message->Received)
		{
			return;
		}

		strcpy(LVM.temp->string, "none");
		if(!update_filling_pos(LVM.pos, LVM.temp->string))
		{
			paint_main(Time,Model->mode->netstat, PAINT_ALL);
			keyhit_block();
			return;
		}

		//debug
		//									LCD_Print(temp, 40, 50, 2, 2, 3, FGC, BGC);
		//									_delay_ms(2000);
		// Copy position bytes from temp to device_pos (each digit is a byte)
		strcpy(LVM.vars->device_pos, LVM.temp->string);
		display_on();    // otherwise the display would turn black just after returning



		// Wake up XBee
		xbee_wake_up_plus();
		

		// Clear display and inform about the sending
		InitScreen_AddLine(STR_SENDING_INFO,1);
		InitScreen_AddLine(STR_START_FILLING,0);
		InitScreen_AddLine(STR_TO_THE_SERVER,0);

		// Pack frame (including device id, device position and status)
		uint8_t index = 0;


		index = devicePos_to_buffer(LVM.vars->device_pos, index, LVM.temp->buffer);  // Positions are 4 letters
		LVM.temp->buffer[index++] =  get_status_byte_levelmeter();

		#ifdef ALLOW_COM
		// Send packed message and get an answer
		if (xbee_send_request(FILLING_BEGIN_MSG, LVM.temp->buffer, index) == 0xFF)
		InitScreen_AddLine(STR_UN_SUCCESSFUL,0);
		else InitScreen_AddLine(STR_SUCCESSFUL,0);

		// comment: message is not stored if communication to database server is not working
		#else
		// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
		if (xbee_send_message(FILLING_BEGIN_MSG, LVM.temp->buffer, index))
		{
			CLEAR_ALL(); // Clear all errors
		}
		#endif

		// for reading the message
		_delay_ms(2000);

		

	} // end if not offline


	// Number of measurements
	LVM.vars->fill_meas_counter = (LVM.options->transmit_fast_sec)?
	((((uint16_t) LVM.options->fill_timeout*60))/LVM.options->transmit_fast) +1
	:(LVM.options->fill_timeout/LVM.options->transmit_fast) + 1;

	paint_filling(Time, Model->mode->netstat, 0, 0);

	
	
	// Set filling mode
	Model->mode->next = ex_filling;
	keyhit_block();
	
}

// make measurement
void main_pressedMeasure(Controller_Model *Model){

	
	if (LVM.message->Received)
	{

		LVM.message->Received = false;
		for(uint8_t i = 5; i > 0 ;i--)
		{
			sprintf(LVM.temp->string,"%i",i);
			LCD_Print(LVM.temp->string, X_M_150, Y_M_5,2,2,2,FGC,BGC);
			_delay_ms(1000);
		}

		paint_main(Time, Model->mode->netstat, PAINT_ALL);


		return;
	}
	
	// Wake up XBee module if not in offline mode
	if(Model->mode->netstat == online)
	{
		xbee_wake_up_plus();
		
	}

	measure(Model);
	
	paint_main(Time,Model->mode->netstat,UPDATE_ONLY);
	
	// Transmit data to server if not in offline mode
	if (Model->mode->netstat == online)
	{
		// Pack data in frame
		xbee_reconnect();
		
		collect_and_send_MeasData(LVM.temp->buffer,STATUS_MSG);

		// Free XBee module
		

		// clear info line
		paint_info_line("",0);
	}
	
	return;
	
}




// request shutdown
void main_pressedLEFT(Controller_Model *Model){
	if (LVM.message->Received)
	{
		return;
	}

	uint8_t status;	// Pressed "cancel" or "ok"
	LVM.vars->entered_options_pw = get_Password(&status);

	if(status || (LVM.vars->entered_options_pw != LVM.options->options_pw))
	{
		// Canceled or wrong password, go back to the main mode
		paint_main(Time, Model->mode->netstat, PAINT_ALL);
		keyhit_block();
		return;
	}

	if(LCD_Dialog(STR_SHUTDOWN_OPT,STR_DO_YOU_REALLY_WANT_SHUTDOWN, D_FGC, D_BGC,SHUTDOWN_TIMEOUT_TIME))
	{
		// Confirmed shutting down, set shutdown action
		LCD_Cls(BGC);

		//SHUTDOWN
		shutdown_LVM(Model);
		return;
	}
	else
	{
		// Canceled, go back to the main mode
		paint_main(Time, Model->mode->next, PAINT_ALL);
		keyhit_block();
		return;
	}
}

// request shutdown
void main_pressedRIGHT(Controller_Model *Model){
	// do the same as in pressed left --> shutdown request
	main_pressedLEFT(Model);
}





// reconnect to Server
void main_pressedDOWN(Controller_Model *Model){
	if (LVM.message->Received)
	{
		return;
	}

	if (Model->mode->netstat == online)
	{
		InitScreen_AddLine(STR_NETWORK_CONN,1);
		InitScreen_AddLine(STR_IN_PROGRESS,0);



		// enter pressure
		LVM.vars->pressure_level = 0;
		if (HoneywellSSC_status.connected)
		{
			HoneywellSSC_read_pressure();
			if (HoneywellSSC_status.status < 4) LVM.vars->pressure_level = HoneywellSSC_Pressure;
		}

		xbee_wake_up_plus();
		

		// Define frame

		uint8_t index = 0;

		LVM.temp->buffer[index++] = (uint16_t) LVM.vars->pressure_level >> 8;
		LVM.temp->buffer[index++] = (uint16_t) LVM.vars->pressure_level;
		LVM.temp->buffer[index++] =  get_status_byte_levelmeter();



		// Try to reconnect to the network
		xbee_reconnect();

		(CHECK_ERROR(NETWORK_ERROR))? 	InitScreen_AddLine(STR_UN_SUCCESSFUL,0) : InitScreen_AddLine(STR_SUCCESSFUL,0) ;
		
		//_delay_ms(2000);

		if (!CHECK_ERROR(NETWORK_ERROR))
		{

			//uint8_t status = 0;
			#ifdef ALLOW_COM
			InitScreen_AddLine(STR_AWAKE_MESSAGE_SENT,0);
			InitScreen_AddLine(STR_TO_SERVER_DOT,0);






			if(xbee_send_request(XBEE_ACTIVE_MSG, LVM.temp->buffer, index)!= 0xFF){
				InitScreen_AddLine(STR_NEW_TIME_RECEIVED,0);

				// set time
				struct tm newtime2;
				newtime2.tm_sec =	LVM.temp->buffer[0];
				newtime2.tm_min =	LVM.temp->buffer[1];
				newtime2.tm_hour =		LVM.temp->buffer[2];
				newtime2.tm_mday =		LVM.temp->buffer[3];
				newtime2.tm_mon =	LVM.temp->buffer[4];
				newtime2.tm_year =		LVM.temp->buffer[5];

				// Message on screen
				sprintf(LVM.temp->string,STR_NEW_DATE, newtime2.tm_mday, newtime2.tm_mon, newtime2.tm_year+2000);
				InitScreen_AddLine(LVM.temp->string,0);
				sprintf(LVM.temp->string,STR_NEW_TIME, newtime2.tm_hour, newtime2.tm_min, newtime2.tm_sec);
				InitScreen_AddLine(LVM.temp->string,0);

				if (connected.DS3231M)
				{
					DS3231M_set_time(&newtime2);
				}

			}
			else
			{
				InitScreen_AddLine(STR_UN_SUCCESSFUL,0);
			}



			#else
			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(XBEE_ACTIVE_MSG, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}
			#endif

		}





		// transmit old measurements that could not be sent (if available) to database server
		if (!((CHECK_ERROR(NETWORK_ERROR)) || (CHECK_ERROR(NO_REPLY_ERROR))) && ((LVM.measbuff->numberStored) > 0))
		{
			InitScreen_AddLine(STR_TRANSMITTING_STORED,1);
			InitScreen_AddLine(STR_MEASUREMENTS ,0);
			sprintf(LVM.temp->string,STR_REMAINING,LVM.measbuff->numberStored);
			InitScreen_AddLine(LVM.temp->string,0);

			while (!((CHECK_ERROR(NETWORK_ERROR)) || (CHECK_ERROR(NO_REPLY_ERROR))) && ((LVM.measbuff->numberStored) > 0))
			{

				#ifdef ALLOW_COM

				if (xbee_send_request(LVM.measbuff->measurements[LVM.measbuff->firstMeas].type, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data, LVM.measbuff->measurements[LVM.measbuff->firstMeas].data_len) != 0xFF)
				{  // there was an answer from the database server, so the data was transmitted
					--LVM.measbuff->numberStored;
					if (LVM.measbuff->firstMeas < (MEASBUFFER_LENGTH-1)) LVM.measbuff->firstMeas = LVM.measbuff->firstMeas + 1;
					else LVM.measbuff->firstMeas = 0;
					InitScreen_AddLine(STR_SUCCESSFUL,0);
				}
				else InitScreen_AddLine(STR_UN_SUCCESSFUL,0);

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

				sprintf(LVM.temp->string,STR_REMAINING,LVM.measbuff->numberStored);
				InitScreen_AddLine(LVM.temp->string,0);
			}

			_delay_ms(2000);
		}
		
		DS3231M_read_time();


		paint_main(Time, Model->mode->netstat, PAINT_ALL);

		// Reset the awake period
		set_timeout(0, TIMER_5, RESET_TIMER);
		set_timeout(xbee_get_awake_period(), TIMER_5, USE_TIMER);		// Stay active for xbee_awake_period
		
		

	}


}

// Go to options pages
void main_pressedBOT(Controller_Model *Model){
	if (LVM.message->Received)
	{
		return;
	}

	uint8_t status;	// Pressed "cancel" or "ok"
	LVM.vars->entered_options_pw = get_Password(&status);
	not_ready_for_new_key();

	if(status || (LVM.vars->entered_options_pw != LVM.options->options_pw))
	{
		// Cancelled or wrong password, go back to the main mode
		paint_main(Time, Model->mode->netstat, PAINT_ALL);
		Model->mode->next = ex_main;
		return;
	}
	// Set options mode
	Model->mode->next = ex_options;
	
	if (Model->mode->netstat == offline)
	{
		make_he_vol_changable();


	}

	

	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);

	// Init. options flow control
	set_OptionModel(1,1,0);
	opt_drawPage();
	set_bufferVars(false);
	
	keyhit_block();



}

//do nothing
void main_pressedNONE(Controller_Model *Model){
	
}



//=========================================================================
// Measure levels and send to the database every "INTERVAL SLOW"
//=========================================================================
void main_pre_Switch_case_Tasks(Controller_Model *Model){

	if(Model->mode->netstat == online)
	{
		if(!set_timeout(0, TIMER_1, USE_TIMER))
		{

			// turn display on
			display_on();

			measure(Model);

			xbee_wake_up_plus();
			

			collect_and_send_MeasData(LVM.temp->buffer,LONG_INTERVAL_MSG);

			// Free XBee module
			

			// Set timer for the next delay
			(LVM.options->transmit_slow_min) ? set_timeout(ceil(LVM.options->transmit_slow*60), TIMER_1, USE_TIMER) :	set_timeout(LVM.options->transmit_slow*3600, TIMER_1, USE_TIMER);
			
			if (LVM.message->Received)
			{
				paint_Notification(LVM.message);
			}
			else
			{
				paint_main(Time, 1, PAINT_ALL);
			}
		}
	}

	
}

void main_post_Switch_case_Tasks(Controller_Model *Model){
	
	manage_Xbee_sleep_cycles(Model);
	
	interval_slow_changed(Model);
	
	autofill_check(Model);
	
	handle_received_Messages(Model);
	
}



void autofill_check(Controller_Model * Model){
	//=========================================================================
	// Auto filling
	//=========================================================================


	if(auto_fill_pin_on() && LVM.vars->auto_fill_enabled)
	{//auto fill
		//Vessel empty, back to main, no filling
		if(LVM.vars->very_low_He_level)
		{
			timed_dialog(STR_HE_LEVEL,STR_FILLING_IS_DISABLE_LEVEL_TOO_LOW ,10, D_FGC, D_BGC);
			LVM.vars->auto_fill_enabled = false;
			Model->mode->next= ex_main;
			paint_main(Time,Model->mode->netstat,PAINT_ALL);
		}
		else 
		{
			_delay_ms(100);
			//no external filling signal any more?
			if(!auto_fill_pin_on())
			{
				Model->mode->next= ex_main;
				paint_main(Time,Model->mode->netstat,PAINT_ALL);
			}
			else
			{

				InitScreen_AddLine(STR_AUTOFILL_STARTED ,1);
				sprintf(LVM.temp->string,STR_TIMEOUT_MIN ,LVM.options->fill_timeout);
				InitScreen_AddLine(LVM.temp->string, 0);
				if (LVM.options->transmit_fast_sec) sprintf(LVM.temp->string,STR_INTERVAL_S ,LVM.options->transmit_fast);
				else sprintf(LVM.temp->string,STR_INTERVAL_MIN ,LVM.options->transmit_fast);
				InitScreen_AddLine(LVM.temp->string, 0);
				InitScreen_AddLine(STR_SENING_DATA, 0);

				xbee_wake_up_plus();
				

				uint8_t indx = 0;  



				indx = devicePos_to_buffer(LVM.vars->device_pos, indx, LVM.temp->buffer);
				LVM.temp->buffer[indx++] =  get_status_byte_levelmeter();

				//send
				#ifdef ALLOW_COM
				if (xbee_send_request(FILLING_BEGIN_MSG, LVM.temp->buffer, indx) == 0xFF)
				InitScreen_AddLine(STR_UN_SUCCESSFUL, 0);
				else InitScreen_AddLine(STR_SUCCESSFUL, 0);
				// comment: message is not stored if communication to database server is not working
				#else
				// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
				if (xbee_send_message(FILLING_BEGIN_MSG, LVM.temp->buffer, indx))
				{
					CLEAR_ALL(); // Clear all errors
				}
				#endif

				


				if (LVM.options->transmit_fast_sec) LVM.vars->fill_meas_counter = (LVM.options->fill_timeout * 60) / LVM.options->transmit_fast;
				else LVM.vars->fill_meas_counter = LVM.options->fill_timeout/LVM.options->transmit_fast;

				paint_filling(Time, Model->mode->netstat, 0, 1);
				


				set_timeout(0, TIMER_4, RESET_TIMER);
				
				Model->mode->next = ex_filling;
				//set output fill pin
				START_AUTO_FILL;
				LVM.vars->auto_fill_started = true;
			}
		}
	}

}
