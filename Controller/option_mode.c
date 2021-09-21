/*
* option_mode.c
*
* Created: 01.03.2021 14:10:55
*  Author: Weges
*/


#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include "base_controller.h"
#include "option_mode.h"
#include "../display_driver.h"
#include "../display_utilities.h"
#include "../HoneywellSSC.h"
#include "../main.h"
#include "xbee.h"
#include "xbee_utilities.h"
#include "../timer_utilities.h"
#include "../keyboard.h"
#include "status.h"

#ifdef DISP_3000
#include "../StringPixelCoordTable.h"
#endif

#ifdef ili9341
#include "../StringPixelCoordTable_ili9341.h"
#endif


//===================================================================================
//Init of ValueOptions structs
// one for each row an
//========================================================
//Page1
ValueOptionsBool   shutdown_opt;
ValueOptionsChar   pos_opt;
ValueOptionsBool   autofill_opt;
ValueOptionsuInt8  he_min_opt;
ValueOptionsuInt8  fill_timeout_opt;

//Page2
ValueOptionsBool   diagmode_opt;
ValueOptionsDouble quench_current_opt;
ValueOptionsDouble quench_time_opt;
ValueOptionsDouble meas_current_opt;
ValueOptionsDouble wait_time_opt;

//Page3
ValueOptionsDouble res_min_opt;
ValueOptionsDouble res_max_opt;
ValueOptionsChar   enable_press_opt;
ValueOptionsDouble span_opt;
ValueOptionsDouble zero_opt;

//Page4
ValueOptionsuInt16 transmit_slow_opt;
ValueOptionsuInt16 transmit_fast_opt;
ValueOptionsDouble batt_min_opt;
ValueOptionsDouble batt_max_opt;
ValueOptionsuInt8  critical_batt_opt;

//Page5
ValueOptionsDouble r_span_opt;
ValueOptionsDouble r_zero_opt;
ValueOptionsuInt8  meas_cycles_opt;
ValueOptionsDouble total_volume_opt;
ValueOptionsBool   display_reversed_opt;

//Page6

ValueOptionsBool alphanum_opt;
ValueOptionsuInt8 dev_id_max_len_opt;
ValueOptionsNull Null3;
ValueOptionsNull Null4;
ValueOptionsNull Null5;



ValueOptions *OptArray[30];

uint16_t YcoordOfOptline[5] ={Y_PO_20,Y_PO_40,Y_PO_60,Y_PO_80,Y_PO_100};
char optPage[6][20]= {STR_OPTIONS_1of6,STR_OPTIONS_2of6,STR_OPTIONS_3of6,STR_OPTIONS_4of6,STR_OPTIONS_5of6,STR_OPTIONS_6of6};

// Char type option string-arrays
//Pos option
char *pos_str_arr[1] = {vars.device_pos};
//PRess enable
char *pres_enable_str_arr[3];

uint8_t pos_str_arr_ind = 0;

//dummy variables neded for object structure
_Bool shutdown = false;
_Bool diagmode = false;


optPageStrType optionStrings[]={
	{.Page[0]=STR_SHUTDOWN_OPT ,.Page[1]=STR_POS ,.Page[2]=STR_AUTOFILL ,.Page[3]=STR_HE_MIN_LVL ,.Page[4]=STR_FILLING_TIMEOUT},
	{.Page[0]=STR_DIAG,.Page[1]=STR_QUENCH_CURRENT,.Page[2]=STR_HEAT_TIME,.Page[3]=STR_MEAS_CURRENT,.Page[4]=STR_WAIT_TIME},
	{.Page[0]=STR_RES_MIN,.Page[1]=STR_RES_MAX,.Page[2]=STR_ENABLE_PR,.Page[3]=STR_SPAN,.Page[4]=STR_ZERO},
	{.Page[0]=STR_TRANSMIT_SLOW,.Page[1]=STR_TRANSMIT_FAST,.Page[2]=STR_BATTMIN,.Page[3]=STR_BATTMAX,.Page[4]=STR_CRITVOLT},
	{.Page[0]=STR_ADCSPAN,.Page[1]=STR_ADCZERO,.Page[2]=STR_MEASUREMENT_CYCLES,.Page[3]=STR_TOTAL_VOL,.Page[4]=STR_FLIP_DISP},
	{.Page[0]=STR_ALPHANUM,.Page[1]=STR_NLOGINCHARS,.Page[2]="",.Page[3]="",.Page[4]=""}
};


struct Controller_Model_Vtable option_Vtable ={
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&base_pressedDONOTHING,
	&option_pressedTOP,
	&option_pressedUP,
	&option_pressedLEFT,
	&option_pressedRIGHT,
	&option_pressedDOWN,
	&option_pressedBOT,
	&base_pressedDONOTHING,
	&base_display_always_on,
	&base_display_DONOTHING,
	&option_pre_Switch_case_Tasks,
	&option_post_Switch_case_Tasks
};

Controller_Model_options option_model ={
	.super.mode = &global_mode,
	.super.vtable = &option_Vtable,
	.batt_max_buff = BATT_MAX_DEF,
	.batt_min_buff = BATT_MIN_DEF,
	.display_reversed_buff = false,
	.option = 1,
	.optionLast = 1,
	.page =1,
	.pageLast = 1,
	.value = 0,
	.valueLast =0,
	.valueChange =0,
	.options_changed = false
};


Controller_Model_options* get_option_model(){
	return &option_model;
}

void set_options_changed(){
		option_model.options_changed = true;
}

void set_bufferVars(_Bool   options_changed){
	// some variables may not be altered during the options menu -> transfer to local variable
	option_model.display_reversed_buff = LVM.options->display_reversed;
	option_model.batt_min_buff = LVM.options->batt_min;
	option_model.batt_max_buff = LVM.options->batt_max;
	option_model.critical_batt_buff = LVM.options->critical_batt;
	option_model.options_changed = options_changed;
}


void set_OptionModel(uint8_t page, uint8_t option, uint8_t value){
	
	option_model.page = page;
	option_model.pageLast = page;
	option_model.option = option;
	option_model.optionLast = option;
	option_model.value = value;
	option_model.valueLast =value;
	option_model.valueChange = 0;
}


void option_pressedTOP(Controller_Model *Model){
	option_model.page--;
}

void option_pressedUP(Controller_Model *Model){
	if (option_model.value == 0)
	{

		option_model.option--;


		if ((LVM.options->enable_pressure != 2) && (option_model.page == 3) && (option_model.option == 0))
		{
			option_model.option = 3; // Skips the last two options if Press is not set to analog
		}
		
		if((option_model.page == 6)&&(option_model.option == 0)){
			option_model.option = 2;
		}


		if (option_model.option == 0)
		{
			option_model.option = 5;
		}



	}
	else
	{
		option_model.valueChange = 1;
	}
	
}

void option_pressedLEFT(Controller_Model *Model){
	if(option_model.value == 1){

		option_model.value = 0;
	}
}

void option_pressedRIGHT(Controller_Model *Model){
	if(option_model.value == 0){

		option_model.value = 1;
	}
	
}

void option_pressedDOWN(Controller_Model *Model){
	if (option_model.value == 0)
	{

		option_model.option++;

		if ((LVM.options->enable_pressure != 2) && (option_model.page == 3) &&(option_model.option == 4))
		{
			option_model.option = 1; // Skips the last two options if Press is not set to analog
		}
		
		if((option_model.page == 6)&&(option_model.option == 3)){
			option_model.option = 1;
		}


		if (option_model.option == 6)
		{
			option_model.option = 1;
		}
	}else
	{
		option_model.valueChange = -1;
	}
	
}

void option_pressedBOT(Controller_Model *Model){
	option_model.page++;
	
}


void option_pre_Switch_case_Tasks(Controller_Model *Model){
	if (Model->mode->Key != 0)
	{
		set_timeout(0, TIMER_3, RESET_TIMER);
		set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);
	}

	if(!set_timeout(0,TIMER_3, USE_TIMER))  //shutdown after shutdown_timout if no key was pressed
	{
		option_model.options_changed = false;
		option_exit(Model);
	}
}


void option_post_Switch_case_Tasks(Controller_Model *Model){
	if(option_model.page != option_model.pageLast) // --> Page changed
	{
		not_ready_for_new_key();
		if ((option_model.page == 0)||(option_model.page ==  7))
		{
			set_OptionModel(1,1,0);
			option_exit(Model);
			ready_for_new_key();
			return;
		}

		opt_drawPageChange();

		//paint_opt_model(&optionModel);


		option_model.pageLast = option_model.page;

		option_model.option = 1;
		option_model.optionLast = 1;
		option_model.value = 0;
		option_model.valueLast = 0;
		option_model.valueChange = 0;



	}else if(option_model.option != option_model.optionLast) // --> active option changed
	{
		opt_optionChange();
		option_model.optionLast = option_model.option;

	}else if(option_model.value != option_model.valueLast) // --> option / Value switch
	{
		opt_ValOpt_switch();
		option_model.valueLast = option_model.value;

	}else if(option_model.valueChange != 0) // --> Value changed
	{
		opt_ValueChange();
		option_model.valueChange = 0;
	}

	ready_for_new_key();
	
}







//========================================================
//Base
//========================================================


// Polymorphism  here:
void Option_valueDraw(ValueOptions*optEntry,unsigned int color)
{
	optEntry->vtable->pfvalueDraw(optEntry,color);
}

void Option_valueChange(ValueOptions*optEntry,int key)
{
	optEntry->vtable->pfvalueChange(optEntry,key);
}

void Option_StrDraw(ValueOptions*optEntry,unsigned int color){
	LCD_Print(optEntry->OptName, xoff+X_PO_2, optEntry->lineYcoord, 2, 1,1, color, BGC);
}



//===================================================================================
//Boolean
//========================================================
void Bool_valueDraw (ValueOptions*optEntry, unsigned int color)
{
	ValueOptionsBool *optEntry_ = (ValueOptionsBool *) optEntry;
	if (*optEntry_->bValue)
	{
		LCD_Print("on ",X_PO_85+xoff,optEntry_->super.lineYcoord,2,1,1,color,BGC);
		}else{
		LCD_Print("off",X_PO_85+xoff,optEntry_->super.lineYcoord,2,1,1,color,BGC);
	}
}
void Bool_valueChange (ValueOptions *optEntry, int key)
{
	ValueOptionsBool *optEntry_ = (ValueOptionsBool *) optEntry;
	if (*optEntry_->bValue)
	{
		*optEntry_->bValue = false;
		Option_valueDraw(optEntry,ERR);
	}
	else
	{
		*optEntry_->bValue = true;
		Option_valueDraw(optEntry,ERR);
		
		
	}
	
	
	if((option_model.options_changed == false) && (optEntry_->isUploaded)){
		option_model.options_changed = true;
	}
	
}

void Bool_valueChange_Shutdown(ValueOptions *optEntry, int key){
	if(LCD_Dialog(STR_SHUTDOWN_OPT,STR_DO_YOU_REALLY_WANT_SHUTDOWN, D_FGC, D_BGC,SHUTDOWN_TIMEOUT_TIME))
	{
		LCD_Cls(BGC);
		
		Controller_Model * Model = (Controller_Model *) &option_model;
		shutdown_LVM(Model);
	}
	else {
		set_timeout(OPT_TIMEOUT_TIME, TIMER_3, USE_TIMER);
		set_OptionModel(1,1,0);
		opt_drawPage();
	}
}

void Bool_valueChange_diagmode(ValueOptions*optEntry, int key){
	//increase (change) Diagnostics
	paint_diag(1);
	option_model.super.mode->next= ex_diagnostic;
}

//Boolean Vtable
struct ValueOptVtable ValueOptBoolVtable =
{
	&Bool_valueDraw,
	&Bool_valueChange
};

struct ValueOptVtable ValueOptBoolVtable_shutdown = {
	&Bool_valueDraw,
	&Bool_valueChange_Shutdown
};

struct ValueOptVtable ValueOptBoolVtable_diagmode= {
	&Bool_valueDraw,
	&Bool_valueChange_diagmode
};

void ValOptBool_init( ValueOptionsBool *optEntry, struct ValueOptVtable *bool_vtable, uint16_t lineYcoord,char *OptName, _Bool *bValue, _Bool isUploaded){

	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = bool_vtable;
	optEntry->bValue=bValue;
	optEntry->isUploaded = isUploaded;
	
	
}

//===================================================================================
//Double
//========================================================
void Double_valueDraw (ValueOptions *optEntry, unsigned int color)
{
	ValueOptionsDouble *optEntry_ = (ValueOptionsDouble *) optEntry;
	draw_double(
	*optEntry_->dValue,
	X_POVP_76 + xoff ,
	optEntry_->super.lineYcoord,
	optEntry_->precision,
	optEntry_->unit,
	color, 2
	);
}
void Double_valueChange (ValueOptions *optEntry, int key)
{
	ValueOptionsDouble *optEntry_ = (ValueOptionsDouble *) optEntry;
	if (key == KEY_DOWN_S9){
		*optEntry_->dValue -= optEntry_->increment;
	}
	else{
		*optEntry_->dValue += optEntry_->increment;
	}
	
	if(*optEntry_->dValue < optEntry_->MIN) {
		*optEntry_->dValue = optEntry_->MIN;
	}
	if(*optEntry_->dValue > optEntry_->MAX) {
		*optEntry_->dValue = optEntry_->MAX;
	}
	Option_valueDraw(optEntry,ERR);
	_delay_ms(400);

	while(keyhitwithoutinterrupt()==key)
	{
		if (key == KEY_DOWN_S9){
			*optEntry_->dValue -= optEntry_->continuousInc;
		}
		else{
			*optEntry_->dValue += optEntry_->continuousInc;
		}
		if(*optEntry_->dValue < optEntry_->MIN) {
			*optEntry_->dValue = optEntry_->MIN;
		}
		if(*optEntry_->dValue > optEntry_->MAX) {
			*optEntry_->dValue = optEntry_->MAX;
		}
		Option_valueDraw(optEntry,ERR);
		_delay_ms(50);
	}
	
	
	if((option_model.options_changed == false) && (optEntry_->isUploaded)){
		option_model.options_changed = true;
	}
	
	
}


//Double Vtable
struct ValueOptVtable ValueOptDoubleVtable =
{
	&Double_valueDraw,
	&Double_valueChange
};


void ValOptDouble_init(ValueOptionsDouble *optEntry, uint16_t lineYcoord,char *OptName, double *dValue, double increment, double continuousInc,double MAX,double MIN, uint8_t pecision, char* unit,_Bool isUploaded)
{
	
	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = &ValueOptDoubleVtable;
	optEntry->dValue = dValue;
	optEntry->increment = increment;
	optEntry->continuousInc = continuousInc;
	optEntry->MAX = MAX;
	optEntry->MIN = MIN;
	optEntry->precision = pecision;
	optEntry->unit = unit;
	optEntry->isUploaded = isUploaded;
}
//===================================================================================
//uint16_t
//========================================================
void uInt16_valueDraw (ValueOptions *optEntry, unsigned int color)
{
	ValueOptionsuInt16 *optEntry_ = (ValueOptionsuInt16 *) optEntry;
	(*optEntry_->unit_selector) ?
	draw_int(*optEntry_->uint16_Value, xoff+X_PO_85, optEntry_->super.lineYcoord, optEntry_->unit_1, color, 2)
	: 	draw_int(*optEntry_->uint16_Value, xoff+X_PO_85, optEntry_->super.lineYcoord, optEntry_->unit_2,color, 2);
}
void uInt16_valueChange (ValueOptions *optEntry, int key)
{
	ValueOptionsuInt16 *optEntry_ = (ValueOptionsuInt16 *) optEntry;
	//decrease Value
	if (key == KEY_DOWN_S9){
		*optEntry_->uint16_Value -= optEntry_->increment;
	}
	else{
		*optEntry_->uint16_Value+= optEntry_->increment;
	}
	if (*optEntry_->unit_selector)
	{
		if(*optEntry_->uint16_Value < optEntry_->MIN) {
			*optEntry_->uint16_Value = optEntry_->MIN;
		}
		if(*optEntry_->uint16_Value > 60)
		{
			*optEntry_->uint16_Value = 1;
			*optEntry_->unit_selector = false;
			
		}
		Option_valueDraw(optEntry,ERR);
		
	}
	else
	{
		if (*optEntry_->uint16_Value < 1)
		{
			*optEntry_->uint16_Value = 60;
			*optEntry_->unit_selector = true;
		}
		else
		{
			if(*optEntry_->uint16_Value > optEntry_->MAX) {
				*optEntry_->uint16_Value = optEntry_->MAX;
			}
			
		}
		Option_valueDraw(optEntry,ERR);
	}

	_delay_ms(400);

	while(keyhitwithoutinterrupt()==key)
	{
		if (key == KEY_DOWN_S9){
			*optEntry_->uint16_Value -= optEntry_->increment;
		}
		else{
			*optEntry_->uint16_Value += optEntry_->increment;
		}
		if (*optEntry_->unit_selector)
		{
			if(*optEntry_->uint16_Value < optEntry_->MIN) {
				*optEntry_->uint16_Value = optEntry_->MIN;
			}
			if(*optEntry_->uint16_Value > 60)
			{
				*optEntry_->uint16_Value = 1;
				*optEntry_->unit_selector = false;

			}
			Option_valueDraw(optEntry,ERR);
			
		}
		else
		{
			if (*optEntry_->uint16_Value < 1)
			{
				*optEntry_->uint16_Value = 60;
				*optEntry_->unit_selector = true;
			}
			else
			{
				if(*optEntry_->uint16_Value > optEntry_->MAX) {
					*optEntry_->uint16_Value = optEntry_->MAX;
				}
				
			}
			Option_valueDraw(optEntry,ERR);
		}

		_delay_ms(50);
		

	}

	option_model.options_changed = true;
	*optEntry_->val_changed = true;
}


//u_Int16 Vtable
struct ValueOptVtable ValueOptuInt16Vtable =
{
	&uInt16_valueDraw,
	&uInt16_valueChange
};


void ValOptInt16_init(ValueOptionsuInt16 *optEntry, uint16_t lineYcoord,char *OptName,uint16_t *uint16_value, uint8_t increment,uint16_t MAX, uint16_t MIN,_Bool *unit_selector,_Bool *val_changed, char *unit_1, char *unit_2)
{
	
	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = &ValueOptuInt16Vtable;
	optEntry->uint16_Value = uint16_value;
	optEntry->increment = increment;
	optEntry->MAX = MAX;
	optEntry->MIN = MIN;
	optEntry->unit_selector = unit_selector;
	optEntry->val_changed = val_changed;
	optEntry->unit_1 = unit_1;
	optEntry->unit_2 = unit_2;
	
}

//===================================================================================
//uint8_t
//========================================================
void uInt8_valueDraw (ValueOptions *optEntry, unsigned int color)
{
	ValueOptionsuInt8 *optEntry_ = (ValueOptionsuInt8 *) optEntry;
	draw_int(*optEntry_->uint8_Value,X_PO_85+xoff,optEntry_->super.lineYcoord,optEntry_->unit,color, 2);
}
void uInt8_valueChange (ValueOptions *optEntry, int key)
{
	ValueOptionsuInt8 *optEntry_ = (ValueOptionsuInt8 *) optEntry;
	if (key == KEY_DOWN_S9){
		*optEntry_->uint8_Value -= optEntry_->increment;
	}
	else{
		*optEntry_->uint8_Value += optEntry_->increment;
	}
	
	if(*optEntry_->uint8_Value < optEntry_->MIN) {
		*optEntry_->uint8_Value = optEntry_->MIN;
	}
	if(*optEntry_->uint8_Value > optEntry_->MAX) {
		*optEntry_->uint8_Value = optEntry_->MAX;
	}
	Option_valueDraw(optEntry,ERR);
	_delay_ms(400);

	while(keyhitwithoutinterrupt()==key)
	{
		if (key == KEY_DOWN_S9){
			*optEntry_->uint8_Value -= optEntry_->increment;
		}
		else{
			*optEntry_->uint8_Value += optEntry_->increment;
		}
		if(*optEntry_->uint8_Value < optEntry_->MIN) {
			*optEntry_->uint8_Value = optEntry_->MIN;
		}
		if(*optEntry_->uint8_Value > optEntry_->MAX) {
			*optEntry_->uint8_Value = optEntry_->MAX;
		}
		Option_valueDraw(optEntry,ERR);
		_delay_ms(50);
	}
	//}
	option_model.options_changed = true;
}


//u_Int8 Vtable
struct ValueOptVtable ValueOptuInt8Vtable =
{
	&uInt8_valueDraw,
	&uInt8_valueChange
};

void ValOptInt8_init(ValueOptionsuInt8 *optEntry, uint16_t lineYcoord,char *OptName, uint8_t *uint8_Value, uint8_t increment,uint8_t MAX, uint8_t MIN, char *unit)
{
	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = &ValueOptuInt8Vtable;
	optEntry->uint8_Value = uint8_Value;
	optEntry->increment = increment;
	optEntry->MAX = MAX;
	optEntry->MIN = MIN;
	optEntry->unit = unit;
}

//===================================================================================
//Char*
//========================================================

void Char_valueDraw (ValueOptions *optEntry, unsigned int color)
{
	ValueOptionsChar *optEntry_ = (ValueOptionsChar *) optEntry;
	LCD_Print("      " ,X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,BGC,BGC);
	LCD_Print(optEntry_->string_arr[*optEntry_->arr_ind],X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,color,BGC);
}
void Char_valueChange (ValueOptions *optEntry, int key)
{

	
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
	
	Option_valueDraw(optEntry,ERR);
	
	if (*optEntry_->arr_ind == 2)// draw span and zero
	{
		Option_StrDraw(OptArray[13],FGC);
		Option_valueDraw(OptArray[13],FGC);
		Option_StrDraw(OptArray[14],FGC);
		Option_valueDraw(OptArray[14],FGC);
	}else //erase span and zero
	{
		Option_StrDraw(OptArray[13],BGC);
		Option_valueDraw(OptArray[13],BGC);
		Option_StrDraw(OptArray[14],BGC);
		Option_valueDraw(OptArray[14],BGC);
	}
	_delay_ms(400);

}

//Char Vtable
struct ValueOptVtable ValueOptCharVtable =
{
	&Char_valueDraw,
	&Char_valueChange
	
};


void ValOptChar_init(ValueOptionsChar *optEntry, struct ValueOptVtable *char_vtable, uint16_t lineYcoord,char *OptName, char **string_arr ,uint8_t* arr_ind,	uint8_t MIN, uint8_t MAX){
	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = char_vtable;
	optEntry->string_arr = string_arr;
	optEntry->arr_ind = arr_ind;
	optEntry->MAX = MAX;
	optEntry->MIN = MIN;
}

//===================================================================================
//NULL OPT
//========================================================

void null_valueDraw (ValueOptions *optEntry, unsigned int color)
{
	/*
	ValueOptionsNull *optEntry_ = (ValueOptionsNull *) optEntry;
	LCD_Print("      " ,X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,BGC,BGC);
	LCD_Print("",X_POVP_76+xoff,optEntry_->super.lineYcoord,2,1,1,color,BGC);
	*/
}

void null_valueChange (ValueOptions *optEntry, int key)
{
	// nothing to do here

}

//null Vtable
struct ValueOptVtable ValueOptNullVtable =
{
	&null_valueDraw,
	&null_valueChange
};


void ValOptNull_init(ValueOptionsNull *optEntry, uint16_t lineYcoord,char *OptName, char * optval){
	optEntry->super.lineYcoord = lineYcoord;
	optEntry->super.OptName = OptName;
	optEntry->super.vtable = &ValueOptNullVtable;
	optEntry->optval = optval;

}




void  OptentrysInit(){

	pres_enable_str_arr[0] = "off";
	pres_enable_str_arr[1] = "I2C";
	pres_enable_str_arr[2] = "analog";

	
	//page1
	ValOptBool_init(&shutdown_opt,&ValueOptBoolVtable_shutdown,YcoordOfOptline[0],optionStrings[0].Page[0],&shutdown,true);
	ValOptChar_init(&pos_opt, &ValueOptCharVtable,YcoordOfOptline[1],optionStrings[0].Page[1],pos_str_arr,&pos_str_arr_ind,0,0);
	ValOptBool_init(&autofill_opt,&ValueOptBoolVtable,YcoordOfOptline[2],optionStrings[0].Page[2],&LVM.vars->auto_fill_enabled,false);
	ValOptInt8_init(&he_min_opt,YcoordOfOptline[3],optionStrings[0].Page[3],&LVM.options->he_min,1,MAX_AUTO_FILL_HE,MIN_AUTO_FILL_HE,"%");
	ValOptInt8_init(&fill_timeout_opt,YcoordOfOptline[4],optionStrings[0].Page[4],&LVM.options->fill_timeout,1,MAX_FILLING_TIMEOUT,MIN_FILLING_TIMEOUT,"min");
	
	//page2
	ValOptBool_init(&diagmode_opt,&ValueOptBoolVtable_diagmode,YcoordOfOptline[0],optionStrings[1].Page[0],&diagmode,true);
	ValOptDouble_init(&quench_current_opt,YcoordOfOptline[1],optionStrings[1].Page[1],&LVM.options->quench_current,1,1,QUENCH_CURRENT_MAX,QUENCH_CURRENT_MIN,5,"mA",true);
	ValOptDouble_init(&quench_time_opt,YcoordOfOptline[2],optionStrings[1].Page[2],&LVM.options->quench_time,0.1,0.1,QUENCH_TIME_MAX,QUENCH_TIME_MIN,1,"s",true);
	ValOptDouble_init(&meas_current_opt,YcoordOfOptline[3],optionStrings[1].Page[3],&LVM.options->meas_current,1,1,MEAS_CURRENT_MAX,MEAS_CURRENT_MIN,5,"mA",true);
	ValOptDouble_init(&wait_time_opt,YcoordOfOptline[4],optionStrings[1].Page[4],&LVM.options->wait_time,0.1,0.1,WAIT_TIME_MAX,WAIT_TIME_MIN,1,"s",true);
	
	//page3
	ValOptDouble_init(&res_min_opt,YcoordOfOptline[0],optionStrings[2].Page[0],&LVM.options->res_min,0.1,1,RES_MIN_MAX,RES_MIN_MIN,1,"o",true);
	ValOptDouble_init(&res_max_opt,YcoordOfOptline[1],optionStrings[2].Page[1],&LVM.options->res_max,0.1,1,RES_MAX_MAX,RES_MAX_MIN,1,"o",true);
	ValOptChar_init(&enable_press_opt, &ValueOptCharVtable,YcoordOfOptline[2],optionStrings[2].Page[2],pres_enable_str_arr,&LVM.options->enable_pressure,0,2);
	ValOptDouble_init(&span_opt,YcoordOfOptline[3],optionStrings[2].Page[3],&LVM.options->span,0.1,1,MAX_SPAN,MIN_SPAN,1,"",true);
	ValOptDouble_init(&zero_opt,YcoordOfOptline[4],optionStrings[2].Page[4],&LVM.options->zero,0.1,1,MAX_ZERO,MIN_ZERO,1,"",true);
	
	//page4
	ValOptInt16_init(&transmit_slow_opt,YcoordOfOptline[0],optionStrings[3].Page[0],&LVM.options->transmit_slow,1,TRANSMIT_SLOW_MAX,TRANSMIT_SLOW_MIN,&LVM.options->transmit_slow_min,&LVM.vars->transmit_slow_changed,"min","h");
	ValOptInt16_init(&transmit_fast_opt,YcoordOfOptline[1],optionStrings[3].Page[1],&LVM.options->transmit_fast,1,TRANSMIT_FAST_MAX,TRANSMIT_FAST_MIN,&LVM.options->transmit_fast_sec,&LVM.vars->transmit_fast_changed,"sec","min");
	ValOptDouble_init(&batt_min_opt,YcoordOfOptline[2],optionStrings[3].Page[2],&option_model.batt_min_buff,0.1,0.1,BATT_MIN_MAX,BATT_MIN_MIN,1,"V",true);
	ValOptDouble_init(&batt_max_opt,YcoordOfOptline[3],optionStrings[3].Page[3],&option_model.batt_max_buff,0.1,0.1,BATT_MAX_MAX,BATT_MAX_MIN,1,"V",true);
	ValOptInt8_init(&critical_batt_opt,YcoordOfOptline[4],optionStrings[3].Page[4],&option_model.critical_batt_buff,1,99,1,"%");
	
	//page5
	ValOptDouble_init(&r_span_opt,YcoordOfOptline[0],optionStrings[4].Page[0],&LVM.options->r_span,0.001,0.01,100,-100,3,"",false);
	ValOptDouble_init(&r_zero_opt,YcoordOfOptline[1],optionStrings[4].Page[1],&LVM.options->r_zero,0.1,0.1,100,-100,1,"",false);
	ValOptInt8_init(&meas_cycles_opt,YcoordOfOptline[2],optionStrings[4].Page[2],&LVM.options->meas_cycles,1,MEASUREMENT_CYCLES_MAX,MEASUREMENT_CYCLES_MIN,"c");
	ValOptDouble_init(&total_volume_opt,YcoordOfOptline[3],optionStrings[4].Page[3],&LVM.options->total_volume,0,0,TOTAL_VOL_MAX,TOTAL_VOL_MIN,1,"l",false);
	ValOptBool_init(&display_reversed_opt,&ValueOptBoolVtable,YcoordOfOptline[4],optionStrings[4].Page[4],&option_model.display_reversed_buff,true);
	
	//page6
	ValOptBool_init(&alphanum_opt,&ValueOptBoolVtable,YcoordOfOptline[0],optionStrings[5].Page[0],&LVM.options->Dev_ID_alpahnum,false);
	ValOptInt8_init(&dev_id_max_len_opt,YcoordOfOptline[1],optionStrings[5].Page[1],&LVM.options->Dev_ID_Max_len,1,DEV_ID_CHARS_MAX,DEV_ID_CHARS_MIN,"");
	ValOptNull_init(&Null3,YcoordOfOptline[2],optionStrings[5].Page[2],"");
	ValOptNull_init(&Null4,YcoordOfOptline[3],optionStrings[5].Page[3],"");
	ValOptNull_init(&Null5,YcoordOfOptline[4],optionStrings[5].Page[4],"");
	
	
	OptArray[0]= (ValueOptions* )&shutdown_opt;
	OptArray[1]= (ValueOptions* )&pos_opt;
	OptArray[2]= (ValueOptions* )&autofill_opt;
	OptArray[3]= (ValueOptions* )&he_min_opt;
	OptArray[4]= (ValueOptions* )&fill_timeout_opt;
	
	OptArray[5]= (ValueOptions* )&diagmode_opt;
	OptArray[6]= (ValueOptions* )&quench_current_opt;
	OptArray[7]= (ValueOptions* )&quench_time_opt;
	OptArray[8]= (ValueOptions* )&meas_current_opt;
	OptArray[9]= (ValueOptions* )&wait_time_opt;
	
	OptArray[10]= (ValueOptions* )&res_min_opt;
	OptArray[11]= (ValueOptions* )&res_max_opt;
	OptArray[12]= (ValueOptions* )&enable_press_opt;
	OptArray[13]= (ValueOptions* )&span_opt;
	OptArray[14]= (ValueOptions* )&zero_opt;
	
	OptArray[15]= (ValueOptions* )&transmit_slow_opt;
	OptArray[16]= (ValueOptions* )&transmit_fast_opt;
	OptArray[17]= (ValueOptions* )&batt_min_opt;
	OptArray[18]= (ValueOptions* )&batt_max_opt;
	OptArray[19]= (ValueOptions* )&critical_batt_opt;
	
	OptArray[20]= (ValueOptions* )&r_span_opt;
	OptArray[21]= (ValueOptions* )&r_zero_opt;
	OptArray[22]= (ValueOptions* )&meas_cycles_opt;
	OptArray[23]= (ValueOptions* )&total_volume_opt;
	OptArray[24]= (ValueOptions* )&display_reversed_opt;
	
	OptArray[25]= (ValueOptions* )&alphanum_opt;
	OptArray[26]= (ValueOptions* )&dev_id_max_len_opt;
	OptArray[27]= (ValueOptions* )&Null3;
	OptArray[28]= (ValueOptions* )&Null4;
	OptArray[29]= (ValueOptions* )&Null5;

}

void opt_drawPageChange(){

	uint8_t PageNr = option_model.page;
	uint8_t PageNrLast = option_model.pageLast;
	uint8_t OptarrIndex = 0;
	

	
	if (PageNr == 1)
	{
		if (!LVM.options->display_reversed)
		{
			LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			//LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			LCD_Print("esc",X_PB_159-(3*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			//LCD_Print("next",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
		else
		{
			LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			//LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			LCD_Print("esc",X_PB_15-(3*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			//LCD_Print("next",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
	}

	if (PageNr == 6)
	{
		if (!LVM.options->display_reversed)
		{
			//LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			//LCD_Print("prev",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("esc",X_PB_159-(3*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
		else
		{
			//LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			//LCD_Print("prev",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("esc",X_PB_15-(3*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
	}
	if ( ((PageNr == 2)&&(PageNrLast == 1)) || ((PageNr == 5)&&(PageNrLast == 6)) )
	{
		if (!LVM.options->display_reversed)
		{
			LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("    ",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			LCD_Print("prev",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("next",X_PB_159-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
		else
		{
			LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("    ",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
			
			LCD_Print("prev",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_5,FONTNR_PB_1,1,2,FGC, BGC);
			LCD_Print("next",X_PB_15-(4*LD_HALF_CHAR_WIDTH)+1,Y_PB_109,FONTNR_PB_1,1,2,FGC, BGC);
		}
	}

	PageNr--;
	OptarrIndex = PageNr * 5;
	LCD_Print("           ", xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC); //TODO box instead of empty string
	LCD_Print(optPage[PageNr], xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC);
	
	LCD_Box(X_PO_2+xoff,YcoordOfOptline[0],X_PO_2+xoff+X_PO_120,YcoordOfOptline[0]+24,BGC);
	Option_StrDraw(OptArray[OptarrIndex],ERR);
	Option_valueDraw(OptArray[OptarrIndex],FGC);
	
	
	for (uint8_t i = 1; i < 5; i++ )
	{
		LCD_Box(X_PO_2+xoff,YcoordOfOptline[i],X_PO_2+xoff+X_PO_120,YcoordOfOptline[i]+24,BGC);
		
		if (!(option_model.page== 3 && LVM.options->enable_pressure != 2 && i > 2))
		{	// skip drawing span and zero if analog pressure is not selected
			Option_StrDraw(OptArray[OptarrIndex+i],FGC);
			
			Option_valueDraw(OptArray[OptarrIndex + i],FGC);
		}

	}

}

void opt_ValOpt_switch(){

	uint8_t PageNr = option_model.page;
	uint8_t OptNr = option_model.option;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptarrIndex = PageNr * 5;
	if (option_model.value)
	{
		//Pos Change:
		if ((option_model.page == 1)&&(option_model.option == 2))
		{
			strcpy(LVM.temp->string, "none");
			if(update_filling_pos(LVM.pos, LVM.temp->string)) strcpy(LVM.vars->device_pos, LVM.temp->string);

			display_on();    // otherwise the display would turn black just after returning
			option_model.value = 0;  // focus goes back to the left side
			option_model.valueLast =0;
			opt_drawPage();
			return;
		}
		Option_StrDraw(OptArray[OptarrIndex+OptNr],FGC);
		Option_valueDraw(OptArray[OptarrIndex+OptNr],ERR);
	}
	else
	{
		Option_valueDraw(OptArray[OptarrIndex+OptNr],FGC);
		Option_StrDraw(OptArray[OptarrIndex+OptNr],ERR);
		
	}
	
}

void opt_optionChange(){
	

	
	uint8_t PageNr = option_model.page;
	uint8_t OptNr = option_model.option;
	uint8_t OptNrLast = option_model.optionLast;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptNrLast--;
	OptarrIndex = PageNr * 5;
	Option_StrDraw(OptArray[OptarrIndex+OptNrLast],FGC);
	Option_StrDraw(OptArray[OptarrIndex+OptNr],ERR);

}

void opt_drawPage(){

	
	//LCD_Box(0+xoff,0,X_PB_144 - 1+xoff,LCD_HEIGHT,BGC);
	LCD_Cls(BGC);
	uint8_t PageNr    = option_model.page;
	uint8_t activeOpt = option_model.option;
	uint8_t OptarrIndex = 0;
	if (PageNr == 1){
		paint_buttons("esc","next",3);
		}else if (PageNr == 5){
		paint_buttons("prev","esc",3);
		}else{
		paint_buttons("prev","next",3);
	}
	
	
	PageNr--;
	activeOpt--;
	OptarrIndex = PageNr * 5;
	LCD_Print(optPage[PageNr], xoff+X_PO_25, Y_PO_2, 2, 1,1, ERR, BGC);
	for (uint8_t i = 0; i < 5; i++ )
	{
		if (activeOpt == i)
		{
			Option_StrDraw(OptArray[OptarrIndex+i],ERR);
		}
		else
		{
			Option_StrDraw(OptArray[OptarrIndex+i],FGC);
		}
		
		Option_valueDraw(OptArray[OptarrIndex + i],FGC);
	}
}

void opt_ValueChange(){
	uint8_t PageNr = option_model.page;
	uint8_t OptNr = option_model.option;
	uint8_t OptarrIndex = 0;
	OptNr--;
	PageNr--;
	OptarrIndex = PageNr * 5;
	if(option_model.valueChange == 1)
	{
		Option_valueChange(OptArray[OptarrIndex+OptNr],KEY_UP_S6);
		
		}else{
		Option_valueChange(OptArray[OptarrIndex+OptNr],KEY_DOWN_S9);
	}
}



void option_exit(Controller_Model * Model){
	// Set buffered variables
	LVM.options->display_reversed = option_model.display_reversed_buff;
	LVM.options->batt_min = option_model.batt_min_buff;
	LVM.options->batt_max = option_model.batt_max_buff;
	LVM.options->critical_batt = option_model.critical_batt_buff;

	// Set display orientation
	DISPLAY_CONFIG
	xoff = (!LVM.options->display_reversed)? 0 : XOffset;


	if(Model->mode->netstat == online)
	{
		// Dialog about changed options
		if (option_model.options_changed)
		{
			if(!LCD_Dialog(STR_OPTIONS, STR_UPLOAD_CHANGES_TO_DATABASE, D_FGC, D_BGC,OPT_CHANGED_TIMEOUT_TIME))
			option_model.options_changed = false;  // Escape without saving
		}
		if(option_model.options_changed)
		{
			xbee_wake_up_plus();
			

			uint8_t index = write_Opts_to_Buffer(LVM.temp->buffer);

			InitScreen_AddLine(STR_SAVING_SETTINGS,1);
			InitScreen_AddLine(STR_TO_THE_SERVER,0);

			#ifdef ALLOW_COM
			if (xbee_send_request(OPTIONS_CHANGED_MSG, LVM.temp->buffer, index) == 0xFF)
			InitScreen_AddLine(STR_UN_SUCCESSFUL,0);
			else InitScreen_AddLine(STR_SUCCESSFUL,0);
			// comment: message is not stored if communication to database server is not working
			#else
			// Pack full frame with 64-bit address (neither acknowledgment nor response frame), then send to the database server
			if (xbee_send_message(OPTIONS_CHANGED_MSG, LVM.temp->buffer, index))
			{
				CLEAR_ALL(); // Clear all errors
			}
			#endif
			// Free XBee module
			

			//to read the messages
			_delay_ms(2000);

		}

	}
	paint_main(Time, Model->mode->netstat, PAINT_ALL);
	Model->mode->next = ex_main;
	option_model.options_changed = false;

	#ifdef ALLOW_EEPROM_SAVING
	write_opts_to_EEPROM();
	#endif
	
	base_display_extend_onTime();

}

void make_he_vol_changable(void)
{

		total_volume_opt.increment= 0.1;
		total_volume_opt.continuousInc = 1;
		total_volume_opt.MIN = TOTAL_VOL_MIN;
		total_volume_opt.MAX = TOTAL_VOL_MAX;
}

void paint_opt_model(){
	char temp_print[40];

	sprintf(temp_print,"Page: %i",option_model.page);
	LCD_Print(temp_print,60,50,2,1,1,red,BGC);
	sprintf(temp_print,"PageLast: %i",option_model.pageLast);
	LCD_Print(temp_print,60,50+(1*FONT2_H),2,1,1,red,BGC);
	sprintf(temp_print,"Option: %i",option_model.option);
	LCD_Print(temp_print,60,50+(2*FONT2_H),2,1,1,red,BGC);
	sprintf(temp_print,"OptionLast: %i",option_model.optionLast);
	LCD_Print(temp_print,60,50+(3*FONT2_H),2,1,1,red,BGC);
	sprintf(temp_print,"Value: %i",option_model.value);
	LCD_Print(temp_print,60,50+(4*FONT2_H),2,1,1,red,BGC);
	sprintf(temp_print,"ValueLast: %i",option_model.valueLast);
	LCD_Print(temp_print,60,50+(5*FONT2_H),2,1,1,red,BGC);
}

