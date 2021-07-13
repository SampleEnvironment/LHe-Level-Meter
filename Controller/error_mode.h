/*
 * error_mode.h
 *
 * Created: 01.03.2021 13:56:40
 *  Author: Weges
 */ 


#ifndef ERROR_MODE_H_
#define ERROR_MODE_H_
//=====================================================================
// ERROR MODE
//=====================================================================



typedef struct
{
	Controller_Model super;
	
}Controller_Model_error;




extern globalModesType global_mode;

Controller_Model_error* get_error_model(void);


void error_pre_Switch_case_Tasks(Controller_Model *Model);






#endif /* ERROR_MODE_H_ */