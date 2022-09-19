#ifndef HONEYWELLSSC_H_
#define HONEYWELLSSC_H_


//#define F_CPU 6144000


#include <util/delay.h>
// #include "commutils.h"

// I²C address of the Honeywell Pressure Sensor SSCSRNN1.6BA7A3
#define HoneywellSSC_address	 0x78   // this version of the sensor has address 0x78

typedef struct {
	unsigned int connected;		//1: connected 0: not connected
	unsigned int status;        //0: normal operation, 1: device in command mode, 2: stale data (already read), 3: diagnostic condition
} stats4;

extern volatile stats4 HoneywellSSC_status;

extern volatile double HoneywellSSC_Temperature;
extern volatile double HoneywellSSC_Pressure;

extern volatile uint8_t HoneywellSSC_temp_MSB;
extern volatile uint8_t HoneywellSSC_temp_LSB;
extern volatile uint8_t HoneywellSSC_press_MSB;
extern volatile uint8_t HoneywellSSC_press_LSB;


uint8_t init_HoneywellSSC(void);
uint8_t HoneywellSSC_read_pressure(void);

#endif 