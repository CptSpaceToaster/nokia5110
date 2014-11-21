/*
 * nokia5110.c
 *
 * Created: 11/20/2014 8:15:16 PM
 * Author: CaptainSpaceToaster
 */ 
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "nokia5110.h"
#include "screen.h"

#ifndef WIDTH
#  error "define WIDTH before nokia5110.c"
#endif
#ifndef HEIGHT
#  error "define HEIGHT before nokia5110.c"
#endif

//global variable for remembering where to start writing the next text string on 3310 LCD
unsigned char char_start;

/* current cursor */
volatile static unsigned int cursor_row = 0;
volatile static unsigned int cursor_col = 0;
static unsigned char lcd_buffer[HEIGHT/8][WIDTH];



/*--------------------------------------------------------------------------------------------------
  Name         :  spi_init
  Description  :  Initialising atmega SPI for using with 3310 LCD
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
//SPI initialize
//clock rate: 250000hz
void spi_init( unsigned char reg )
{
	DDRLCD |= _BV(LCD_DC_PIN) | _BV(LCD_CE_PIN) | _BV(SPI_MOSI_PIN) | _BV(LCD_RST_PIN) | _BV(SPI_CLK_PIN);
	SPCR = reg; //setup SPI
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_init
  Description  :  LCD controller initialization.
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_init ( void )
{
	CLEAR_SCE_PIN;    //Enable LCD

	CLEAR_RST_PIN;
	_delay_ms(100);
	SET_RST_PIN;	//reset LCD
			  
	SET_SCE_PIN;	//disable LCD

	LCD_writeCommand( 0x21 );  // LCD Extended Commands.
	LCD_writeCommand( 0xD0 );  // Set LCD Vop (Contrast).
	LCD_writeCommand( 0x04 );  // Set Temp coefficent.
	LCD_writeCommand( 0x13 );  // LCD bias mode 1:48.
	LCD_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
	LCD_writeCommand( 0x0c );  // LCD in normal mode.

	LCD_clear();
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeCommand
  Description  :  Sends command to display controller.
  Argument(s)  :  command -> command to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeCommand ( unsigned char command )
{
	CLEAR_SCE_PIN;	  //enable LCD

	CLEAR_DC_PIN;	  //set LCD in command mode

	//  Send data to display controller.
	SPDR = command;

	//  Wait until Tx register empty.
	while ( !(SPSR & 0x80) );

	SET_SCE_PIN;   	 //disable LCD
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeData
  Description  :  Sends Data to display controller.
  Argument(s)  :  Data -> Data to be sent
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeData ( unsigned char Data )
{
	CLEAR_SCE_PIN;	  //enable LCD

	SET_DC_PIN;	  //set LCD in Data mode

	//  Send data to display controller.
	SPDR = Data;

	//  Wait until Tx register empty.
	while ( !(SPSR & 0x80) );

	SET_SCE_PIN;   	 //disable LCD
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_clear
  Description  :  Clears the display
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_clear ( void )
{
	int i,j;
	
	LCD_gotoXY (0,0);  	//start with (0,0) position

	for(i=0; i<(HEIGHT/8); i++)
	{
		for(j=0; j<WIDTH; j++)
		{
			LCD_writeData( 0x00 );
			if ((i < (HEIGHT/8)) && (j < WIDTH))
				lcd_buffer[i][j] = 0x00;
		}
	}
   
    LCD_gotoXY (0,0);	//bring the XY position back to (0,0)
      
}

void LCD_update( void )
{
	int i,j;
	
	LCD_gotoXY (0,0);  	//start with (0,0) position

	for(i=0; i<(HEIGHT/8); i++)
	{
		LCD_gotoXY (0,i);
		for(j=0; j<WIDTH; j++)
		{
			LCD_writeData(lcd_buffer[i][j]);
		}
	}
   
	LCD_gotoXY (0,0);	//bring the XY position back to (0,0)
      
}

void LCD_drawSplash( void )
{
	int i,j;
	
	for(i=0; i<(HEIGHT/8); i++)
	{
		for(j=0; j<WIDTH; j++)
		{
			lcd_buffer[i][j] = pgm_read_byte(&(splash[i*WIDTH+j]));
		}
	}
   
	LCD_update();
      
}

void LCD_drawSplashNoUpdate( void )
{
	int i,j;
	
	for(i=0; i<HEIGHT/8; i++)
	{
		for(j=0; j<84; j++)
		{
			lcd_buffer[i][j] = pgm_read_byte(&(splash[i*WIDTH+j]));
		}
	}
}

void LCD_setPixelNoUpdate( unsigned char x, unsigned char y )
{
	unsigned char value;
	unsigned char row;
	
	row = y / 8;

	value = lcd_buffer[row][x];
	value |= (1 << (y % 8));
	lcd_buffer[row][x] = value;
}

void LCD_setPixel( unsigned char x, unsigned char y )
{
	unsigned char value;
	unsigned char row;
	
	row = y / 8;

	value = lcd_buffer[row][x];
	value |= (1 << (y % 8));
	lcd_buffer[row][x] = value;

	//LCD_update();
	LCD_gotoXY (x,row);
	LCD_writeData(value);
}

void LCD_clearPixel( unsigned char x, unsigned char y )
{
	unsigned char value;
	unsigned char row;
	
	row = y / 8;

	value = lcd_buffer[row][x];
	value &= ~(1 << (y % 8));
	lcd_buffer[row][x] = value;

	//LCD_update();
	LCD_gotoXY (x,row);
	LCD_writeData(value);
}

void LCD_clearPixelNoUpdate( unsigned char x, unsigned char y )
{
	unsigned char value;
	unsigned char row;
	
	row = y / 8;

	value = lcd_buffer[row][x];
	value &= ~(1 << (y % 8));
	lcd_buffer[row][x] = value;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_gotoXY
  Description  :  Sets cursor location to xy location corresponding to basic font size.
  Argument(s)  :  x - range: 0 to 84
  			   	  y -> range: 0 to 6
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_gotoXY ( unsigned char x, unsigned char y )
{
	LCD_writeCommand (0x80 | x);   //column
	LCD_writeCommand (0x40 | y);   //row

	cursor_row = y;
	cursor_col = x;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar
  Description  :  Displays a character at current cursor location and increment cursor location.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar (unsigned char ch)
{
	unsigned char j;
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<5; j++)
		lcd_buffer[cursor_row][cursor_col + j + 1] = pgm_read_byte(&(smallFont [(ch-32)*5 + j] ));

	lcd_buffer[cursor_row][cursor_col + 6] = 0x00;

	for(j=0; j<7; j++)
		LCD_writeData(lcd_buffer[cursor_row][cursor_col++]);
} 

void LCD_writeChar_P (unsigned char ch)
{
	unsigned char j;
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<5; j++)
	lcd_buffer[cursor_row][cursor_col + j + 1] = pgm_read_byte(&(smallFont [(ch-32)*5 + j] ));

	lcd_buffer[cursor_row][cursor_col + 6] = 0x00;

	for(j=0; j<7; j++) {
		printf("%d ", cursor_col);
		LCD_writeData(lcd_buffer[cursor_row][cursor_col++]);
	}
	printf("\n");
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar
  Description  :  Displays a character at current cursor location and increment cursor location.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar_L (unsigned char ch)
{
	unsigned char j;
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<5; j++)
	lcd_buffer[cursor_row][cursor_col + j + 1] = pgm_read_byte(&(smallFont [(ch-32)*5 + j] ));

	lcd_buffer[cursor_row][cursor_col + 6] = 0x00;

	for(j=0; j<7; j++) {
		if (cursor_col<WIDTH) {
			LCD_writeData(0x00);
		} else {
			LCD_writeData(lcd_buffer[cursor_row][cursor_col]);
		}
		cursor_col++;
	}
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar
  Description  :  Displays a character at current cursor location and increment cursor location.
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar_C (unsigned char ch)
{
	unsigned char j;
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<5; j++)
		lcd_buffer[cursor_row][cursor_col + j + 1] = pgm_read_byte(&(smallFont [(ch-32)*5 + j] ));

	lcd_buffer[cursor_row][cursor_col + 6] = 0x00;

	for(j=0; j<7; j++) {
		if(cursor_col<(WIDTH)) {
			LCD_writeData(lcd_buffer[cursor_row][cursor_col++]);
		}
	}
}

#if 1

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar_megaFont
  Description  :  Displays a character in large fonts, used here for displatying temperature 
		  (for displaying '.', '+', '-', and '0' to '9', stored 
		  in 3310_routines.h as three dimensional array, number[][][])
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar_megaFont (unsigned char ch)
{
	unsigned char i, j;
   
	if(ch == '.')
		ch = 10;
	else if (ch == '+')
		ch = 11;
	else if (ch == '-')
		ch = 12;
	else
		ch = ch & 0x0f;
	
	for(i=0;i<3;i++)
	{	
		LCD_gotoXY (4 + char_start, 1+i);
 
		for(j=0; j<16; j++) {
			lcd_buffer[cursor_row][cursor_col + j] |=  pgm_read_byte(&(number[ch][i][j]));
			LCD_writeData(lcd_buffer[cursor_row][cursor_col + j]);
		}
	} 
   
	if(ch == '.') 
		char_start += 5;
	else
		char_start += 12;
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString_megaFont
  Description  :  Displays a string at current location, in large fonts
  Argument(s)  :  string -> Pointer to ASCII string (stored in RAM)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString_megaFont ( char *string )
{
	char_start = 2;
	
	while ( *string )
		LCD_writeChar_megaFont( *string++ );
	
	//LCD_gotoXY(char_start+6, 2);
	//LCD_writeChar('z'+1); 			  //symbol of Degree
	//LCD_writeChar('F');
}
#endif

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString_F
  Description  :  Displays either the first 12 characters of a string, or a 
				  string stored in FLASH, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString_C ( const char *string )
{
	int index = 0;
	while ( *string && index < 12) {	
		index ++;
		LCD_writeChar_C( *string++ );
	}
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString_F
  Description  :  Displays either the first 12 characters of a string, or a 
				  string stored in FLASH, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString_L ( const char *string )
{
	LCD_writeChar_L( *string++ );
	if (cursor_col>=WIDTH) {
		cursor_col-=WIDTH;
		cursor_row++;
	}
	uint8_t index = 0;
	while ( *string && index < 12) {	
		index ++;
		LCD_writeChar_C( *string++ );
	}
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString_F
  Description  :  Displays a string stored in FLASH, in small fonts (refer to 3310_routines.h)
  Argument(s)  :  string -> Pointer to ASCII string (stored in FLASH)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString_F ( const char *string )
{
	while ( *string )
		LCD_writeChar( *string++ );
}


/*--------------------------------------------------------------------------------------------------
  Name         :  delay_ms
  Description  :  1 millisec delay (appx.)
  Argument(s)  :  None.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void delay_ms(int miliSec)  //for 1Mhz clock
{
	int i,j;
  
	for(i=0;i<miliSec;i++)
		for(j=0;j<100;j++)
		{
			asm("nop");
			asm("nop");
		}
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_drawBorder
  Description  :  Draws rectangle border on the display
  Argument(s)  :  None
  Return value :  None
--------------------------------------------------------------------------------------------------*/
void LCD_drawBorder (void )
{
	unsigned char i, j;  
	    
	for(i=0; i<(WIDTH/8); i++)
	{
		LCD_gotoXY (0,i);
			
		for(j=0; j<84; j++)
		{
			if(j == 0 || j == WIDTH-1) {
				lcd_buffer[cursor_row][cursor_col + j] |= 0xff;
			} else if(i == 0) {
				lcd_buffer[cursor_row][cursor_col + j] |= 0x01;
				//LCD_writeData (0x08);		// row 0 is having only 5 bits (not 8)
			} else if(i == (HEIGHT/8)-1) {
				lcd_buffer[cursor_row][cursor_col + j] |= 0x80;
				//LCD_writeData (0x04);		// row 6 is having only 3 bits (not 8)
			} else {
				lcd_buffer[cursor_row][cursor_col + j] |= 0x00;
				//LCD_writeData (0x00);
			}
		}
	}

	LCD_update();
}	

/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

