/*
* getCode_mode.h
*
* Created: 05.03.2021 14:41:48
*  Author: Weges
*/


#ifndef GETCODE_MODE_H_
#define GETCODE_MODE_H_


//=====================================================================
// GET-CODE MODE
//=====================================================================



typedef struct
{
	Controller_Model super;
	char  Code_Str[DEVICE_ID_STRING_LEN+1];
	uint8_t digits[DEVICE_ID_STRING_LEN];
	uint16_t  xCoords[DEVICE_ID_STRING_LEN];
	int8_t active_digit;
	int8_t active_last;
	uint16_t Space_Factor;
	uint8_t FontNR;
	uint8_t FontW;
	uint8_t FontH;
	uint8_t Code_len;
	_Bool   alphanum;
	uint16_t digit_xOffset;
	uint8_t * status;
	_Bool exit_loop;
	uint8_t lowest_char;
	
}Controller_Model_getCode;


extern globalModesType global_mode;



//void getCode_pressedFILL(Controller_Model *Model);
//void getCode_pressedMeasure(Controller_Model *Model);
void getCode_pressedHIDDEN(Controller_Model *Model);
void getCode_pressedTOP(Controller_Model *Model);
void getCode_pressedUP(Controller_Model *Model);
void getCode_pressedLEFT(Controller_Model *Model);
void getCode_pressedRIGHT(Controller_Model *Model);
void getCode_pressedDOWN(Controller_Model *Model);
void getCode_pressedBOT(Controller_Model *Model);
void getCode_pressedBOT_PW(Controller_Model *Model);
//void getCode_pressedNONE(Controller_Model *Model);


void getDEV_ID_pre_Switch_case_Tasks(Controller_Model *Model);
void get_Pw_pre_Switch_case_Tasks(Controller_Model *Model);

void get_Device_ID(uint8_t *status);
uint16_t get_Password(uint8_t *status);
void paint_getCode_digits(void);
void update_Dev_ID_digits(void);

uint8_t validate_Dev_ID(void);
char* itoc_dev(char * letterstr,uint8_t letter_num);

#endif /* GETCODE_MODE_H_ */