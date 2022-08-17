/*
 * error_mode.c
 *
 * Created: 01.03.2021 13:56:20
 *  Author: Weges
 */ 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base_controller.h"
#include "error_mode.h"

#include "../display_utilities.h"
#include "../HoneywellSSC.h"
#include "../main.h"

struct Controller_Model_Vtable error_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	
	&base_display_on_pressed,
	&base_display_off_pressed,
	&error_pre_Switch_case_Tasks,
	&base_pressedDONOTHING
};



Controller_Model_error error_model ={
	.super.mode = &global_mode,
	.super.vtable = &error_Vtable
};

Controller_Model_error* get_error_model(){
	return &error_model;
}


void error_pre_Switch_case_Tasks(Controller_Model *Model){
	LCD_Cls(BGC);
	switch(Model->mode->error)
	{
		case login_failed:
		LCD_Print(STR_CONNECTION, X_M_5, Y_M_40, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_TO_SERVER_FAILED, X_M_5, Y_M_60, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_RESTART_DEVICE_TO_TRY_AGAIN, X_M_2, Y_M_90, 1, 1, 1, FGC, BGC);
		_delay_ms(10000);
		SHUTDOWN;
		case shutdown_failed:
		LCD_Print(STR_FAILED,X_M_5, Y_M_40, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_TO_SHUTDOWN, X_M_5, Y_M_60, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_PROBABLY_HARDWARE_ISSUE, X_M_2, Y_M_90, 1, 1, 1, FGC, BGC);
		_delay_ms(10000);
		SHUTDOWN;
		default:
		LCD_Print(STR_UNKNOWN_ERROR, X_M_5, Y_M_40, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_PROBABLY_HARDWARE, X_M_5, Y_M_60, 2, 1, 1, FGC, BGC);
		char bufferStr[20];
		sprintf(bufferStr, "code = %u", Model->mode->error);
		LCD_Print(bufferStr, X_M_2, Y_M_90, 1, 1, 1, FGC, BGC);
		break;
	}
}


