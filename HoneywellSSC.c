#include "HoneywellSSC.h"
#include "avr-util-library/i2cmaster.h"

 volatile stats4 HoneywellSSC_status = {
	.connected = 0,  // not connected
	.status = 2     // old data
};

volatile double HoneywellSSC_Temperature;
volatile double HoneywellSSC_Pressure;

volatile uint8_t HoneywellSSC_press_MSB;
volatile uint8_t HoneywellSSC_press_LSB;
volatile uint8_t HoneywellSSC_temp_MSB;
volatile uint8_t HoneywellSSC_temp_LSB;


/*
*	Function: init_HoneywellSSC
*
*	Description: Initialization of the external device Honeywell SSCSRNN1.6BA7A3 , a pressure sensor unit. It's connected via I²C (Two Wire Interface).
*   Sensor information:
*   max. pressure 1.6 bar (absolut)
*   min pressure 0 bar (absolut)
*   max output 0.9 * 2^14 = 14745
*   min output 0.1 * 2^14 = 1638
*
*   
*	Input: /
*
*	Output:	0:	successful
*			1:	error while initialization
*/

uint8_t init_HoneywellSSC(void)
{
   HoneywellSSC_status.status = HoneywellSSC_read_pressure();
   HoneywellSSC_status.connected = !(HoneywellSSC_status.status	== 4); // status 4 means no data was received
   switch (HoneywellSSC_status.status)
   {
	   case 0: return 0; // normal operation
	   case 2: return 0; // old data
	   case 3: return 1; // sense element problem  
	   default: return 1;
   }
 return 0;
}

/*
*	Function: HoneywellSSC_read_pressure
*
*	Description: reads the pressure and the temperature of device.
*
*	Input: /
*
*	Output: 0: normal operation
*           1: device in command mode
*			2: stale data (data was already read but is o.k.) 
*			3: diagnostic condition 
*           else: error
*/

uint8_t HoneywellSSC_read_pressure(void)
{
	uint16_t pressure_output = 0;
	uint16_t temperature_output = 0;
	uint8_t  status_output = 4;

	HoneywellSSC_Temperature = 0;
	HoneywellSSC_Pressure = 0;
	
	HoneywellSSC_press_MSB = 0;
	HoneywellSSC_press_LSB = 0;
	HoneywellSSC_temp_MSB = 0;
	HoneywellSSC_temp_LSB = 0;
	
	status_output = i2c_start((HoneywellSSC_address << 1) +I2C_READ);
    if (status_output==0)
	{   
		HoneywellSSC_press_MSB = i2c_readAck();
		HoneywellSSC_press_LSB = i2c_readAck();
		HoneywellSSC_temp_MSB = i2c_readAck();
		HoneywellSSC_temp_LSB = i2c_readNak();
		i2c_stop();
	}
	else
	{
		i2c_stop();
		return 4;
	}

	
	if (!HoneywellSSC_press_MSB)
	{
		if (!HoneywellSSC_press_LSB)
		{
			if (!HoneywellSSC_temp_MSB)
			{
				if (!HoneywellSSC_temp_LSB)
				{
				   return 4;	
				}			
			}
		}
	}

	status_output = HoneywellSSC_press_MSB >> 6;  // 2 highest bits
	HoneywellSSC_press_MSB = HoneywellSSC_press_MSB & 63;  // get rid of the highest 2 bits
	
	pressure_output = (((uint16_t) HoneywellSSC_press_MSB) << 8) + HoneywellSSC_press_LSB;
	temperature_output = (((uint16_t) HoneywellSSC_temp_MSB) << 8) + HoneywellSSC_temp_LSB; 
	
	HoneywellSSC_Pressure = (double)(pressure_output - 1638) * 1600 / (14745-1638);
	HoneywellSSC_Temperature = (double) (temperature_output*200/2047)-50 +273; // in Kelvin    

	return status_output;
}
