/*
 * main_mode.h
 *
 * Created: 23.02.2021 16:39:49
 *  Author: Weges
 */ 


#ifndef MAIN_MODE_H
#define MAIN_MODE_H

#include "base_controller.h"

//=====================================================================
// MAIN MODE
//=====================================================================



typedef struct 
{
	Controller_Model super;
	//TODO
}Controller_Model_main;

extern globalModesType global_mode;


Controller_Model_main* get_main_model(void);

void main_pressedFILL(Controller_Model *Model);
void main_pressedMeasure(Controller_Model *Model);
//void main_pressedHIDDEN(Controller_Model *Model);
//void main_pressedTOP(Controller_Model *Model);
//void main_pressedUP(Controller_Model *Model);
void main_pressedLEFT(Controller_Model *Model);
void main_pressedRIGHT(Controller_Model *Model);
void main_pressedDOWN(Controller_Model *Model);
void main_pressedBOT(Controller_Model *Model);
void main_pressedNONE(Controller_Model *Model);


void main_pre_Switch_case_Tasks(Controller_Model *Model);
void main_post_Switch_case_Tasks(Controller_Model *Model);

void autofill_check(Controller_Model * Model);



#endif /* MAIN_MODE_H */