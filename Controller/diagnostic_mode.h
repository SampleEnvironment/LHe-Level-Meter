/*
* diagnostic_mode.h
*
* Created: 01.03.2021 11:35:37
*  Author: Weges
*/


#ifndef DIAGNOSTIC_MODE_H_
#define DIAGNOSTIC_MODE_H_

//=====================================================================
// DIAGNOSTIC MODE
//=====================================================================

#define CALIB_LOOPS 120

typedef struct
{
	Controller_Model super;
	uint8_t page_Nr;
	//Calibration
	double 		temp_r_span;
	double 		temp_r_zero ;
	double		temp_rmin_calc;
	double		temp_rmax_calc;
}Controller_Model_diagnostic;


extern globalModesType global_mode;

Controller_Model_diagnostic* get_diag_model(void);

//void diag_pressedFILL(Controller_Model *Model);
//void diag_pressedMeasure(Controller_Model *Model);
//void diag_pressedHIDDEN(Controller_Model *Model);
void diag_pressedTOP(Controller_Model *Model);
void diag_pressedUP(Controller_Model *Model);
void diag_pressedLEFT(Controller_Model *Model);
void diag_pressedRIGHT(Controller_Model *Model);
void diag_pressedDOWN(Controller_Model *Model);
void diag_pressedBOT(Controller_Model *Model);
void diag_pressedNONE(Controller_Model *Model);


void diag_pre_Switch_case_Tasks(Controller_Model *Model);
void diag_post_Switch_case_Tasks(Controller_Model *Model);

void calibration(Controller_Model *Model);


void diag_page1(double r_zero, double r_span, double batt_min, double batt_max, double res_min, double res_max, double zero, double span);

void diag_page2(double r_zero);


void diag_set_temp_r_span(double temp_r_span);
void diag_set_temp_r_zero(double temp_r_zero);

#endif /* DIAGNOSTIC_MODE_H_ */