// Display.h - Copyright 2016, HZB, ILL/SANE & ISIS

// Add the following commands:
//
// => ...

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
// 
// 
// Dieses Beispiel hier läuft auf dem Board D071, D072, D073 von Display 3000

//#include <stdbool.h>


#ifndef DISPLAY_H
#define DISPLAY_H

typedef enum
{
	Portrait		= 0,	// Do not change these values!
	Portrait180		= 3,	// They are sent to the display
	Landscape		= 5,	// in order to set the graphics
	Landscape180	= 6		// mode of the display.
} EWindowOrientation;

typedef enum
{
	cm256 = 0,
	cm65k = 1
} EColorMode;

/*typedef enum
{
	cm65k_uncompressed		= 0,
	cm65k_compressed		= 1,
	cm256low_uncompressed	= 2,
	cm256low_compressed		= 3,
	cm256high_uncompressed	= 4,
	cm256high_compressed	= 5
} ECompressionMode;*/

extern EWindowOrientation Orientation;

extern unsigned int WindowWidth(void);
extern unsigned int WindowHeight(void);

void LCD_Init(void);
void LCD_Print(const char* Text, unsigned char X, unsigned char Y, unsigned char FontNr,
unsigned char XScale, unsigned char YScale, unsigned int ForeColor, unsigned int BackColor);
void LCD_Window(int x1, int y1, int x2, int y2);
void LCD_Cls(unsigned int color);
void LCD_Plot(unsigned char x1, unsigned char y1, unsigned char line_type, unsigned int color);
void LCD_Box(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned int color);
void LCD_Rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char line_type, unsigned int color);
void LCD_Draw(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2,unsigned char line_type ,unsigned int color);
void LCD_CirclePoints(int x, int y, int xOfs, int yOfs, unsigned char line_type, unsigned int color);
void LCD_Circle(int x1, int y1, int r, unsigned char fill, unsigned char line_type, unsigned int color);
_Bool LCD_Dialog(char *title, char *text, unsigned int BackColor, unsigned int ForeColor);

void LCD_SPI_Byte(const unsigned char data[], unsigned int count);
void LCD_SPI_DBCommand(const unsigned int data[], unsigned int count);
void LCD_SPI_Int(unsigned int Value);
void LCD_Wait(void);

/////////////////////////////
// The following functions have been deactivated because not used
/////////////////////////////

//void LCD_Bitmap_65k(int x1, int y1, int x2, int y2, const prog_uint16_t Pbmp[], _Bool Compressed);
//void LCD_Bitmap_256low(int x1, int y1, int x2, int y2, const prog_uint8_t Pbmp[], _Bool Compressed);
//void LCD_Bitmap_256high(int x1, int y1, int x2, int y2, const prog_uint8_t Pbmp[], const prog_uint16_t ColorTable[], _Bool Compressed);

#define GLUE(a, b)     a##b
#define PORT(x)        GLUE(PORT, x)
#define PIN(x)         GLUE(PIN, x)
#define DDR(x)         GLUE(DDR, x)

#define LCD_PORT 		PortB  	
#define LCD_CLK 		7		// Clock 
#define LCD_RESET 		6		// Reset Display 
#define LCD_SELECT 		2		// Cable Select 
#define LCD_DC 			4		// Data/Command 
#define LCD_DATA 		5		// Send Data 

#define RGB(R,G,B) ((((R) & 0x1f) << 11) + (((G) & 0x3f) << 5) + ((B) & 0x1f))
#define SET(bit) (PORTB |= _BV(bit))
#define RESET(bit) (PORTB &= ~_BV(bit))

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

#define THICK   1
#define THIN    0

#define FILL    1
#define NOFILL  0

#endif  // Display.h
