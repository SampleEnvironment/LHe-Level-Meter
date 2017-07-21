
#include "keyboard.h"

///init keyboard for module atmega2561 
void keyboard_init(void)  {
  DDRD &= ~((1 << DDD6) | (1 << DDD5) | (1 << DDD4) | (1 << DDD3) | (1 << DDD2));//= 0b10000011;						//set all but pins 0,1 and 7 to input
  DDRC &= ~((1 << DDC6) | (1 << DDC7));	// set pin6 and 7 on portC as Input, leave rest untouched
  DDRA &= ~(1 << DDA7);					// set pin7 on portA as Input	
  
  
  PORTD |= 0b01111100;		//set pins 2,3,4,5,6, to 1 for intern pull up 
  PORTC |= (1 << DDC6); 	// intern pullup for C6
  PORTC |= (1 << DDC7);
  PORTA |= (1 << DDA7);
  
  
  
}

///asks for a keypress, returns 0 or pressed key
int keyhit(void)  {
  register unsigned char value = (~(PIND))&0b01111100;
  switch(value)  
  {	
	//case 0b10000000:	return KEY_LEFT_1; 	//D7 --> must be A7!! (S10)

	case 0x40:			return KEY_LEFT_2;	//D6 (S9)

    case 0x20:			return KEY_DOWN;   	//D5 (S8)

    case 0x10:			return KEY_UP;		// D4 (S7)

    case 0x04:			return KEY_RIGHT_1;	// D2 (S5)

	case 0x08:			return KEY_RIGHT_2;	// D3 (S6)
	
	//case 0x24:			return HIDDEN_FUNCTION;	// D2&D6 --> must be C6!! (S3&S4)

    //default:			return 0;
  }
  
  
  value = (~(PINC))&0b11000000;
  switch(value)  
  {	
	case 0b01000000:	return HIDDEN_FUNCTION;	// C6!! (S3&S4)
	
	case 0b10000000:	return KEY_FILL;	// C7!! (S1)

    //default:			return 0;
  }
  
  value = (~(PINA))&0b10000000;
  switch(value)  
  {	
	case 0b10000000:	return KEY_LEFT_1;	// A7 (S10)
    //default:			return 0;
  }
  
  
  return 0;
}

///waits until key is pressed
int getkey(void)  {
  int v;
  while(keyhit()!=0);				//wait for release of key
  while((v = keyhit())==0);		//wait for a new pressed key
  _delay_ms(40.0);
  return v;  
}
