/*
 * filling_mode.h
 *
 * Created: 01.03.2021 09:18:57
 *  Author: Weges
 */ 


#ifndef FILLING_MODE_H_
#define FILLING_MODE_H_

//=====================================================================
// FILLING MODE
//=====================================================================



typedef struct
{
	Controller_Model super;
	uint8_t meas_progress;
	uint32_t t_last_presss_meas;
}Controller_Model_filling;


extern globalModesType global_mode;

Controller_Model_filling * get_fill_model(void);

//void filling_pressedFILL(Controller_Model *Model);
void filling_pressedMeasure(Controller_Model *Model);
//void filling_pressedHIDDEN(Controller_Model *Model);
void filling_pressedTOP(Controller_Model *Model);
//void filling_pressedUP(Controller_Model *Model);
//void filling_pressedLEFT(Controller_Model *Model);
//void filling_pressedRIGHT(Controller_Model *Model);
//void filling_pressedDOWN(Controller_Model *Model);
void filling_pressedBOT(Controller_Model *Model);
void filling_pressedNONE(Controller_Model *Model);


void filling_post_Switch_case_Tasks(Controller_Model *Model);
void paint_filling_status(void);


#endif /* FILLING_MODE_H_ */