/*
* option_mode.h
*
* Created: 01.03.2021 14:10:35
*  Author: Weges
*/


#ifndef OPTION_MODE_H_
#define OPTION_MODE_H_

//=====================================================================
// OPTION MODE
//=====================================================================
#include <stdint.h>
#include "base_controller.h"

extern globalModesType global_mode;
extern VarsType vars;

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
	_Bool	display_reversed_buff ;
	double 	batt_min_buff ;
	double 	batt_max_buff;
	uint8_t critical_batt_buff ;
	_Bool   options_changed;
}Controller_Model_options;




typedef struct {
	char Page[5][20];
}optPageStrType;


Controller_Model_options* get_option_model(void);

void set_OptionModel(uint8_t page, uint8_t option, uint8_t value);

void set_options_changed(void);

void set_bufferVars(_Bool   options_changed);

//void main_pressedFILL(Controller_Model *Model);
//void main_pressedMeasure(Controller_Model *Model);
//void main_pressedHIDDEN(Controller_Model *Model);
void option_pressedTOP(Controller_Model *Model);
void option_pressedUP(Controller_Model *Model);
void option_pressedLEFT(Controller_Model *Model);
void option_pressedRIGHT(Controller_Model *Model);
void option_pressedDOWN(Controller_Model *Model);
void option_pressedBOT(Controller_Model *Model);
//void main_pressedNONE(Controller_Model *Model);


void option_pre_Switch_case_Tasks(Controller_Model *Model);
void option_post_Switch_case_Tasks(Controller_Model *Model);



//===================================================================================
//Base Vtable
//========================================================
struct ValueOptVtable;

typedef struct ValueOptions
{
	struct ValueOptVtable const *vtable;
	uint16_t lineYcoord;
	const char *OptName;
}ValueOptions;

typedef void (*valueDraw_type)(ValueOptions*, unsigned int color);
typedef void (*valueChange_type)(ValueOptions*,int key);

struct ValueOptVtable
{
	valueDraw_type pfvalueDraw;
	valueChange_type pfvalueChange;
	
};



void Option_valueDraw(ValueOptions*optEntry,unsigned int color);
void Option_valueChange(ValueOptions*optEntry,int key);
void Option_StrDraw(ValueOptions*optEntry,unsigned int color);

//===================================================================================
//Boolean
//========================================================
typedef struct
{
	ValueOptions super;
	_Bool *bValue;
	_Bool isUploaded;
} ValueOptionsBool;

void Bool_valueDraw ( ValueOptions *optEntry, unsigned int color);
void Bool_valueChange (ValueOptions *optEntry, int key);
void Bool_valueChange_Shutdown(ValueOptions *optEntry, int key);
void Bool_valueChange_diagmode( ValueOptions *optEntry, int key);
void ValOptBool_init( ValueOptionsBool *optEntry, struct ValueOptVtable *bool_vtable, uint16_t lineYcoord,char *OptName, _Bool *bValue, _Bool isUploaded);

//===================================================================================
//Double
//========================================================
typedef struct
{
	ValueOptions super;
	double *dValue;
	double increment;
	double continuousInc;
	uint8_t precision;
	double MAX;
	double MIN;
	char *unit;
	_Bool isUploaded;
}ValueOptionsDouble;
void Double_valueDraw (ValueOptions *optEntry, unsigned int color);
void Double_valueChange (ValueOptions *optEntry, int key);
void ValOptDouble_init(ValueOptionsDouble *optEntry, uint16_t lineYcoord,char *OptName, double *dValue, double increment, double continuousInc,double MAX,double MIN, uint8_t pecision, char* unit,_Bool isUploaded);

//===================================================================================
//uint16_t
//========================================================
typedef struct
{
	ValueOptions super;
	uint16_t *uint16_Value;
	uint8_t increment;
	uint16_t MAX;
	uint16_t MIN;
	_Bool *unit_selector;
	_Bool *val_changed;
	char *unit_1;
	char *unit_2;
	
}ValueOptionsuInt16;

void uInt16_valueDraw (ValueOptions *optEntry, unsigned int color);
void uInt16_valueChange (ValueOptions *optEntry, int key);
void ValOptInt16_init(ValueOptionsuInt16 *optEntry, uint16_t lineYcoord,char *OptName,uint16_t *uint16_value, uint8_t increment,uint16_t MAX, uint16_t MIN,_Bool *unit_selector,_Bool *val_changed, char *unit_1, char *unit_2);


//===================================================================================
//uint8_t
//========================================================
typedef struct
{
	ValueOptions super;
	uint8_t *uint8_Value;
	uint8_t increment;
	uint8_t MAX;
	uint8_t MIN;
	char *unit;
}ValueOptionsuInt8;

void uInt8_valueDraw (ValueOptions *optEntry, unsigned int color);
void uInt8_valueChange (ValueOptions *optEntry, int key);
void ValOptInt8_init(ValueOptionsuInt8 *optEntry, uint16_t lineYcoord,char *OptName, uint8_t *uint8_Value, uint8_t increment,uint8_t MAX, uint8_t MIN, char *unit);

//===================================================================================
//Char*
//========================================================
typedef struct
{
	ValueOptions super;
	char**  string_arr;
	uint8_t *arr_ind;
	uint8_t MAX;
	uint8_t MIN;
}ValueOptionsChar;

void Char_valueDraw (ValueOptions *optEntry, unsigned int color);
void Char_valueChange (ValueOptions *optEntry, int key);
void ValOptChar_init(ValueOptionsChar *optEntry, struct ValueOptVtable *char_vtable, uint16_t lineYcoord,char *OptName, char **string_arr,uint8_t* arr_ind,	uint8_t MIN, uint8_t MAX);

//===================================================================================
//NULL OPT
//========================================================
typedef struct
{
	ValueOptions super;
	char *  optval;
}ValueOptionsNull;

void null_valueDraw(ValueOptions *optEntry, unsigned int color);
void null_vlueChange (ValueOptions *optEntry, int key);
void ValOptNull_init(ValueOptionsNull *optEntry, uint16_t lineYcoord,char *OptName, char * optval);

void make_he_vol_changable(void);

void  OptentrysInit(void);

void opt_drawPageChange(void);

void opt_ValOpt_switch(void);

void opt_optionChange(void);

void opt_drawPage(void);

void opt_ValueChange(void);

void option_exit(Controller_Model * Model, uint8_t headless);




#endif /* OPTION_MODE_H_ */ 