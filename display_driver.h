// common Functions needed in both drivers:
// LCD_Init() Display Initialisation
// LCD_Window() Sends Command bytes to Display about which Pixel or Pixels are to be Painted
// LCD_SPI_Int() Sends A 16-Bit encoded RGB Value to the Display

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H


#include "config.h"

// COLORS
#define bright_blue		0b1101111011011111	//Predefined colors will make programmers life easier
#define blue			0b0000000000011111
#define dark_blue		0b0000000000010011
#define bright_yellow	0b1111111111001100	//as the display uses 65.536 colors we have to define double-bytes for each color
#define yellow			0b1111111111100000	//Check the programmers manual to learn how to define your own color
#define orange			0b1111110011000110
#define bright_red		0b1111100011100011
#define red				0b1111100000000000
#define dark_red		0b1001100000000000
#define bright_green 	0b1001111111110011
#define green 			0b0000011111100000
#define dark_green 		0b0000001101100000
#define white 			0b1111111111111111
#define grey 			0b0011100011100111
#define black 			0b0000000000000000

#define HZB_Blue        0b0000001011010011
#define HZB_Cyan        0b0000010011111100


#define LCD_PORT 		PortB
#define LCD_CLK 		7		// Clock
#define LCD_RESET 		6		// Reset Display
#define LCD_SELECT 		2		// Cable Select
#define LCD_DC 			4		// Data/Command
#define LCD_DATA 		5		// Send Data

#define SET(bit) (PORTB |= _BV(bit))
#define RESET(bit) (PORTB &= ~_BV(bit))


typedef enum
{
	Portrait		= 0,	// Do not change these values!
	Portrait180		= 3,	// They are sent to the display
	Landscape		= 5,	// in order to set the graphics
	Landscape180	= 6		// mode of the display.
} EWindowOrientation;






//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

#ifdef DISP_3000

//Display Dimensions
#define MAX_X   132
#define MAX_Y   176

unsigned int max_x, max_y;




typedef enum
{
	cm256 = 0,
	cm65k = 1
} EColorMode;


typedef enum
{
	cm65k_uncompressed		= 0,
	cm65k_compressed		= 1,
	cm256low_uncompressed	= 2,
	cm256low_compressed		= 3,
	cm256high_uncompressed	= 4,
	cm256high_compressed	= 5
} ECompressionMode;



//Common Functions (Used as Interface for higher level Functions)
void LCD_Init(void);
void LCD_Window(int x1, int y1, int x2, int y2);
void LCD_SPI_Int(unsigned int Value);

//Display Sepecific Functions
void LCD_SPI_Byte(const unsigned char data[], unsigned int count);
void LCD_SPI_DBCommand(const unsigned int data[], unsigned int count);

void LCD_Wait(void);

#define GLUE(a, b)     a##b
#define PORT(x)        GLUE(PORT, x)
#define PIN(x)         GLUE(PIN, x)
#define DDR(x)         GLUE(DDR, x)

#define RGB(R,G,B) ((((R) & 0x1f) << 11) + (((G) & 0x3f) << 5) + ((B) & 0x1f))

#define THICK   1
#define THIN    0

#define FILL    1
#define NOFILL  0

#endif



#ifdef ili9341
/*
GLCD	AT8    AT32
VCC
GND
CS	PD5 11 19
RST	PD4 6  18
DC	PD6 12 20
MOSI	PB3 17  6
SCK	PB5 19  8
LED	PD7 13 21
MISO	PB4 18  7	// not really needed
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define MAX_X   239
#define MAX_Y   319

#define FONT_SPACE	6
#define FONT_X		8
#define FONT_Y		8


#define MADCTL_MY  0x80  ///< Bottom to top
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04  ///< LCD refresh right to left


extern EWindowOrientation ili_Orientation;
//#define PORTRAIT	0
//#define LANDSCAPE	1

void glcd_cs_low(void);
void glcd_cs_high(void); 
void glcd_dc_low(void);
void glcd_dc_high(void); 
void glcd_led_off(void);
void glcd_led_on(void);
void glcd_rst_off(void);
void glcd_rst_on(void);

//const unsigned char simpleFont[][8];
//unsigned char glcd_orientation;


//Display Specific
void glcd_sendCmd(unsigned char data);
void glcd_sendData(unsigned char data);
void glcd_setX(unsigned int x0,unsigned int x1);
void glcd_setY(unsigned int y0,unsigned int y1);
//void glcd_setXY(unsigned int x0, unsigned int y0);
//void LCD_sendComArgs(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);

//Common Functions
void LCD_Init(void);
void LCD_Window(int x1, int y1, int x2, int y2);
void LCD_SPI_Int(unsigned int Value);



#endif

#endif  // display_driver.h
