#include "display_driver.h"

#ifdef DISP_3000

#define RELEASE_DISPLAY 1.03

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016
// 1.03 - See comments at the end of this file
// 1.03_custom - adapted HZB Klaus Kiefer 2018
// 1.03_withoffset - adapted HZB Klaus Kiefer 2019

// (c) 2007 Speed-IT-up (Display3000), Peter und Stefan Küsters
// es ist gestattet, diese Routinen in eigene Programme einzubauen,
// wenn die folgenden 6 eingrahmten Zeilen im Kopf Ihres Sourcecodes stehen.
//
// ------------------------------------------------------------------------------------------------------------
// Display-Software-Grundlagen wurden von Peter Küsters, www.display3000.com ermittelt
// Dieser Display-Code ist urheberrechtlich geschützt. Sie erhalten eine Source-Code-Lizenz,
// d.h. Sie dürfen den Code in eigenen Programmen verwenden, diese aber nur in kompilierter
// Form weitergeben. Die Weitergabe dieses Codes in lesbarer Form oder die Publizierung
// im Internet etc. ist nicht gestattet und stellen einen Verstoß gegen das Urheberrecht dar.
// Weitere Displays, Platinen, Spezialstecker und fertige Module: www.display3000.com
// ------------------------------------------------------------------------------------------------------------
//
// Dieses Beispiel hier läuft auf dem Board D071, D072, D073 von Display 3000

#include <stdio.h>
#include <avr/io.h>
//#include <avr/wdt.h>
#include <util/delay.h>
#include <math.h>
#include <avr/pgmspace.h>
//#include <avr/eeprom.h>
#include <string.h>
#include <stdbool.h>



const unsigned int LCD_Width  = MAX_X;
const unsigned int LCD_Height = MAX_Y;


EWindowOrientation Orientation = Landscape180;
EWindowOrientation ili_Orientation = Portrait180;



//-------------------------------------------------------------------------------
// Initialize the display
//-------------------------------------------------------------------------------
void LCD_Init(void)
{
	DDRB = 255; //all Ports to output
	SPSR        |= _BV(SPI2X);
	SPCR        = _BV (SPE) | _BV(MSTR);// | _BV(CPOL) | _BV(CPHA);
	
	_delay_ms(300);
	RESET(LCD_RESET);	_delay_ms(75);			// Set PB6 to false and wait
	SET(LCD_SELECT);	_delay_ms(75);			// Set PB2 to true and wait
	RESET(LCD_CLK);		_delay_ms(75); 			// Set PB7 to false and wait
	SET(LCD_DC);		_delay_ms(75); 			// Set PB4 to true and wait
	SET(LCD_RESET);		_delay_ms(75); 			// Set PB6 to true and wait
	
	unsigned int init_data[] =
	{
		0xFDFD, 0xFDFD,
		// pause
		0xEF00, 0xEE04, 0x1B04, 0xFEFE, 0xFEFE,
		0xEF90, 0x4A04, 0x7F3F, 0xEE04, 0x4306,
		// pause
		0xEF90, 0x0983, 0x0800, 0x0BAF, 0x0A00,
		0x0500, 0x0600, 0x0700, 0xEF00, 0xEE0C,
		0xEF90, 0x0080, 0xEFB0, 0x4902, 0xEF00,
		0x7F01, 0xE181, 0xE202, 0xE276, 0xE183,
		0x8001, 0xEF90, 0x0000
	};
	LCD_SPI_DBCommand(&init_data[0], 2);	_delay_ms(75);
	LCD_SPI_DBCommand(&init_data[2], 10);	_delay_ms(75);
	LCD_SPI_DBCommand(&init_data[12], 23);
	RESET(LCD_SELECT);
}

//-------------------------------------------------------------------------------
//Set Output window
//-------------------------------------------------------------------------------
void LCD_Window(int x1, int y1, int x2, int y2)
{

	unsigned char data[] =
	{
		0xEF, 0x08,
		0x18, 0x00,
		0x12, x1,
		0x15, x2,
		0x13, y1,
		0x16, y2
	};
	switch (Orientation)
	{
		default:
		// Invalid! Fall through to portrait mode
		case Portrait:
		// do nothing, data is intitialized for Portrait mode
		break;
		case Portrait180:
		data[3] = 0x03;		// Mode
		data[5] = LCD_Width-1 - x1;
		data[7] = LCD_Width-1 - x2;
		data[9] = LCD_Height-1 - y1;
		data[11] = LCD_Height-1 - y2;
		break;
		case Landscape:
		data[3] = 0x05;		// Mode
		data[5] = LCD_Width-1 - y1;
		data[7] = LCD_Width-1 - y2;
		data[9] = x1;
		data[11] = x2;
		break;
		case Landscape180:
		data[3] = 0x06;		// Mode
		data[5] = y1;
		data[7] = y2;
		data[9] = LCD_Height-1 - x1;
		data[11] = LCD_Height-1 - x2;
		break;
	}
	SET(LCD_DC);														// switch to command mode as we send a command array, not a data array
	RESET(LCD_SELECT);             										// enable display
	LCD_SPI_Byte(data, 12); 											// send 12 command bytes
	RESET(LCD_DC);														// disable command mode = data mode on
}

//-------------------------------------------------------------------------------
// Send single bytes (or array with Bytes) by SPI
//-------------------------------------------------------------------------------
void LCD_SPI_Byte(const unsigned char data[], unsigned int count)
{
	
	//....data
	for (unsigned int i=0; i<count; ++i)	// Count Bytes
	{
		SPCR |= _BV(SPE);
		SPDR = data[i];
		LCD_Wait();
	}
}

//-------------------------------------------------------------------------------
// Send word (16 bit) (or array with words) by SPI
//-------------------------------------------------------------------------------
void LCD_SPI_DBCommand(const unsigned int data[], unsigned int count)
{
	for (int i=0; i<count; ++i)
	{
		SET(LCD_DC);						// Selects Command mode
		RESET(LCD_SELECT);					// Enable display
		unsigned char msbyte = (data[i] >> 8) & 0xff;		// msb!
		unsigned char lsbyte = data[i] & 0xff;			// lsb!
		SPCR |= _BV(SPE); 					//SPI: Ready
		SPDR = msbyte;						//send first 8 bits
		LCD_Wait();							//Wait until all bits has been sent
		SPCR |= _BV(SPE);
		SPDR = lsbyte;						//send last 8 bits
		LCD_Wait();
		RESET(LCD_DC);						// Selects Data mode
		SET(LCD_SELECT);					// Disable display
	}
}

//-------------------------------------------------------------------------------
// Send integer by SPI
//-------------------------------------------------------------------------------
void LCD_SPI_Int(unsigned int Value)
{	
	SPCR |= _BV(SPE);
	SPDR = (Value >> 8) & 0xff;
	LCD_Wait();
	SPCR |= _BV(SPE);
	SPDR = Value & 0xff;
	LCD_Wait();
}

//-------------------------------------------------------------------------------
// Wait for SPI transfer is done
//-------------------------------------------------------------------------------
void LCD_Wait(void)
{
	while (SPCR & _BV(SPE))
	{
		while (!(SPSR & (_BV(SPIF))));
		SPCR &= ~(_BV(SPE));
	}
}

#endif





#ifdef ili9341

/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <propaliidealist@gmail.com> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return. Johnny Sorocil
* ----------------------------------------------------------------------------
*/


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>	// abs()
#include <string.h>	// strlen()



#include "disp/ili9341cmd.h"
#include "disp/spi.c" //SPI-Utilities for communicating with Display
//#include "display.h" 

EWindowOrientation Orientation = Landscape180;
EWindowOrientation ili_Orientation = Portrait180;



//-------------------------------------------------------------------------------
//ili9341Display Specific
//-------------------------------------------------------------------------------
void glcd_cs_low() {
	/*
	DDRD |= 0b100000;	// PD5 is output
	PORTD &=~ 0b100000;	// PD5 is 0
	*/
	RESET(LCD_SELECT);
}
void glcd_cs_high() {
	/*
	DDRD |= 0b100000;
	PORTD |=  0b100000;
	*/
	SET(LCD_SELECT);
}
void glcd_dc_low() {
	/*
	DDRD |= 0b1000000;	// PD6
	PORTD &=~ 0b1000000;
	*/
	RESET(LCD_DC);
}
void glcd_dc_high() {
	/*
	DDRD |= 0b1000000;
	PORTD |=  0b1000000;
	*/
	SET(LCD_DC);
}
void glcd_led_off() {
	/*
	DDRD |= 0b10000000;	// PD7
	PORTD &=~ 0b10000000;
	*/
	RESET(3);
}
void glcd_led_on() {
	/*
	DDRD |= 0b10000000;
	PORTD |=  0b10000000;
	*/
	SET(3);
}
void glcd_rst_off() {
	/*
	DDRD |= 0b10000;	// PD4
	PORTD |=  0b10000;
	*/
	RESET(LCD_RESET);
}
void glcd_rst_on() {
	/*
	DDRD |= 0b10000;
	PORTD &=~ 0b10000;
	*/
	SET(LCD_RESET);
}

void glcd_sendCmd(unsigned char data)
{
	
	glcd_dc_low();
	glcd_cs_low();
	SPI_MasterTransmit(data);
	glcd_cs_high();
}


void glcd_sendData(unsigned char data)
{
	
	glcd_dc_high();
	glcd_cs_low();
	SPI_MasterTransmit(data);
	glcd_cs_high();
}

void glcd_setX(unsigned int x0,unsigned int x1)
{
	glcd_sendCmd(ILI9341_CMD_COLUMN_ADDRESS_SET);
	glcd_dc_high();
	glcd_cs_low();
	LCD_SPI_Int(x0);
	LCD_SPI_Int(x1);
	glcd_cs_high();

}

void glcd_setY(unsigned int y0,unsigned int y1)
{
	glcd_sendCmd(ILI9341_CMD_PAGE_ADDRESS_SET);
	glcd_dc_high();
	glcd_cs_low();
	LCD_SPI_Int(y0);
	LCD_SPI_Int(y1);
	glcd_cs_high();
}




#define ILI9341_PWCTR1     0xC0     ///< Power Control 1
#define ILI9341_PWCTR2     0xC1     ///< Power Control 2
#define ILI9341_VMCTR1     0xC5     ///< VCOM Control 1
#define ILI9341_VMCTR2     0xC7     ///< VCOM Control 2
#define ILI9341_MADCTL     0x36     ///< Memory Access Control
#define ILI9341_VSCRSADD   0x37     ///< Vertical Scrolling Start Address
#define ILI9341_RDPIXFMT   0x0C     ///< Read Display Pixel Format
#define ILI9341_FRMCTR1    0xB1     ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_DFUNCTR    0xB6     ///< Display Function Control
#define ILI9341_GAMMASET   0x26     ///< Gamma Set
#define ILI9341_PIXFMT     0x3A     ///< COLMOD: Pixel Format Set
#define ILI9341_GMCTRP1    0xE0     ///< Positive Gamma Correction
#define ILI9341_GMCTRN1    0xE1     ///< Negative Gamma Correction
#define ILI9341_SLPOUT     0x11     ///< Sleep Out
#define ILI9341_DISPON     0x29     ///< Display ON



//-------------------------------------------------------------------------------
//DRIVER INTERFACE TO DISPLAY.H
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Initialize the display
//-------------------------------------------------------------------------------
void LCD_Init(void){
	DDRB = 255; //all Ports to output
	SPI_MasterInit();

	glcd_cs_high();
	glcd_dc_high();

	glcd_rst_off();
	_delay_ms(10);
	
	
	glcd_rst_on();
	_delay_ms(120);
	
	//glcd_setOrientation(PORTRAIT);	// default
	
	glcd_sendCmd(ILI9341_CMD_POWER_ON_SEQ_CONTROL);
	glcd_sendData(ILI9341_CMD_IDLE_MODE_ON);
	glcd_sendData(ILI9341_CMD_MEMORY_WRITE);
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(ILI9341_CMD_TEARING_EFFECT_LINE_OFF);
	glcd_sendData(0x02); 	// XXX

	glcd_sendCmd(ILI9341_CMD_POWER_CONTROL_B);
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(ILI9341_CMD_POWER_CONTROL_2);
	glcd_sendData(ILI9341_CMD_PARTIAL_AREA);

	glcd_sendCmd(ILI9341_CMD_DRIVER_TIMING_CONTROL_A);
	glcd_sendData(0x85); 	// XXX
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(0x78); 	// XXX

	glcd_sendCmd(ILI9341_CMD_DRIVER_TIMING_CONTROL_B);
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(ILI9341_CMD_NOP);

	glcd_sendCmd(0xED);	// XXX
	glcd_sendData(0x64); 	// XXX
	glcd_sendData(0x03);	// XXX
	glcd_sendData(ILI9341_CMD_PARTIAL_MODE_ON);
	glcd_sendData(0X81); 	// XXX

	glcd_sendCmd(ILI9341_CMD_PUMP_RATIO_CONTROL);
	glcd_sendData(ILI9341_CMD_DISP_INVERSION_OFF);

	glcd_sendCmd(ILI9341_CMD_POWER_CONTROL_1);
	glcd_sendData(0x23);	//VRH[5:0] 	// XXX

	glcd_sendCmd(ILI9341_CMD_POWER_CONTROL_2);
	glcd_sendData(ILI9341_CMD_ENTER_SLEEP_MODE);

	glcd_sendCmd(ILI9341_CMD_VCOM_CONTROL_1);
	glcd_sendData(ILI9341_CMD_READ_MEMORY_CONTINUE);
	glcd_sendData(ILI9341_CMD_DISPLAY_OFF);

	glcd_sendCmd(ILI9341_CMD_VCOM_CONTROL_2);
	glcd_sendData(0x86);	//--	// XXX

	glcd_sendCmd(ILI9341_CMD_MEMORY_ACCESS_CONTROL);
	glcd_sendData(0x48);	//C8	//48 68gal.gal.gal.//28 E8 gal.gal.gal.	// XXX

	glcd_sendCmd(ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET);
	glcd_sendData(ILI9341_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS);

	glcd_sendCmd(ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL);
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(0x18); 	// XXX

	glcd_sendCmd(ILI9341_CMD_DISPLAY_FUNCTION_CONTROL);
	glcd_sendData(0x08); 	// XXX
	glcd_sendData(0x82);	// XXX
	glcd_sendData(0x27);	// XXX

	glcd_sendCmd(ILI9341_CMD_ENABLE_3_GAMMA_CONTROL);
	glcd_sendData(ILI9341_CMD_NOP);

	glcd_sendCmd(0x26);	//Gamma curve selected 	// XXX
	glcd_sendData(ILI9341_CMD_SOFTWARE_RESET);

	glcd_sendCmd(ILI9341_CMD_POSITIVE_GAMMA_CORRECTION);
	glcd_sendData(0x0F); 	// XXX
	glcd_sendData(0x31);	// XXX
	glcd_sendData(ILI9341_CMD_PAGE_ADDRESS_SET);
	glcd_sendData(ILI9341_CMD_READ_DISP_PIXEL_FORMAT);
	glcd_sendData(ILI9341_CMD_READ_DISP_SIGNAL_MODE);
	glcd_sendData(0x08); 	// XXX
	glcd_sendData(0x4E); 	// XXX
	glcd_sendData(0xF1); 	// XXX
	glcd_sendData(ILI9341_CMD_VERT_SCROLL_START_ADDRESS);
	glcd_sendData(0x07); 	// XXX
	glcd_sendData(ILI9341_CMD_ENTER_SLEEP_MODE);
	glcd_sendData(0x03);	// XXX
	glcd_sendData(ILI9341_CMD_READ_DISP_SIGNAL_MODE);
	glcd_sendData(ILI9341_CMD_READ_DISP_STATUS);
	glcd_sendData(ILI9341_CMD_NOP);

	glcd_sendCmd(ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION);
	glcd_sendData(ILI9341_CMD_NOP);
	glcd_sendData(ILI9341_CMD_READ_DISP_SIGNAL_MODE);
	glcd_sendData(0x14); 	// XXX
	glcd_sendData(0x03);	// XXX
	glcd_sendData(ILI9341_CMD_SLEEP_OUT);
	glcd_sendData(0x07); 	// XXX
	glcd_sendData(0x31); 	// XXX
	glcd_sendData(ILI9341_CMD_POWER_CONTROL_2);
	glcd_sendData(0x48); 	// XXX
	glcd_sendData(0x08); 	// XXX
	glcd_sendData(0x0F); 	// XXX
	glcd_sendData(ILI9341_CMD_READ_DISP_PIXEL_FORMAT);
	glcd_sendData(0x31); 	// XXX
	glcd_sendData(ILI9341_CMD_MEMORY_ACCESS_CONTROL);
	glcd_sendData(ILI9341_CMD_READ_DISP_SELF_DIAGNOSTIC);

	glcd_sendCmd(ILI9341_CMD_SLEEP_OUT);
	_delay_ms(120);

	glcd_sendCmd(ILI9341_CMD_DISPLAY_ON);
	glcd_sendCmd(ILI9341_CMD_MEMORY_WRITE);
	
	//LCD_Cls(black);
	//_delay_ms(150);
}

//-------------------------------------------------------------------------------
//Set Output window
//-------------------------------------------------------------------------------
void LCD_Window(int x1, int y1, int x2, int y2){
	char m  = 0;
	
	//New Orientation Data is only sent to Display if Orientation has changed since last call of LCD_Window()
	if (Orientation != ili_Orientation){
		switch (Orientation)
		{
			case Portrait:
			if (Orientation != ili_Orientation){
				ili_Orientation = Orientation;
				m  = (MADCTL_MX | MADCTL_BGR);
			}
			break;
			case Portrait180:
			if (Orientation != ili_Orientation){
				ili_Orientation = Orientation;
				m = (MADCTL_MY | MADCTL_BGR);
			}
			break;
			case Landscape:
			if (Orientation != ili_Orientation){
				ili_Orientation = Orientation;
				m  = (MADCTL_MV | MADCTL_BGR);
			}
			
			
			break;
			case Landscape180:
			if (Orientation != ili_Orientation){
				ili_Orientation = Orientation;
				m  = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			}

			break;
		}
		
		// Setting ram line and collumn adress write order for the specified Orientation of the Display
		// so that Pixel(0,0) is always at the top-left corner 
		glcd_sendCmd(ILI9341_CMD_MEMORY_ACCESS_CONTROL);
		glcd_sendData(m);		
	}
	
	glcd_setX(x1, x2);
	glcd_setY(y1, y2);
	glcd_sendCmd(ILI9341_CMD_MEMORY_WRITE);
	glcd_dc_high();
	glcd_cs_low();
}


// Sends 16-bit word to Display , mostly used to send color Data
void LCD_SPI_Int(unsigned int Value)
{
	unsigned char data1 = Value>>8;
	unsigned char data2 = Value&0xff;
	//glcd_dc_high();
	//glcd_cs_low();
	SPI_MasterTransmit(data1);
	SPI_MasterTransmit(data2);
	//glcd_cs_high();
}


#endif