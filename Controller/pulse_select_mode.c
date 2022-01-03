/*
* pulse_select_mode.c
*
* Created: 22.03.2021 13:14:05
*  Author: Weges
*/

#include <stdbool.h>
#include <stdio.h>

#include "pulse_select_mode.h"
#include "option_mode.h"
#include "../keyboard.h"
#include "../display_utilities.h"
#include "../timer_utilities.h"
#include "../diag_pulse.h"




uint16_t xAxis_len = X_AXIS_LEN;
uint16_t yAxis_len = Y_AXIS_LEN;


uint8_t Pulstetype_index = 0;

ValueOptionsNull Null_pulse;
ValueOptionsChar pulse_type;


// Page1 (normal Pulse)
//pulse_select (char)
ValueOptionsDouble quench_pulse;
ValueOptionsDouble quench_time_pulse;
ValueOptionsDouble meas_curr_pulse;
ValueOptionsDouble wait_time_pulse;
// Null5


// Page2 (long Pulse)
//pulse_select (char)
ValueOptionsDouble duration_pulse;
ValueOptionsDouble const_current;
// Null4
// Null5

// Page3 (linear Pulse)
//pulse_select (char)
ValueOptionsDouble delta_I;
ValueOptionsDouble I_start;
ValueOptionsDouble I_end;
ValueOptionsDouble delta_t;


ValueOptions *Pulse_select_Array[15];

char *Pulse_type_str_arr[3];

//char optPage[3][20]= {"Pulse Select","Long Pulse","Linear Pulse"};

optPageStrType pulse_select[]={
	{.Page[0]= STR_PULSE_TYPE ,.Page[1]=STR_QUENCH_CURRENT,.Page[2]=STR_HEAT_TIME,.Page[3]=STR_MEAS_CURRENT,.Page[4]=STR_WAIT_TIME},
	{.Page[0]= STR_PULSE_TYPE ,.Page[1]=STR_PULSE_DURATION,.Page[2]=STR_PULSE_CURRENT,.Page[3]="",.Page[4]=""},
	{.Page[0]= STR_PULSE_TYPE ,.Page[1]=STR_PULSE_DELTA_t,.Page[2]=STR_PULSE_DELTA_I,.Page[3]=STR_PULSE_I_MIN,.Page[4]=STR_PULSE_I_MAX}
};



void Char_valueChange_pulse_select (ValueOptions *optEntry, int key){

	ValueOptionsChar *optEntry_ = (ValueOptionsChar*) optEntry;


	if (key == KEY_DOWN_S9 ){
		if (*optEntry_->arr_ind > optEntry_->MIN)
		{
			
			*optEntry_->arr_ind -= 1;
		}
		if (*optEntry_->arr_ind == optEntry_->MIN)
		{
			*optEntry_->arr_ind = 2 ;
		}
	}
	else
	{
		*optEntry_->arr_ind += 1;
	}


	if(*optEntry_->arr_ind > optEntry_->MAX) {
		*optEntry_->arr_ind = 0;
	}
	
}


void Char_valueDraw_pulse_select (ValueOptions *optEntry, unsigned int color)
{
	ValueOptionsChar *optEntry_ = (ValueOptionsChar *) optEntry;
	LCD_Print("      " ,X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,BGC,BGC);
	LCD_Print(optEntry_->string_arr[*optEntry_->arr_ind-1],X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,color,BGC);
}


//Char Vtable Pulse_select
struct ValueOptVtable ValueOptCharVtable_pulse_select =
{
	&Char_valueDraw_pulse_select,
	&Char_valueChange_pulse_select
	
};

struct Controller_Model_Vtable pulse_select_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&pulse_select_pressedTOP,
	&pulse_select_pressedUP,
	&pulse_select_pressedLEFT,
	&pulse_select_pressedRIGHT,
	&pulse_select_pressedDOWN,
	&pulse_select_pressedBOT,
	&base_pressedDONOTHING,
	&base_display_always_on,
	&base_display_DONOTHING,
	&pulse_select_pre_Switch_case_Tasks,
	&pulse_select_post_Switch_case_Tasks
};


Controller_Model_pulse_select pselect_model ={
	.super.mode = &global_mode,
	.super.vtable = &pulse_select_Vtable,
	.option = 1,
	.optionLast = 1,
	.page =1,
	.pageLast = 1,
	.value = 0,
	.valueLast =0,
	.valueChange =0,
	.options_changed = false,
	.pulse_duration = 10, //pulse duration in seconds
	.const_current = 10,  //constant current in pulse

	.delta_I = 5, // delta I in mA
	.delta_t = 1.0, // delta t in ms
	.I_start = 50, // I_min in mA
	.I_end = QUENCH_CURRENT_MAX   //I_max in mA
	
	
};

Controller_Model_pulse_select* get_pulse_select_model(void){
	return &pselect_model;
}





void pulse_select_Init(void){
	Pulse_type_str_arr[0] = "normal";
	Pulse_type_str_arr[1] = "const.";
	Pulse_type_str_arr[2] = "linear";
	
	
	ValOptNull_init(&Null_pulse,YcoordOfOptline[4],"","");
	ValOptChar_init(&pulse_type, &ValueOptCharVtable_pulse_select,YcoordOfOptline[0],pulse_select[0].Page[0],Pulse_type_str_arr,&pselect_model.page,0,2);
	
	//Page1
	ValOptDouble_init(&quench_pulse,YcoordOfOptline[1],pulse_select[0].Page[1],&pselect_model.quench_curr,1,1,QUENCH_CURRENT_MAX,QUENCH_CURRENT_MIN,5,"mA",true);
	ValOptDouble_init(&quench_time_pulse,YcoordOfOptline[2],pulse_select[0].Page[2],&pselect_model.quench_time,0.1,0.1,QUENCH_TIME_MAX,QUENCH_TIME_MIN,1,"s",true);
	ValOptDouble_init(&meas_curr_pulse,YcoordOfOptline[3],pulse_select[0].Page[3],&pselect_model.meas_curr,1,1,MEAS_CURRENT_MAX,MEAS_CURRENT_MIN,5,"mA",true);
	ValOptDouble_init(&wait_time_pulse,YcoordOfOptline[4],pulse_select[0].Page[4],&pselect_model.wait_time,0.1,0.1,WAIT_TIME_MAX,WAIT_TIME_MIN,1,"s",true);
	
	
	//Page2
	ValOptDouble_init(&duration_pulse,YcoordOfOptline[1],pulse_select[1].Page[1],&pselect_model.pulse_duration,1,1,200,5,0,"s",false);
	ValOptDouble_init(&const_current,YcoordOfOptline[2],pulse_select[1].Page[2],&pselect_model.const_current,1,2,QUENCH_CURRENT_MAX,1,5,"mA",false);
	
	//Page3
	ValOptDouble_init(&delta_t,YcoordOfOptline[1],pulse_select[2].Page[1],&pselect_model.delta_t,0.1,0.1,5.0,0.1,1,"s",false);
	ValOptDouble_init(&delta_I,YcoordOfOptline[2],pulse_select[2].Page[2],&pselect_model.delta_I,1,1,20,1,5,"mA",false);
	ValOptDouble_init(&I_start,YcoordOfOptline[3],pulse_select[2].Page[3],&pselect_model.I_start,1,2,QUENCH_CURRENT_MAX,1,5,"mA",false);
	ValOptDouble_init(&I_end,YcoordOfOptline[4],pulse_select[2].Page[4],&pselect_model.I_end,1,2,QUENCH_CURRENT_MAX,1,5,"mA",false);



	Pulse_select_Array[0]= (ValueOptions* )&pulse_type;
	Pulse_select_Array[1]= (ValueOptions* )&quench_pulse;
	Pulse_select_Array[2]= (ValueOptions* )&quench_time_pulse;
	Pulse_select_Array[3]= (ValueOptions* )&meas_curr_pulse;
	Pulse_select_Array[4]= (ValueOptions* )&wait_time_pulse;
	
	Pulse_select_Array[5]= (ValueOptions* )&pulse_type;
	Pulse_select_Array[6]= (ValueOptions* )&duration_pulse;
	Pulse_select_Array[7]= (ValueOptions* )&const_current;
	Pulse_select_Array[8]= (ValueOptions* )&Null_pulse;
	Pulse_select_Array[9]= (ValueOptions* )&Null_pulse;
	
	Pulse_select_Array[10]= (ValueOptions* )&pulse_type;
	Pulse_select_Array[11]= (ValueOptions* )&delta_t;
	Pulse_select_Array[12]= (ValueOptions* )&delta_I;
	Pulse_select_Array[13]= (ValueOptions* )&I_start;
	Pulse_select_Array[14]= (ValueOptions* )&I_end;
	
	pselect_model.pulse_duration = 10; //pulse duration in seconds
	pselect_model.const_current = 100;  //constant current in pulse

	pselect_model.delta_I = 5; // delta I in mA
	pselect_model.delta_t = 0.1; // delta t in s
	pselect_model.I_start = 50; // I_min in mA
	pselect_model.I_end = QUENCH_CURRENT_MAX;   //I_max in mA
	
	pselect_model.quench_curr = LVM.options->quench_current;
	pselect_model.quench_time = LVM.options->quench_time;
	pselect_model.meas_curr   = LVM.options->meas_current;
	pselect_model.wait_time   = LVM.options->wait_time;
	
	Pulstetype_index = 0;
}

void pulse_select_set_custom_pulse(double I_quench, double I_meas, double quench_time, double wait_time){
	pselect_model.quench_curr =  I_quench;
	pselect_model.meas_curr   =  I_meas;
	pselect_model.quench_time = quench_time;
	pselect_model.wait_time   = wait_time;
}

_Bool pulse_seclect_set_linear_params(diag_pulseType * dp, uint8_t i_start,uint8_t i_end,uint8_t delta_i, uint8_t delta_t, uint8_t pulse_duration){
	if (i_end == i_start)
	{
		pselect_model.pulse_duration = ((double)pulse_duration);
		
		if(pselect_model.pulse_duration < 1 || pselect_model.pulse_duration > 250){
			return false;
		}
		pselect_model.const_current = i_start;
		
		if(pselect_model.const_current < 20 || pselect_model.const_current > 250){
			return false;
		}
		
		diag_pulse_init(dp,1,CONST);
		return true;
	}
	
	
	if(i_start > QUENCH_CURRENT_MAX  ){
		return false;
	}
	if(i_end > QUENCH_CURRENT_MAX ){
		return false;
	}
	
	if (delta_t > 50){
		return false;
	}
	
	if (delta_i > 20){
		return false;
	}
	
	
	if (i_start > i_end)
	{
		return false;
	}
	
	if((abs(i_end - i_start)) < delta_i ){
		return false;
	}
	
	uint16_t steps = (abs(i_end -i_start))/delta_i;
	
	
	if (steps > DP_NUMBER_OF_POINTS_140)
	{
		return false;
	}
	
	
	pselect_model.I_start = i_start;
	pselect_model.I_end = i_end;
	pselect_model.delta_t = ((double) delta_t)/10;
	pselect_model.delta_I = delta_i;
	diag_pulse_init(dp,1,LINEAR);
	
	return true;
	
}

void pulse_select_set_Model(uint8_t page, uint8_t option, uint8_t value){
	
	pselect_model.page = page;
	pselect_model.pageLast = page;
	pselect_model.option = option;
	pselect_model.optionLast = option;
	pselect_model.value = value;
	pselect_model.valueLast =value;
	pselect_model.valueChange = 0;
}

//ESC --> return to diag mode
void pulse_select_pressedTOP(Controller_Model *Model){
	LCD_Cls(BGC);
	paint_diag(1);
	Model->mode->next = ex_diagnostic;
	
}

void pulse_select_pressedUP(Controller_Model *Model){
	if (pselect_model.value == 0)
	{

		pselect_model.option--;
		
		if((pselect_model.page == 2)&&(pselect_model.option == 0)){
			pselect_model.option = 3;
			return;
		}


		if(pselect_model.option == 0)
		{
			pselect_model.option = 5;
		}



	}
	else
	{
		if(pselect_model.option == 1)
		{
			pselect_model.page++;
			if (pselect_model.page == 4 )
			{
				pselect_model.page = 1;
			}
			
			
			return;
		}
		pselect_model.valueChange = 1;
	}
	
}

void pulse_select_pressedLEFT(Controller_Model *Model){
	if(pselect_model.value == 1){

		pselect_model.value = 0;
	}
}

void pulse_select_pressedRIGHT(Controller_Model *Model){
	if(pselect_model.value == 0){

		pselect_model.value = 1;
	}
	
}

void pulse_select_pressedDOWN(Controller_Model *Model){
	if (pselect_model.value == 0)
	{

		pselect_model.option++;


		
		if((pselect_model.page == 2)&&(pselect_model.option == 4)){
			pselect_model.option = 1;
			return;
		}



		if(pselect_model.option == 6)
		{
			pselect_model.option = 1;
		}
	}else
	{
		if(pselect_model.option == 1)
		{
			pselect_model.page--;
			if (pselect_model.page == 0 )
			{
				pselect_model.page = 3;
			}
			
			return;
		}
		pselect_model.valueChange = -1;
	}
	
}

void pulse_select_pressedBOT(Controller_Model *Model){

	//TODO create seperate pulse Mode
	//TODO remove new quenchtime

	LCD_Cls(BGC);

	diag_pulseType dp;
	
	if (pselect_model.page == 3)
	{
		uint16_t steps = ((abs(pselect_model.I_end -pselect_model.I_start))/pselect_model.delta_I);
		
		/*
		if (pselect_model.I_start >= pselect_model.I_end)
		{
		InitScreen_AddLine("Invalid Parameters",1);
		InitScreen_AddLine("I min > I max",0);
		InitScreen_AddLine("returning to Pulse select",0);
		_delay_ms(3000);
		pulse_select_drawPage();
		return;
		}
		*/
		if (steps > (DP_NUMBER_OF_POINTS_140-1))
		{
			InitScreen_AddLine("Invalid Parameters",1);
			InitScreen_AddLine("too many steps",0);
			sprintf(LVM.temp->string,"Max allowed:%i",DP_NUMBER_OF_POINTS_140-1);
			InitScreen_AddLine(LVM.temp->string,0);
			sprintf(LVM.temp->string,"you entered:%i",steps);
			InitScreen_AddLine(LVM.temp->string,0);
			InitScreen_AddLine("returning to Pulse select",0);
			_delay_ms(3000);
			pulse_select_drawPage();
			return;
		}
		
		
		
		
		
		
	}

	


	diag_pulse_init(&dp,0, pselect_model.page);

	diag_pulse(&dp);

	pulse_select_drawPage();

	set_timeout(0, TIMER_3, RESET_TIMER);
	set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);



}


void pulse_select_pre_Switch_case_Tasks(Controller_Model *Model){
	if (Model->mode->Key != 0)
	{
		set_timeout(0, TIMER_3, RESET_TIMER);
		set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	}

	if(!set_timeout(0,TIMER_3, USE_TIMER))
	{
		LCD_Cls(BGC);
		paint_diag(1);

		Model->mode->next = ex_diagnostic;
		
	}
}

void pulse_select_post_Switch_case_Tasks(Controller_Model *Model){
	if(pselect_model.page != pselect_model.pageLast) // --> Page changed
	{
		
		not_ready_for_new_key();
		pulse_select_drawPageChange();

		pselect_model.pageLast = pselect_model.page;

		pselect_model.option = 1;
		pselect_model.optionLast = 1;
		pselect_model.value = 1;
		pselect_model.valueLast = 1;
		pselect_model.valueChange = 0;



	}else if(pselect_model.option != pselect_model.optionLast) // --> active option changed
	{
		pulse_select_optionChange();
		pselect_model.optionLast = pselect_model.option;

	}else if(pselect_model.value != pselect_model.valueLast) // --> option / Value switch
	{
		pulse_select_ValOpt_switch();
		pselect_model.valueLast = pselect_model.value;

	}else if(pselect_model.valueChange != 0) // --> Value changed
	{
		pulse_select_ValueChange();
		pselect_model.valueChange = 0;
	}

	ready_for_new_key();
	
}




void pulse_select_drawPageChange(void){
	uint8_t PageNr = pselect_model.page;
	uint8_t OptarrIndex = 0;
	
	PageNr--;
	OptarrIndex = PageNr * 5;
	
	/*
	LCD_Print("           ", xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC); //TODO box instead of empty string
	LCD_Print(optPage[PageNr], xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC);
	*/

	
	LCD_Box(PLOT_X-XOFFSET_32+xoff,PLOT_Y-yAxis_len,PLOT_X-XOFFSET_32+xoff+xAxis_len,PLOT_Y,BGC);
	
	switch(PageNr){
		case 0:
		pulseSelect_paintNormal(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
		case 1:
		pulseSelect_paintConst(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
		case 2:
		pulseSelect_paintLinear(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
	}
	
	
	LCD_Box(X_PO_2+xoff,YcoordOfOptline[0],X_PO_2+xoff+X_PO_120,YcoordOfOptline[0]+24,BGC);
	Option_StrDraw(Pulse_select_Array[OptarrIndex],FGC);
	
	Option_valueDraw(Pulse_select_Array[OptarrIndex],ERR);
	
	
	for (uint8_t i = 1; i < 5; i++ )
	{
		LCD_Box(X_PO_2+xoff,YcoordOfOptline[i],X_PO_2+xoff+X_PO_120,YcoordOfOptline[i]+24,BGC);
		
		Option_StrDraw(Pulse_select_Array[OptarrIndex+i],FGC);
		
		Option_valueDraw(Pulse_select_Array[OptarrIndex + i],FGC);
		

	}

}

void pulse_select_optionChange(void){
	uint8_t PageNr = pselect_model.page;
	uint8_t OptNr = pselect_model.option;
	uint8_t OptNrLast = pselect_model.optionLast;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptNrLast--;
	OptarrIndex = PageNr * 5;
	Option_StrDraw(Pulse_select_Array[OptarrIndex+OptNrLast],FGC);
	Option_StrDraw(Pulse_select_Array[OptarrIndex+OptNr],ERR);

	
}

void pulse_select_ValOpt_switch(void){
	uint8_t PageNr = pselect_model.page;
	uint8_t OptNr = pselect_model.option;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptarrIndex = PageNr * 5;
	if (pselect_model.value)
	{
		Option_StrDraw(Pulse_select_Array[OptarrIndex+OptNr],FGC);
		Option_valueDraw(Pulse_select_Array[OptarrIndex+OptNr],ERR);
	}
	else
	{
		Option_valueDraw(Pulse_select_Array[OptarrIndex+OptNr],FGC);
		Option_StrDraw(Pulse_select_Array[OptarrIndex+OptNr],ERR);
		
	}

	
}

void pulse_select_ValueChange(void){
	uint8_t PageNr = pselect_model.page;
	uint8_t OptNr = pselect_model.option;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptarrIndex = PageNr * 5;
	if (pselect_model.valueChange == 1)
	{
		Option_valueChange(Pulse_select_Array[OptarrIndex+OptNr],KEY_UP_S6);
		
		}else{
		Option_valueChange(Pulse_select_Array[OptarrIndex+OptNr],KEY_DOWN_S9);
	}

}

void pulse_select_drawPage(void){

	
	//LCD_Box(0+xoff,0,X_PB_144 - 1+xoff,LCD_HEIGHT,BGC);
	LCD_Cls(BGC);
	uint8_t PageNr    = pselect_model.page;
	uint8_t activeOpt = pselect_model.option;
	uint8_t OptarrIndex = 0;


	paint_buttons("esc","puls",3);
	
	LCD_Print("Select:", xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC);
	
	
	PageNr--;
	activeOpt--;
	OptarrIndex = PageNr * 5;
	
	switch(PageNr){
		case 0:
		pulseSelect_paintNormal(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
		case 1:
		pulseSelect_paintConst(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
		case 2:
		pulseSelect_paintLinear(PLOT_X-XOFFSET_32+xoff,PLOT_Y);
		break;
	}

	for (uint8_t i = 0; i < 5; i++ )
	{
		if (activeOpt == i && pselect_model.value == 0)
		{
			
			Option_StrDraw(Pulse_select_Array[OptarrIndex+i],ERR);
		}
		else
		{
			Option_StrDraw(Pulse_select_Array[OptarrIndex+i],FGC);

		}
		
		if (activeOpt == i && pselect_model.value == 1)
		{
			Option_valueDraw(Pulse_select_Array[OptarrIndex + i],ERR);
			}else{
			
			Option_valueDraw(Pulse_select_Array[OptarrIndex + i],FGC);
		}

	}

}

void pulseSelect_paintKoords(uint16_t x,uint16_t y){
	
	
	LCD_Print("t",x+xAxis_len+X_t_UNIT,y-Y_t_UNIT,1,1,1,white,BGC);
	LCD_Print("I",x-X_I_UNIT,y-yAxis_len,1,1,1,white,BGC);
	LCD_vline(x,y-(yAxis_len+1),yAxis_len,white);
	LCD_hline(x,y,xAxis_len,white);
	
}


void pulseSelect_paintNormal(uint16_t x, uint16_t y){
	
	
	Y_COODRS_NORMAL
	
	
	pulseSelect_paintKoords(x,y);
	
	for (uint8_t i = 0 ; i < X_COORDS_NORMAL;i++)
	{
		LCD_Draw(x+(2*i)+1,y-Ypoints[i]-1,x+(2*(i+1)),y-Ypoints[i+1]-1,0,red);
	}
	
	
	
	
}

void pulseSelect_paintConst(uint16_t x, uint16_t y){
	pulseSelect_paintKoords(x,y);
	
	LCD_hline(x+1,y-Y_COODRS_CONST,xAxis_len-1,red);
}

void pulseSelect_paintLinear(uint16_t x, uint16_t y){
	pulseSelect_paintKoords(x,y);
	
	LCD_Draw(x+1,y-Y_COODRS_LINEAR,x+xAxis_len-1,y-Y_AXIS_LEN,0,red);
	
}
