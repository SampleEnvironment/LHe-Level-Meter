/*
* getCode_mode.c
*
* Created: 05.03.2021 14:42:07
*  Author: Weges
*/

#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>

#include "base_controller.h"
#include "getCode_mode.h"
#include "../timer_utilities.h"
#include "../display.h"
#include "../display_utilities.h"


struct Controller_Model_Vtable GetDevID_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&getCode_pressedHIDDEN,
	&getCode_pressedTOP,
	&getCode_pressedUP,
	&getCode_pressedLEFT,
	&getCode_pressedRIGHT,
	&getCode_pressedDOWN,
	&getCode_pressedBOT,
	&base_pressedDONOTHING,
	&base_display_always_on,
	&base_display_DONOTHING,
	&getDEV_ID_pre_Switch_case_Tasks,
	&base_pressedDONOTHING
};

struct Controller_Model_Vtable GetPw_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&getCode_pressedTOP,
	&getCode_pressedUP,
	&getCode_pressedLEFT,
	&getCode_pressedRIGHT,
	&getCode_pressedDOWN,
	&getCode_pressedBOT_PW,
	&base_pressedDONOTHING,
	&base_display_always_on,
	&base_display_DONOTHING,
	&get_Pw_pre_Switch_case_Tasks,
	&base_pressedDONOTHING
};




Controller_Model_getCode getCode_model ={
	.super.mode = &global_mode,
	.super.vtable = &GetDevID_Vtable,
	.Code_Str="",
	.digits = {0,0,0,0,0,0},
	.xCoords = {0,0,0,0,0,0},
	.active_digit = 0,
	.active_last = 0,
	.Space_Factor = SPACE_FAC_PE,
	.FontNR = FONTNR_PE_2,
	.FontW = FONT_W_PE,
	.FontH = FONT_H_PE,
	.Code_len = 0,
	.alphanum = 0,
	.digit_xOffset = 0,
	.status = NULL,
	.exit_loop = 0,
	.lowest_char = 0
	};





void getCode_pressedHIDDEN(Controller_Model *Model){
	*getCode_model.status =1;
	getCode_model.exit_loop = 1;
	
}
void getCode_pressedTOP(Controller_Model *Model){
	*getCode_model.status =2;
	getCode_model.exit_loop = 1;

	
}
void getCode_pressedUP(Controller_Model *Model){
	if((getCode_model.digits[getCode_model.active_digit] == 36 && getCode_model.alphanum) ||
	(getCode_model.digits[getCode_model.active_digit] == 10 && !getCode_model.alphanum)){
		getCode_model.digits[getCode_model.active_digit] = getCode_model.lowest_char;
	}

	else
	{
		getCode_model.digits[getCode_model.active_digit] += 1;
	}
	
	
	update_Dev_ID_digits();
	
}
void getCode_pressedLEFT(Controller_Model *Model){
	if (getCode_model.active_digit == 0)
	{
		getCode_model.active_digit = getCode_model.Code_len-1;
	}
	else
	{
		getCode_model.active_digit -= 1;
	}
	
	update_Dev_ID_digits();
	
	getCode_model.active_last = getCode_model.active_digit;
	
}
void getCode_pressedRIGHT(Controller_Model *Model){
	
	if (getCode_model.active_digit == getCode_model.Code_len-1)
	{
		getCode_model.active_digit = 0;
	}
	else
	{
		getCode_model.active_digit += 1;
	}
	update_Dev_ID_digits();
	
	getCode_model.active_last = getCode_model.active_digit;
	
}
void getCode_pressedDOWN(Controller_Model *Model){
	if(getCode_model.digits[getCode_model.active_digit] == getCode_model.lowest_char)
	{
		getCode_model.digits[getCode_model.active_digit] = getCode_model.alphanum?36:10;
		
	}
	else
	{
		getCode_model.digits[getCode_model.active_digit] -= 1;
	}
	update_Dev_ID_digits();
	
	
}
void getCode_pressedBOT(Controller_Model *Model){
	if (validate_Dev_ID())
	{
		// DEV_ID not accepted try again
		
		
		return;
	}
	

	*getCode_model.status =0;
	getCode_model.exit_loop = 1;
	_delay_ms(100);
	
}

void getCode_pressedBOT_PW(Controller_Model *Model){
	*getCode_model.status =0;
	getCode_model.exit_loop = 1;
	_delay_ms(100);
	
}



void getDEV_ID_pre_Switch_case_Tasks(Controller_Model *Model){
	if (Model->mode->Key != 0 )
	{
		//reset timer! a key was pressed
		set_timeout(0, TIMER_3, RESET_TIMER);
		set_timeout(SHUTDOWN_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	}
	
	
	if(!set_timeout(0,TIMER_3, USE_TIMER) )  //shutdown after shutdown_timout if no key was pressed
	{
		LCD_Cls(BGC);
		LCD_Print(STR_NO_DEVICE_ENTERED, X_M_5, Y_M_40, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_SHUTDOWN_NOW, X_M_5, Y_M_60, 2, 1, 1, FGC, BGC);
		LCD_Print(STR_RESTART_DEVICE_TO_TRY_AGAIN, X_M_2, Y_M_90, 1, 1, 1, FGC, BGC);
		_delay_ms(10000);
		SHUTDOWN;
		_delay_ms(4000);
	}
	
	
}

void get_Pw_pre_Switch_case_Tasks(Controller_Model *Model){
	if (Model->mode->Key != 0 )
	{
		//reset timer! a key was pressed
		set_timeout(0, TIMER_3, RESET_TIMER);
		set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	}
	
	
	if(!set_timeout(0,TIMER_3, USE_TIMER) )
	{
		*getCode_model.status = 2; //cancel
		getCode_model.exit_loop = true;
		
	}
	
	
}



void get_Device_ID(uint8_t *status)
{
	Controller_Model * const  GET_CODE_MODEL= (Controller_Model*) &getCode_model;
	
	getCode_model.super.vtable = &GetDevID_Vtable;
	getCode_model.Code_len = LVM.options->Dev_ID_Max_len;
	getCode_model.alphanum = LVM.options->Dev_ID_alpahnum;
	getCode_model.active_digit = getCode_model.Code_len-1;
	getCode_model.active_last = getCode_model.Code_len-1;
	getCode_model.Space_Factor = SPACE_FAC_PE;
	getCode_model.FontNR = FONTNR_PE_2;
	getCode_model.FontW = FONT_W_PE;
	getCode_model.FontH = FONT_H_PE;
	getCode_model.digit_xOffset = CENTER_LINE - ((getCode_model.Space_Factor * getCode_model.Code_len)/2);
	getCode_model.exit_loop = 0;
	getCode_model.status = status;
	getCode_model.lowest_char = 0;
	
	for(uint16_t i = 0; i < getCode_model.Code_len; i++ ){
		getCode_model.xCoords[i] = (i*getCode_model.Space_Factor)+ getCode_model.digit_xOffset;
		getCode_model.digits[i]=0;
	}

	paint_Dev_ID();
	paint_getCode_digits();
	

	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(SHUTDOWN_TIMEOUT_TIME, TIMER_3, USE_TIMER);

	while(!getCode_model.exit_loop)
	{
		Controller(GET_CODE_MODEL);
	}


}

uint16_t get_Password(uint8_t *status){
	Controller_Model * const  GET_CODE_MODEL= (Controller_Model*) &getCode_model;
	
	getCode_model.super.vtable = &GetPw_Vtable;
	getCode_model.Code_len = 3;
	getCode_model.alphanum = false;
	getCode_model.active_digit = getCode_model.Code_len-1;
	getCode_model.active_last = getCode_model.Code_len-1;
	getCode_model.Space_Factor = SPACE_FAC_PE;
	getCode_model.FontNR = FONTNR_PE_2;
	getCode_model.FontW = FONT_W_PE;
	getCode_model.FontH = FONT_H_PE;
	getCode_model.digit_xOffset = CENTER_LINE - ((getCode_model.Space_Factor * getCode_model.Code_len)/2);
	getCode_model.exit_loop = 0;
	getCode_model.status = status;
	getCode_model.lowest_char = 1;
	
	for(uint16_t i = 0; i < getCode_model.Code_len; i++ ){
		getCode_model.xCoords[i] = (i*getCode_model.Space_Factor)+ getCode_model.digit_xOffset;
		getCode_model.digits[i]=1;
	}

	paint_get_password();
	paint_getCode_digits();
	
	
	//reset timer! a key was pressed
	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(SHUTDOWN_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	
	while(!getCode_model.exit_loop)
	{
		Controller(GET_CODE_MODEL);
	}
	

	
	uint16_t digit1 = (uint16_t) getCode_model.digits[0];
	uint16_t digit2 = (uint16_t) getCode_model.digits[1];
	uint16_t digit3 = (uint16_t) getCode_model.digits[2];

	

	
	return (((digit1-1)*100) + ((digit2-1)*10) + (digit3-1));
	
}




void paint_getCode_digits(void)
{

	char letter[5];
	
	for (uint8_t i = 0; i < getCode_model.Code_len-1; i++ ){
		LCD_Print(itoc_dev(letter,getCode_model.digits[i]),getCode_model.xCoords[i] +xoff,Y_PE_70,getCode_model.FontNR,1,1,FGC,BGC);
		
		LCD_hline(getCode_model.xCoords[i]+xoff,Y_PE_70+ getCode_model.FontH+1,getCode_model.FontW,FGC);
	}

	LCD_Print(itoc_dev(letter,getCode_model.digits[getCode_model.Code_len-1]),getCode_model.xCoords[getCode_model.Code_len-1] +xoff,Y_PE_70,getCode_model.FontNR,1,1,ERR,FGC);
	LCD_hline(getCode_model.xCoords[getCode_model.Code_len-1]+xoff,Y_PE_70+ getCode_model.FontH+1,getCode_model.FontW,FGC);

}

void update_Dev_ID_digits(void){
	char letter[5];
	LCD_Print(itoc_dev(letter,getCode_model.digits[getCode_model.active_digit]),xoff+getCode_model.xCoords[getCode_model.active_digit],Y_PE_70,FONTNR_PE_2,1,1,ERR,FGC);
	
	if (getCode_model.active_digit != getCode_model.active_last)
	{
		LCD_Print(itoc_dev(letter,getCode_model.digits[getCode_model.active_last]),xoff+getCode_model.xCoords[getCode_model.active_last],Y_PE_70,FONTNR_PE_2,1,1,FGC,BGC);
	}
	
}

uint8_t validate_Dev_ID(void){
	char temp_Dev[getCode_model.Code_len+1];
	
	LVM.vars->Device_ID_Str[DEVICE_ID_STRING_LEN] = '\0';
	
	for (uint8_t i=0;i<getCode_model.Code_len;i++)
	{
		itoc_dev(&temp_Dev[i],getCode_model.digits[i]);
	}
	temp_Dev[getCode_model.Code_len] = '\0';
	
	
	uint8_t j = 0;
	for (uint8_t i = 0; i<getCode_model.Code_len;i++)
	{
		if (temp_Dev[i] == ' ' && j != 0)
		{
			LVM.vars->Device_ID_Str[j++] = '\0';
			break;
		}
		
		if (temp_Dev[i] != ' ')
		{
			LVM.vars->Device_ID_Str[j] = temp_Dev[i];
			j++;
		}
	}
	

	for (uint8_t i = j; i < DEVICE_ID_STRING_LEN; i++)
	{
		LVM.vars->Device_ID_Str[i] = '\0';
	}
	

	if (j == 0)
	{
		
		return 1; // dev id only spaces not accepted
	}
	else
	{
		
		return 0; // dev id accepted full length
		
	}
	

}

char* itoc_dev(char * letterstr,uint8_t letter_num){
	uint8_t ascii_letter = 0;
	
	if (letter_num == 0)
	{
		ascii_letter= ' ';
	}
	
	if (letter_num > 0 && letter_num < 11)
	{
		ascii_letter = letter_num + 47;
	}
	
	if(letter_num > 10)
	{
		ascii_letter = letter_num + '6';
	}
	
	sprintf(letterstr,"%c",ascii_letter);
	return letterstr;
}





