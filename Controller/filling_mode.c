/*
* filling_mode.c
*
* Created: 01.03.2021 09:18:44
*  Author: Weges
*/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "base_controller.h"
#include "filling_mode.h"
#include "../display_driver.h"
#include "../display_utilities.h"
#include "../HoneywellSSC.h"
#include "../main.h"
#include "../timer_utilities.h"

#include "xbee.h"
#include "DS3231M.h"

#ifdef DISP_3000
#include "../StringPixelCoordTable.h"
#endif

#ifdef ili9341
#include "../StringPixelCoordTable_ili9341.h"
#endif


struct Controller_Model_Vtable filling_Vtable ={
	&base_pressedDONOTHING,
	&filling_pressedMeasure,
	&base_pressedDONOTHING,
	&filling_pressedTOP,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&filling_pressedBOT,
	&filling_pressedNONE,
	&base_display_on_pressed,
	&base_display_off_pressed,
	&base_pressedDONOTHING,
	&filling_post_Switch_case_Tasks
};

Controller_Model_filling filling_model ={
	.super.mode = &global_mode,
	.super.vtable = &filling_Vtable,
	.meas_progress = 0
};

Controller_Model_filling * get_fill_model(){
	return &filling_model;
}

// make measurement
void filling_pressedMeasure(Controller_Model *Model){
	if (LVM.message->Received)
	{

		LVM.message->Received = false;
		for(uint8_t i = 5; i > 0 ;i--)
		{
			sprintf(LVM.temp->string,"%i",i);
			LCD_Print(LVM.temp->string, X_M_150, Y_M_5,2,2,2,FGC,BGC);
			_delay_ms(1000);
		}
		
		paint_filling(Time, Model->mode->netstat, 0, 1);
		return;
	}
	
	

	
	// Wake up XBee module if not in offline mode
	if(Model->mode->netstat == online)
	{
		xbee_wake_up_plus();
		
	}

	measure(Model);
	
	

	paint_filling(Time, Model->mode->netstat, UPDATE_ONLY, 1);
	
	// Transmit data to server if not in offline mode
	if (Model->mode->netstat == online)
	{
		// Pack data in frame
		collect_and_send_MeasData(LVM.temp->buffer,STATUS_MSG);

		// Free XBee module
		

		// clear info line
		paint_info_line("",0);
	}
	
	return;
	
}


// Force to stop filling (omit command if autofilling)
void filling_pressedTOP(Controller_Model *Model){
	if (LVM.message->Received || LVM.vars->auto_fill_started)
	{
		return;
	}

	if(LCD_Dialog(STR_FILLING,STR_DO_YOU_REALLY_WANT_TO_STOP_FILLING, D_FGC, D_BGC,STOP_FILL_TIMEOUT_TIME))
	{
		
		InitScreen_AddLine("Last Measurement",1);

		// Stop filling procedure!
		set_timeout(0, TIMER_4, RESET_TIMER); 		// Clear timer


		// Do a last measurement
		measure(Model);

		
		if (Model->mode->netstat == online)
		{
			// Wake up XBee module
			xbee_wake_up_plus();
			
			


			InitScreen_AddLine(STR_SENDING_INFO,0);
			InitScreen_AddLine(STR_END_OF_FILLING,0);
			InitScreen_AddLine(STR_TO_THE_SERVER,0);
			
			

			if (collect_and_send_MeasData(LVM.temp->buffer,FILLING_END_MSG))
			{
				InitScreen_AddLine(STR_UN_SUCCESSFUL,0);
			}
			else
			{
				InitScreen_AddLine(STR_SUCCESSFUL,0);
			}
			
			
			// to read the messages
			_delay_ms(2000);

		}
		// Regardless communication errors continue anyway
		paint_main(Time, Model->mode->netstat, PAINT_ALL);
		
		// Autofill will be restatrted automatically
		// --> disable autofill in options
		
		STOP_AUTO_FILL;
		LVM.vars->auto_fill_started = false;
		//LVM.vars->auto_fill_enabled = false;
		Model->mode->next = ex_main;
		base_display_extend_onTime();


	}
	else
	{

		// Cancelled stopping request
		paint_filling(Time, Model->mode->netstat, 0, 1);
		
	}
	return;
	
}


// More filling needed
void filling_pressedBOT(Controller_Model *Model){
	if (LVM.message->Received)
	{
		return;
	}

	LVM.vars->fill_meas_counter++;		// More filling needed
	if(LVM.options->transmit_fast_sec) {
		if(LVM.vars->fill_meas_counter > ((((uint16_t) LVM.options->fill_timeout*60))/LVM.options->transmit_fast))	 	LVM.vars->fill_meas_counter = (((uint16_t) LVM.options->fill_timeout*60))/LVM.options->transmit_fast;
		} else {
		if(LVM.vars->fill_meas_counter > (LVM.options->fill_timeout/LVM.options->transmit_fast)) 						LVM.vars->fill_meas_counter = LVM.options->fill_timeout/LVM.options->transmit_fast;
	}
	

	paint_filling_status();
	return;
	
	
}



//=================================================================================================================
// Filling routine
//=================================================================================================================

void filling_pressedNONE(Controller_Model *Model){
	_Bool filling_end = false;

	filling_model.meas_progress = set_timeout(0, TIMER_4, USE_TIMER);
	
	if(!filling_model.meas_progress){	// Time between measurements is passed, new measurement

		display_on();

		paint_info_line(STR_MEASURING, 0);

		(LVM.options->transmit_fast_sec)?
		set_timeout(LVM.options->transmit_fast, TIMER_4, USE_TIMER)
		:	set_timeout(LVM.options->transmit_fast*60, TIMER_4, USE_TIMER);

		measure(Model);

		clear_progress_bar(xoff+X_M_5, Y_M_105);


		if (!LVM.message->Received)
		{
			paint_filling(Time, Model->mode->netstat, 1, 1);
		}

		
		if (Model->mode->netstat == online)
		{
			// Wake up XBee module
			xbee_wake_up_plus();
			

			collect_and_send_MeasData(LVM.temp->buffer,FILLING_MSG);

			
		}



		LVM.vars->fill_meas_counter--;
		_delay_ms(150);

		//if autofilling and no more signal - end
		if(!auto_fill_pin_on() && LVM.vars->auto_fill_started)
		{
			
			//filling_end = true;
			
			
			//set auto fill pin low
			STOP_AUTO_FILL
			LVM.vars->auto_fill_started = false;
			LVM.vars->fill_meas_counter = 5;
			
			paint_filling_status();
		}
		//if normal filling and counter=0 - end
		if(!LVM.vars->fill_meas_counter && !LVM.vars->auto_fill_started)
		{
			//FE
			filling_end = true;
		}
		//he_level twice lower then min, end/disable filling
		if((LVM.vars->he_level < LVM.options->he_min) && LVM.vars->auto_fill_started)
		{
			if(LVM.vars->last_he_level < LVM.options->he_min)
			{
				STOP_AUTO_FILL
				timed_dialog(STR_HE_LEVEL, STR_AUTFILL_IS_DISABLED_HE_LEVEL_TOO_LOW, 10, D_FGC, D_BGC);
				LVM.vars->very_low_He_level = true;

				//FE
				LVM.vars->fill_meas_counter = 5;
				LVM.vars->auto_fill_started = false;
				LVM.vars->auto_fill_enabled = false;		//disable autofill regardless any signals
				
				paint_filling(Time, Model->mode->netstat, 0, 0);
				paint_filling_status();
			}
		}
		LVM.vars->last_he_level = LVM.vars->he_level;
		if(!filling_end)
		{
			//set timer till next measurement
			paint_info_line(STR_WAITING, 0);
			clear_progress_bar(xoff+X_M_5, Y_M_105);

			paint_filling_status();

		}
	}//if(timer ready)
	else if(!auto_fill_pin_on() && LVM.vars->auto_fill_started)
	{
		//LCD_Print("END_timer",20,50,2,1,1,red,BGC);
		//FE
		//filling_end = true;
		//set auto fill pin low
		STOP_AUTO_FILL
		LVM.vars->auto_fill_started = false;
		LVM.vars->fill_meas_counter = 5;
		
		paint_filling_status();
	}
	else // do this when nothing special has to be done
	{
		
		(LVM.options->transmit_fast_sec)? 	draw_current_wait_time(X_M_60+xoff, Y_M_90, LVM.options->transmit_fast, filling_model.meas_progress, ERR)
		:						draw_current_wait_time(X_M_60+xoff, Y_M_90, LVM.options->transmit_fast*60, filling_model.meas_progress, ERR);

		// enter pressure
		if (HoneywellSSC_status.connected)
		{
			LVM.vars->pressure_level = 0;
			HoneywellSSC_read_pressure();
			if (HoneywellSSC_status.status < 4) LVM.vars->pressure_level = HoneywellSSC_Pressure;
			paint_time_pressure(Time, LVM.vars->pressure_level, 1);  // update mode
		}

	}

	if(filling_end)
	{

		display_on();
		
		set_timeout(0, TIMER_4, RESET_TIMER);
		STOP_AUTO_FILL;
		InitScreen_AddLine(STR_FILLING_TERMINATED,1);
		if (Model->mode->netstat == online)
		{
			
			InitScreen_AddLine(STR_SENDING_INFO,0);
			InitScreen_AddLine(STR_TO_THE_SERVER,0);

			// Wake up XBee module
			xbee_wake_up_plus();
			

			//send end ..
			if (collect_and_send_MeasData(LVM.temp->buffer,FILLING_END_MSG))
			{
				InitScreen_AddLine(STR_UN_SUCCESSFUL,0);
			}
			else
			{
				InitScreen_AddLine(STR_SUCCESSFUL,0);
			}


			
			
		}
		_delay_ms(2000);  // for the messages to be read
		paint_main(Time, Model->mode->netstat, PAINT_ALL);
		Model->mode->next = ex_main;

		
	}

}


void filling_post_Switch_case_Tasks(Controller_Model *Model){
	
	manage_Xbee_sleep_cycles(Model);
	
	interval_slow_changed(Model);
	
	handle_received_Messages(Model);
}


void paint_filling_status(void){
	if (LVM.vars->auto_fill_started)
	{
		LCD_Print("Autofill", xoff+X_M_2, Y_M_105, 1, 1, 1, green, BGC);
	}
	else
	{
		int16_t helpvariable = round((double) LVM.vars->fill_meas_counter * LVM.options->transmit_fast / 60);
		if (LVM.options->transmit_fast_sec) sprintf(LVM.temp->string, FILL_WAITING_LABEL, helpvariable);
		else sprintf(LVM.temp->string, FILL_WAITING_LABEL, LVM.vars->fill_meas_counter*LVM.options->transmit_fast);
		LCD_Print("               ", xoff+X_M_1, Y_M_100, 2, 1, 1, FGC, BGC);
		LCD_Print(LVM.temp->string, xoff+X_M_2, Y_M_105, 1, 1, 1, FGC, BGC);
	}
	
}