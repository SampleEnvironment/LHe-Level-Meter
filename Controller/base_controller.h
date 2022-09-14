/*
* base_controller.h
*
* Created: 23.02.2021 17:56:31
*  Author: Weges
*/


#ifndef BASE_CONTROLLER_H_
#define BASE_CONTROLLER_H_

#include "../main.h"


extern LVM_ModelType LVM;



enum PARENT_MODE
{
	offline,
	online
};

enum CHILD_MODE
{
	ex_main,
	ex_filling,
	ex_options,
	ex_diagnostic,
	ex_diag_pulse,
	ex_pulse_select,
	ex_error
};




enum ERROR_CODE
{
	no_error,
	login_failed,
	shutdown_failed,
	offlie
};

typedef struct{
		enum PARENT_MODE netstat;
		enum CHILD_MODE curr;
		enum CHILD_MODE next;
		enum ERROR_CODE error;
		int Key;
}globalModesType;



//===================================================================================
//Base Vtable
//========================================================

struct Controller_Model_Vtable;

typedef struct Controller_Model
{
	struct Controller_Model_Vtable const *vtable;
	globalModesType * mode;
	_Bool batt_check;
	
}Controller_Model;

typedef void  (* Button_pressed_type)(Controller_Model*);
typedef void  (* None_pressed_type)(Controller_Model*);
typedef _Bool (* Display_State_type)(void);
typedef void  (* Static_Task_type )(Controller_Model*);


struct Controller_Model_Vtable
{
	Button_pressed_type   pressedFILL;
	Button_pressed_type   pressedMeasure;
	Button_pressed_type   pressedHIDDEN;
	Button_pressed_type   pressedTOP;
	Button_pressed_type   pressedUP;
	Button_pressed_type   pressedLEFT;
	Button_pressed_type   pressedRIGHT;
	Button_pressed_type   pressedDOWN;
	Button_pressed_type   pressedBOT;
	None_pressed_type     pressedNONE;
	
	Display_State_type    display_on_pressed;
	Display_State_type    display_off_pressed;
	
	Static_Task_type      pre_Switch_case_Tasks;
	Static_Task_type      post_Switch_case_Tasks;
		
};



void pressedFILL(Controller_Model *Model); 
void pressedMeasure(Controller_Model *Model);
void pressedHIDDEN(Controller_Model *Model);
void pressedTOP(Controller_Model *Model);
void pressedUP(Controller_Model *Model);
void pressedLEFT(Controller_Model *Model);
void pressedRIGHT(Controller_Model *Model);
void pressedDOWN(Controller_Model *Model);
void pressedBOT(Controller_Model *Model);
void pressedNONE(Controller_Model *Model);


void display_on_pressed(Controller_Model *Model);
void display_off_pressed(Controller_Model *Model);

void pre_Switch_case_Tasks(Controller_Model *Model);
void post_Switch_case_Tasks(Controller_Model *Model);

Controller_Model * Controller(Controller_Model *Model);


_Bool base_display_on_pressed(void);
_Bool base_display_always_on(void);
_Bool base_display_off_pressed(void);
_Bool base_display_DONOTHING(void);

void base_display_extend_onTime(void);
void set_Options(uint8_t * optBuffer,uint8_t OpCode);
uint8_t Options_Buonds_Check(optionsType * optBuff);
uint8_t write_Opts_to_Buffer(uint8_t * sendbuff);
void addline_number(char * text,uint32_t number);
void base_pressedDONOTHING(Controller_Model *Model);

void measure(Controller_Model * Model);
void shutdown_LVM(Controller_Model * Model);

void manage_Xbee_sleep_cycles(Controller_Model *Model);
void send_awake_message(Controller_Model *Model);

void interval_slow_changed(Controller_Model *Model);
void Battery_check(Controller_Model *Model);
void handle_received_Messages(Controller_Model *Model);
double get_he_level(double res_min, double res_max, double r_span, double r_zero, uint8_t count, double quench_time, double quench_current, double wait_time, double meas_current, uint8_t show_progress);
#endif /* BASE_CONTROLLER_H_ */