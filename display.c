// Display.c - Copyright 2016, HZB, ILL/SANE & ISIS
#define RELEASE_DISPLAY 1.03

// HISTORY -------------------------------------------------------------
// 1.00 - First release on March 2016
// 1.03 - See comments at the end of this file

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

#include "keyboard.h"
#include "display.h"

const unsigned int LCD_Width = 132;
const unsigned int LCD_Height = 176;

unsigned int WindowWidth(void)
{
	if (Orientation==Portrait || Orientation==Portrait180)
		return LCD_Width;
	else
		return LCD_Height;
}

unsigned int WindowHeight(void)
{
	if (Orientation==Portrait || Orientation==Portrait180)
		return LCD_Height;
	else
		return LCD_Width;
}

EWindowOrientation Orientation = Landscape;

extern const uint8_t Font1[], Font2[] PROGMEM;	// Shared font arrays stored in EEPROM
//extern void delay_ms(uint16_t period);			// Delay routine (in milliseconds)

//const char  *Bitmap;
//const uint16_t *Bitmap PROGMEM;
//const uint16_t *Bitmap PROGMEM;
//const prog_uint16_t  *Bitmap;
//const uint16_t  *Bitmap;

// ##### LCD subroutines below  ######################################################################################

typedef struct
{
	unsigned char CharWidth;	// How many pixel wide (real pixel data, not including any empty pixel for frame)
	unsigned char CharHeight;	// How many pixel height (real pixel data, not including any empty pixel for frame)
	unsigned char CellWidth;	// How many pixel wide including empty pixels, if you like to get more space between the characters rise this num
	unsigned char CellHeight;	// How many pixel height including empty pixels, if you like to get more space between the lines rise this number
} TFontInfo;

TFontInfo FontInfo[2] =
{
	{ 5, 8, 6, 9},		// normal font
	{ 8, 14, 9, 15 }	// large font
};

// Initialise the display
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
// Displays a string
//-------------------------------------------------------------------------------
void LCD_Print(const char* Text, unsigned char X, unsigned char Y, unsigned char FontNr,
				unsigned char XScale, unsigned char YScale, unsigned int ForeColor, unsigned int BackColor)
{

	if (--FontNr > 1) FontNr = 1;
	const uint8_t* Font PROGMEM = (FontNr==0) ? Font1 : Font2;
//	const prog_uint8_t* Font = (FontNr==0) ? Font1 : Font2;
//	const uint8_t* Font = (FontNr==0) ? Font1 : Font2;
	const unsigned int Len = strlen(Text);
	const unsigned int CharWidth = FontInfo[FontNr].CharWidth;
	const unsigned int CharHeight = FontInfo[FontNr].CharHeight;
	const unsigned int CellWidth = FontInfo[FontNr].CellWidth;
	const unsigned int CellHeight = FontInfo[FontNr].CellHeight;

	for (unsigned int Index=0; Index<Len; ++Index)	// For each character in the string
	{
		unsigned char Ch = Text[Index];				// Ch is the position of character data in the font table
		if (Ch > 122)								// At the end of the font data some german special characters are defined
		{
			switch (Ch)								// special treatment eliminates the storage of unused data
			{
				case 228: Ch = 127; break;			// ä is at Pos. 127 etc.
				case 246: Ch = 128; break;			// ö
				case 252: Ch = 129; break;			// ü
				case 196: Ch = 130; break;			// Ä
				case 214: Ch = 131; break;			// Ö
				case 220: Ch = 132; break;			// Ü
				case 223: Ch = 133; break;			// ß
				default: Ch = '?'; break;				// not allowed: change to ?
			}
        }
		Ch -= 32;													// Substracts 32 as we are not using any character below ASCII 32, we are starting with the character space (ASCII 32)at the font table
		const unsigned int BytePos = Ch * CharHeight;				// Multiply with the data size of this font (our two fonts use either 5 or 8 data/pixel per line)

		LCD_Window(X, Y, X+CellWidth*XScale-1, Y+CellHeight*YScale+1);// Set character window

		for (unsigned int Loop=0; Loop<=CellHeight; ++Loop)			// For each vertical row of this character
		{
			uint8_t Byte PROGMEM = pgm_read_byte(&Font[BytePos+Loop]);
//			prog_uint8_t Byte = pgm_read_byte (&Font[BytePos+Loop]);	// fetch byte value of the character's row
//			uint8_t Byte = pgm_read_byte (&Font[BytePos+Loop]);	// fetch byte value of the character's row
			if (Loop >= CharHeight) Byte = 0;							// the lines with background color at the lower end of the character
			for (unsigned int Pixel=1; Pixel<=YScale; ++Pixel)		// As often as needed by the Y-scale factor (e.g. draws complete line twice if double height font is requested)
			{
				const int XPixelTmp = (CellWidth-CharWidth)*XScale;	// how many empty columns are requested for this font
				for (int Charbit=1; Charbit<=XPixelTmp; ++Charbit)
				{
					// empty column = always background color
					LCD_SPI_Int(BackColor);								// draw pixel
				}

				for (int Charbit=CharWidth-1; Charbit>=0; --Charbit)	// check each bit of this character line
				{
					const int bShowPixel = (Byte >> Charbit) & 0x01;	// check if pixel is on or off (select appropriate bit)
					const unsigned int Color = bShowPixel ?
						ForeColor :										// pixel = on: foreground color
						BackColor;										// pixel = off: background color
					for (unsigned int C=1; C<=XScale; ++C)			// as often as needed by the horizotal scale factor (e.g. draws pixel twice if double wide font is requested)
						LCD_SPI_Int(Color);								// draw pixel
				}										// check next bit of this character
			}											// draw this line again if needed for scaling
		}
		SET(LCD_SELECT);						// Disable display												// next row of this character
		X += CellWidth*XScale;
	}													// next character of this string
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
//Clear display
//-------------------------------------------------------------------------------

void LCD_Cls(unsigned int color) 
{ 
	LCD_Window(0, 0, WindowWidth()-1, WindowHeight()-1);
	for (unsigned int i=0; i<(WindowWidth()*WindowHeight()); ++i) 
		LCD_SPI_Int(color); 
	SET(LCD_SELECT);													// 'Disable display 
} 

//-------------------------------------------------------------------------------
// Plot one pixel to the display
//-------------------------------------------------------------------------------
void LCD_Plot(unsigned char x1, unsigned char y1, unsigned char line_type, unsigned int color)
{
	if (line_type == THICK)
	{
		LCD_Window(x1, y1, x1+1, y1+1); 									// Define Window, exactly four pixels large
		LCD_SPI_Int(color);													// send four pixels (avoid loop overhead)
		LCD_SPI_Int(color);
		LCD_SPI_Int(color);
		LCD_SPI_Int(color);
	}
	else
	{
		LCD_Window(x1, y1, x1, y1); 										// Define Window, exactly one pixel large
		LCD_SPI_Int(color);													// send pixel
	}
	SET(LCD_SELECT);													// Disable Display
}

//-------------------------------------------------------------------------------
// Draw a rectangular filled box
//-------------------------------------------------------------------------------
void LCD_Box(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned int color)
{
	LCD_Window(x1, y1, x2, y2);   										// Define Window
	for (unsigned int i=0; i<((x2-x1+1)*(y2-y1+1)); ++i)			// for every pixel...
		LCD_SPI_Int(color); 											// ...send color information
	SET(LCD_SELECT);													// Disable Display
}

//-------------------------------------------------------------------------------
// Draw a rectangular (not filled)
//-------------------------------------------------------------------------------
void LCD_Rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char line_type, unsigned int color)	//draw rectangle
{
      LCD_Draw(x1, y1, x2, y1, line_type, color);
      LCD_Draw(x2, y1, x2, y2, line_type, color);
      LCD_Draw(x1, y2, x2, y2, line_type, color);
      LCD_Draw(x1, y1, x1, y2, line_type, color);
}

//-------------------------------------------------------------------------------
// Draw a line
//-------------------------------------------------------------------------------
void LCD_Draw(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char line_type, unsigned int color)//draw line - bresenham algorithm
{
   int x = x1;
   int y = y1;
   int d = 0;
   int hx = x2 - x1;    									// how many pixels on each axis
   int hy = y2 - y1;
   int xinc = 1;
   int yinc = 1;
   if (hx < 0)
   {
      xinc = -1;
      hx = -hx;
   }
   if (hy < 0)
   {
      yinc = -1;
      hy = -hy;
   }
   if (hy <= hx)
   {
      int c = 2 * hx;
      int m = 2 * hy;
      while (x != x2)
	  {
         LCD_Plot(x, y, line_type, color);
         x += xinc;
         d += m;
         if (d > hx)
		 {
            y += yinc;
            d -= c;
         }
      }
   }
   else
   {
      int c = 2 * hy;
      int m = 2 * hx;
      while (y != y2)
	  {
         LCD_Plot(x, y, line_type, color);
         y += yinc;
         d += m;
         if (d > hy)
		 {
            x += xinc;
            d -= c;
         }
      }
   }
   LCD_Plot(x, y, line_type, color); 												  // finally, the last pixel
 }

//-------------------------------------------------------------------------------
// Draw an unfilled circle
//-------------------------------------------------------------------------------
void LCD_CirclePoints(int x, int y, int xOfs, int yOfs, unsigned char line_type, unsigned int color)
{
	LCD_Plot(xOfs+x, yOfs+y, line_type, color);
	LCD_Plot(xOfs+y, yOfs+x, line_type, color);
	LCD_Plot(xOfs+y, yOfs-x, line_type, color);
	LCD_Plot(xOfs+x, yOfs-y, line_type, color);
	LCD_Plot(xOfs-x, yOfs-y, line_type, color);
	LCD_Plot(xOfs-y, yOfs-x, line_type, color);
	LCD_Plot(xOfs-y, yOfs+x, line_type, color);
	LCD_Plot(xOfs-x, yOfs+y, line_type, color);
}


//-------------------------------------------------------------------------------
// Draw a filled circle
//-------------------------------------------------------------------------------
void LCD_Circle(int x1, int y1, int r, unsigned char fill, unsigned char line_type, unsigned int color)	//draw circle
{
	int x=0, y=r, d=1-r, deltaE=3, deltaSE=-2*r+5;
	LCD_CirclePoints(x, y, x1, y1, line_type, color);
	if	(fill)
		LCD_Draw(x1-y, y1-x, x1+y, y1-x, line_type, color);
	while (y > x)
	{
		if (d < 0)	// select E
		{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			++x;
		}
		else       // select SE
		{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			++x; --y;
		}
		if (fill)
		{
			LCD_Box(x1-x, y1+y, x1+x, y1+y, color);			
			LCD_Box(x1-x ,y1-y, x1+x, y1-y, color);
			LCD_Box(x1-y, y1+x, x1+y, y1+x, color);
			LCD_Box(x1-y, y1-x, x1+y, y1-x, color);
		}
		else
		{
			LCD_CirclePoints(x, y, x1, y1, line_type, color);
		}
	}
}

//yes no dialog for Landscape
//output a message with title and text
//at last function waits for keypress
_Bool LCD_Dialog(char *title, char *text, unsigned int BackColor, unsigned int ForeColor)  {
  uint8_t x,y,i,len;
  LCD_Cls(BackColor);
  
  char temp[2];
  temp[1] = '\0';
  
  if (Orientation == Landscape)
  {
	LCD_Box(10,25,160,120,ForeColor);
	LCD_Print(title,87-(strlen(title)*5),1,2,1,1,ForeColor,BackColor);
  }
  else {
	LCD_Box(10,25,120,165,ForeColor);
	LCD_Print(title,65-(strlen(title)*5),1,2,1,1,ForeColor,BackColor);
  }
  
  //for Landscape only
  x = 13;
  y = 28;
  len = strlen(text);
  for(i=0;i<len;i++)  {
    if(text[i] != '\n')  
	{
	  temp[0] = text[i];
	  LCD_Print(temp,x,y,1,1,1,BackColor,ForeColor);
	  x += 6;
	  if(x >= 160)  
	  {
	    if(y>=110)
		  break;
		y += 10;
		x = 13;
	    continue;
	  }
	}
	else  {
	  if(y >= 110)
	    break;
	  y += 10;
	  x = 13;
	  continue;
	}
  }
  
  LCD_Print("Y",165,1,2,1,1,ForeColor,BackColor);
  LCD_Print("e",165,17,2,1,1,ForeColor,BackColor);
  LCD_Print("s",165,33,2,1,1,ForeColor,BackColor);
  
  LCD_Print("N",165,99,2,1,1,ForeColor,BackColor);
  LCD_Print("o",165,115,2,1,1,ForeColor,BackColor);
  
  while(1)  
  {	//check for pressed Key
	switch(keyhit())  
	{
		case KEY_TOP_S5:	return 1;
		case KEY_BOT_S10:	return 0;
	}
  }
}

/*static void h_draw(int x1, int x2, int y, unsigned char line_type, unsigned int color)
{
  while (x1 != x2)
  {
	 LCD_Box(x1,y,x2,y,color);
  }
}
*/

//-------------------------------------------------------------------------------
// Calculate 2-byte color from 1-byte color (332 RGB); used by Bitmap_256low... bitmap routines
//-------------------------------------------------------------------------------
//static unsigned int CalcColor(unsigned char Color256)
//{
	//// NOTE: A lookup-table with alle the 256 colors would be much faster.
	//// It would consume 512 bytes, which is not much more than the code
	//// in this function plus the encoded function call overhead
	//unsigned int R = Color256 >> 5;			// 3 most significant bits
	//unsigned int G = (Color256 >> 2) & 0x7;	// 3 bits of the middle
	//unsigned int B = Color256 & 0x3;			// 2 least significant bits
	//R *= 4;		if (R == 28) R = 31;
	//G *= 9;
	//B *= 10;	if (B == 30) R = 31;
	//return (R << 11) + (G << 5) + (B);
//}

//-------------------------------------------------------------------------------
// Send data for 65k color bitmap file
//-------------------------------------------------------------------------------
//static void Bitmap_65k_uncompressed(const uint16_t Pbmp[], unsigned int PixelCount)
//{
	//for(unsigned int i=0; i<PixelCount; ++i)
	//{
		//const unsigned int PixelColor = pgm_read_word(&Pbmp[i]);
		//LCD_SPI_Int(PixelColor);
	//}
//}

//-------------------------------------------------------------------------------
// Send data for compressed 65k color bitmap file
//-------------------------------------------------------------------------------
//static void Bitmap_65k_compressed(const uint16_t Pbmp[], unsigned int PixelCount)
//{
	//unsigned int PixelColor = 0;
	//unsigned int Pos = 0;
	//_Bool FirstRead = true;
	//for (unsigned int i=0; i<PixelCount; ++i)
	//{
		//if (FirstRead)										// read first pixel after first start or after finishing a compressed bunch of data
		//{
			//PixelColor = pgm_read_word(&Pbmp[Pos++]);
			//LCD_SPI_Int(PixelColor);						// paint pixel
			//FirstRead = false;
		//}
		//else
		//{
			//unsigned int LastPixel = PixelColor;						// save data of last pixel (word format) to temporary variable
			//PixelColor = pgm_read_word(&Pbmp[Pos++]);					// read next pixel
			//LCD_SPI_Int(PixelColor);									// paint new pixel
			//if (LastPixel == PixelColor)								// check if the last two read pixels are identical
			//{
				//unsigned int Repeat = pgm_read_word(&Pbmp[Pos++]);	// if yes: read number following pixels of this color
				//i += Repeat;											// increment pixel counter
				//while (Repeat--)										// output requested number of pixels
					//LCD_SPI_Int(PixelColor);
				//FirstRead = true;										// restart potential repetition sequence
			//}
		//}
    //}
//}

//-------------------------------------------------------------------------------
// Send data for 256 color bitmap file (332 RGB)
//-------------------------------------------------------------------------------
//static void Bitmap_256low_uncompressed(const uint8_t Pbmp[], unsigned int PixelCount)
//{
	//for(unsigned int i=0; i<PixelCount; ++i)
	//{
		//const unsigned char PixelColor256 = pgm_read_byte(&Pbmp[i]);
		//LCD_SPI_Int(CalcColor(PixelColor256));
	//}
//}

//-------------------------------------------------------------------------------
// Send data for compressed 256 color bitmap file (332 RGB)
//-------------------------------------------------------------------------------
//static void Bitmap_256low_compressed(const uint8_t Pbmp[], unsigned int PixelCount)
//{
	//unsigned char PixelColor = 0;
	//unsigned int Pos = 0;
	//_Bool FirstRead = true;
	//for (unsigned int i=0; i<PixelCount; ++i)
	//{
		//if (FirstRead)										// read first pixel after first start or after finishing a compressed bunch of data
		//{
			//PixelColor = pgm_read_byte(&Pbmp[Pos++]);
			//LCD_SPI_Int(CalcColor(PixelColor));			// paint pixel
			//FirstRead = false;
		//}
		//else
		//{
			//unsigned char LastPixel = PixelColor;						// save data of last pixel (word format) to temporary variable
			//PixelColor = pgm_read_byte(&Pbmp[Pos++]);					// read next pixel
			//LCD_SPI_Int(CalcColor(PixelColor));						// paint new pixel
			//if (LastPixel == PixelColor)								// check if the last two read pixels are identical
			//{
				//unsigned char Repeat = pgm_read_byte(&Pbmp[Pos++]);	// if yes: read number following pixels of this color
				//i += Repeat;											// increment pixel counter
				//while (Repeat--)										// output requested number of pixels
					//LCD_SPI_Int(CalcColor(PixelColor));
				//FirstRead = true;										// restart potential repetition sequence
			//}
		//}
    //}
//}

//-------------------------------------------------------------------------------
// Send data for 256 out of 65k color bitmap file (using color table)
//-------------------------------------------------------------------------------
//static void Bitmap_256high_uncompressed(const uint8_t Pbmp[], const uint16_t ColorTable[], unsigned int PixelCount)
//{
	//for(unsigned int i=0; i<PixelCount; ++i)
	//{
		//const unsigned char PixelColorIndex = pgm_read_byte(&Pbmp[i]);
		//const unsigned int Color = pgm_read_word(&ColorTable[PixelColorIndex]);
		//LCD_SPI_Int(Color);
	//}
//}

//-------------------------------------------------------------------------------
// Send data for compressed 256 out of 65k color bitmap file (using color table)
//-------------------------------------------------------------------------------
//static void Bitmap_256high_compressed(const uint8_t Pbmp[], const uint16_t ColorTable[], unsigned int PixelCount)
//{
	//unsigned char PixelColorIndex = 0;
	//unsigned int Pos = 0;
	//_Bool FirstRead = true;
	//for (unsigned int i=0; i<PixelCount; ++i)
	//{
		//if (FirstRead)										// read first pixel after first start or after finishing a compressed bunch of data
		//{
			//PixelColorIndex = pgm_read_byte(&Pbmp[Pos++]);
			//LCD_SPI_Int(pgm_read_word(&ColorTable[PixelColorIndex]));	// paint pixel
			//FirstRead = false;
		//}
		//else
		//{
			//unsigned char LastPixelIndex = PixelColorIndex;			// save data of last pixel (word format) to temporary variable
			//PixelColorIndex = pgm_read_byte(&Pbmp[Pos++]);				// read next pixel
			//LCD_SPI_Int(pgm_read_word(&ColorTable[PixelColorIndex]));	// paint new pixel
			//if (LastPixelIndex == PixelColorIndex)						// check if the last two read pixels are identical
			//{
				//unsigned char Repeat = pgm_read_byte(&Pbmp[Pos++]);	// if yes: read number following pixels of this color
				//i += Repeat;											// increment pixel counter
				//while (Repeat--)										// output requested number of pixels
					//LCD_SPI_Int(pgm_read_word(&ColorTable[PixelColorIndex]));
				//FirstRead = true;										// restart potential repetition sequence
			//}
		//}
    //}
//}


//-------------------------------------------------------------------------------
// Display 65k color bitmap file
//-------------------------------------------------------------------------------
//void LCD_Bitmap_65k(int x1, int y1, int x2, int y2, const uint16_t Pbmp[], _Bool Compressed)  //draw bitmap
//{	
	//LCD_Window(x1, y1, x2, y2);    // Define Window	
	//
	//const int Width = x2 - x1 + 1;
	//const int Height = y2 - y1 + 1;
	//const unsigned int PixelCount = Width * Height;
//
	//if (Compressed)
		//Bitmap_65k_compressed(Pbmp, PixelCount);
	//else
		//Bitmap_65k_uncompressed(Pbmp, PixelCount);
//
	//SET(LCD_SELECT);				 // Disable display
//}

//-------------------------------------------------------------------------------
// Display 256 color bitmap file (332 RGB)
//-------------------------------------------------------------------------------
//void LCD_Bitmap_256low(int x1, int y1, int x2, int y2, const uint8_t Pbmp[], _Bool Compressed)
//{	
	//LCD_Window(x1, y1, x2, y2);    // Define Window	
	//
	//const int Width = x2 - x1 + 1;
	//const int Height = y2 - y1 + 1;
	//const unsigned int PixelCount = Width * Height;
//
	//if (Compressed)
		//Bitmap_256low_compressed(Pbmp, PixelCount);
	//else
		//Bitmap_256low_uncompressed(Pbmp, PixelCount);
//
	//SET(LCD_SELECT);				 // Disable display
//}

//-------------------------------------------------------------------------------
// Display 256 out of 65k color bitmap file (using color table)
//-------------------------------------------------------------------------------
//void LCD_Bitmap_256high(int x1, int y1, int x2, int y2, const uint8_t Pbmp[], const uint16_t ColorTable[], _Bool Compressed)
//{
	//LCD_Window(x1, y1, x2, y2);    // Define Window	
	//
	//const int Width = x2 - x1 + 1;
	//const int Height = y2 - y1 + 1;
	//const unsigned int PixelCount = Width * Height;
//
	//if (Compressed)
		//Bitmap_256high_compressed(Pbmp, ColorTable, PixelCount);
	//else
		//Bitmap_256high_uncompressed(Pbmp, ColorTable, PixelCount);
//
	//SET(LCD_SELECT);				 // Disable display
//}



//##### SPI START  ##############################################################################################
//   If you do not want to use hardware SPI but software SPI (e.g. because your controller does not offer hardware SPI,
//   exchange the called subroutines:
//   1): LCD_SPI_Byte with subroutine LCD_Send_Byte  AND....
//   2): LCD_SPI_DBCommand with LCD_Send_DBCommand
//   But it should be mentioned, that hardware SPI is up to 10 times faster than software SPI!

// Send single bytes (or array with Bytes) by SPI 
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

// Send word (16 bit) (or array with words) by SPI 
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

// Send integer by SPI 
void LCD_SPI_Int(unsigned int Value) 
{ 
	SPCR |= _BV(SPE);
	SPDR = (Value >> 8) & 0xff;
	LCD_Wait();
	SPCR |= _BV(SPE);
	SPDR = Value & 0xff;
	LCD_Wait();
}

// Wait for SPI transfer is done
void LCD_Wait(void)
{						
    while (SPCR & _BV(SPE))
    {
        while (!(SPSR & (_BV(SPIF))));
        	SPCR &= ~(_BV(SPE));
    }
}


//##### SPI END   ###################################################################################################

// Replacement subroutines for SPI. IF you do not want to use hardware SPI you may use software SPI (e.g. if you want to
// change the code to be used by a controller without hardware SPI capabilities)
// Then you have to send all needed bits on all lines manually instead of letting the controller doing the work.
// Caution: With a usual 16 or 20 Mhz Controller, software SPI is always slower than hardware SPI.
//
// What to do to change the code for software-SPI (see also blueline-soft.c as an example):
// 1) Uncomment the following lines
// 2) change ALL occurencies of "LCD_SPI...." to "LCD_Send..." in glcd-Display3000-211.c and glcd-Display3000-211.h (use search & replace)
// 3) At subroutine LCD_Init, delete the two lines at the beginning with SPSR ... and SPCR.....
//Thats all.

/*
// software SPI: sends all 16 Bits of each Word or a Data Array
void LCD_Send_DBCommand(const unsigned int Data[], unsigned int Count) 
{ 
	for (int i=0; i<Count; ++i)			// Count DBytes 
	{ 
		RESET(LCD_SELECT);					// CSelect := 0 
		SET(LCD_DC);						// DataCmd := 1 
		//....data 
		for (int Bit=15; Bit>=0; --Bit)	// vom msb zum lsb 
		{ 
			RESET(LCD_CLK);					// Clock := 0 
			if (Data[i] & _BV(Bit))		// Bit testen 
				SET(LCD_DATA);				// falls das Bit 1 ist 
			else 
				RESET(LCD_DATA);			// falls das Bit 0 ist 
			SET(LCD_CLK);					// Clock := 1
		} 
		RESET(LCD_DC);						// DataCmd := 0 
		SET(LCD_SELECT);					// Disable display
	} 
}


// software SPI: sends all 8 Bits of a Byte
void LCD_Send_Byte(const unsigned char Data[], unsigned int Count) 
{ 
	//....data 
	for (unsigned int i=0; i<Count; ++i)	// Count Bytes 
	{
		for (int Bit=7; Bit>=0; --Bit)	// vom msb zum lsb 
		{ 
			RESET(LCD_CLK);					// Clock := 0 
			if (Data[i] & _BV(Bit))		// Bit testen 
				SET(LCD_DATA);				// falls das Bit 1 ist 
			else 
				RESET(LCD_DATA);			// falls das Bit 0 ist 
			SET(LCD_CLK);					// Clock := 1 
		} 
	}
} 

void LCD_Send_Int(unsigned int Value) 
{ 

		for (int Bit=15; Bit>=0; --Bit)	// vom msb zum lsb 
		{ 
			RESET(LCD_CLK);					// Clock := 0 
			if (Value & _BV(Bit))		// Bit testen 
				SET(LCD_DATA);				// falls das Bit 1 ist 
			else 
				RESET(LCD_DATA);			// falls das Bit 0 ist 
			SET(LCD_CLK);					// Clock := 1 
		} 
}
*/




// ------------------------------------------------------------------------------------------------------
// History 
// V1.03: 07.05.2008: speed up of filled circle by optimizing the code
// V1.03: 07.05.2008: Corrected the status of the "CS" line which could cause a problem when using several devices on the SPI bus
// V1.02: 29.04.2008: optimized some of the SPI line settings
// V1.01: first official version of Library


