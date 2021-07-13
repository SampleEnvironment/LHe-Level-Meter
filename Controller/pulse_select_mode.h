/*
 * pulse_select_mode.h
 *
 * Created: 22.03.2021 13:13:42
 *  Author: Weges
 */ 


#ifndef PULSE_SELECT_MODE_H_
#define PULSE_SELECT_MODE_H_

//=====================================================================
// Pulse Select MODE
//=====================================================================
#include <stdint.h>
#include "base_controller.h"
#include "option_mode.h"

extern globalModesType global_mode;
extern uint16_t YcoordOfOptline[5];




typedef struct
{
	Controller_Model super;
	uint8_t page;
	uint8_t pageLast;
	uint8_t option;
	uint8_t optionLast;
	uint8_t value;
	uint8_t valueLast;
	int8_t  valueChange;
	_Bool   options_changed; //dummy var
	double pulse_duration; //pulse duration in seconds
	double const_current;  //constant current in pulse

	double delta_I; // delta I in mA
	double delta_t; // delta t in ms
	double I_min ; // I_min in mA
	double I_max ;   //I_max in mA
	
	double quench_curr;
	double quench_time;
	double meas_curr; 
	double wait_time;
}Controller_Model_pulse_select;




void pulse_select_Init(void);


Controller_Model_pulse_select* get_pulse_select_model(void);

void pulse_select_set_Model(uint8_t page, uint8_t option, uint8_t value);
_Bool pulse_seclect_set_linear_params(uint8_t i_min,uint8_t i_max,uint8_t delta_i, uint8_t delta_t);

//void main_pressedFILL(Controller_Model *Model);
//void main_pressedMeasure(Controller_Model *Model);
//void main_pressedHIDDEN(Controller_Model *Model);
void pulse_select_pressedTOP(Controller_Model *Model);
void pulse_select_pressedUP(Controller_Model *Model);
void pulse_select_pressedLEFT(Controller_Model *Model);
void pulse_select_pressedRIGHT(Controller_Model *Model);
void pulse_select_pressedDOWN(Controller_Model *Model);
void pulse_select_pressedBOT(Controller_Model *Model);
//void main_pressedNONE(Controller_Model *Model);

void pulse_select_pre_Switch_case_Tasks(Controller_Model *Model); 
void pulse_select_post_Switch_case_Tasks(Controller_Model *Model); 

void Char_valueDraw_pulse_select (ValueOptions *optEntry, unsigned int color);
void Char_valueChange_pulse_select (ValueOptions *optEntry, int key);

void pulse_select_drawPageChange(void);
void pulse_select_optionChange(void);
void pulse_select_ValOpt_switch(void);
void pulse_select_ValueChange(void);
void pulse_select_drawPage(void);


void pulseSelect_paintKoords(uint16_t x,uint16_t y);
void pulseSelect_paintNormal(uint16_t x, uint16_t y);
void pulseSelect_paintConst(uint16_t x, uint16_t y);
void pulseSelect_paintLinear(uint16_t x, uint16_t y);

#endif /* PULSE_SELECT_MODE_H_ */