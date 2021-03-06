/*
 * nokia5110.c
 *
 * Created: 11/20/2014 8:15:16 PM
 * Author: CaptainSpaceToaster
 */ 
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.h"
#include "screen.h"

#ifndef WIDTH
#  error "define WIDTH before nokia5110.c"
#endif
#ifndef HEIGHT
#  error "define HEIGHT before nokia5110.c"
#endif

//Should be true if initializing.  This is not yours to mess with...
bool is_init = false;

/* current cursor */
uint16_t cursor_row = 0;
uint16_t cursor_col = 0;
/* These represent where the cursor is on the LCD... not where we think we are */
uint16_t dirty_cursor_row = 0;
uint16_t dirty_cursor_col = 0;
/* Index variables... declared once here, and used by various methods */
uint8_t i, j;

void nokia5110_spi_init(uint8_t reg) {
	//SPI initialize
	//clock rate: 250000hz
	DDRLCD |= _BV(LCD_DC_PIN) | _BV(LCD_CE_PIN) | _BV(SPI_MOSI_PIN) | _BV(LCD_RST_PIN) | _BV(SPI_CLK_PIN);
	SPCR = reg; //setup SPI with a given register value... read the data sheet to see what you can do!
}

void nokia5110_power_on(void) {
	is_init = true;
	CLEAR_SCE_PIN;  // lowering the select pin
	CLEAR_RST_PIN;  // lowering reset (initiating the reset procedure)
	_delay_ms(100); // waiting a bit for things to settle
	SET_RST_PIN;	// raising reset (taking the device OUT of reset)
	SET_SCE_PIN;	// setting the select pin high, ending the reset procedure... SPI should work now.

	nokia5110_writeCommand( 0x21 );  // LCD Extended Commands.
	nokia5110_writeCommand( 0xD0 );  // Set LCD Vop (Contrast).
	nokia5110_writeCommand( 0x04 );  // Set Temp coefficent.
	nokia5110_writeCommand( 0x13 );  // LCD bias mode 1:48.
	nokia5110_writeCommand( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
	nokia5110_writeCommand( 0x0c );  // LCD in normal mode.
	
	nokia5110_clear();
	is_init = false;
}

void nokia5110_writeData(uint8_t data) {
	static uint8_t lcd_buffer[HEIGHT/8][WIDTH]; // may need to go
	static bool cursor_is_dirty = false;

	/* Only clock out the data if it differs from the buffer */
	if (lcd_buffer[cursor_row][cursor_col] != data || is_init) {
		/* Move the cursor if we DIDN'T do that earlier */
		if (cursor_is_dirty) {
			nokia5110_gotoXY(cursor_col, cursor_row);
			cursor_is_dirty = false;
		}
		lcd_buffer[cursor_row][cursor_col] = data;
		CLEAR_SCE_PIN;            // enable LCD
		SET_DC_PIN;               // set LCD in Data mode
		SPDR = data;              // send data to display controller.
		while ( !(SPSR & 0x80) ); // wait until Tx register empty.
		SET_SCE_PIN;              // disable LCD
	} else {
		cursor_is_dirty = true;
	}
	
	//increment the cursors
	cursor_col++;
	if (cursor_col >= WIDTH) {
		cursor_col -= WIDTH;
		cursor_row++;
	}
	if (cursor_row >= HEIGHT) {
		cursor_row -= HEIGHT;
	}
	
	if (!cursor_is_dirty) {
		dirty_cursor_col = cursor_col;
		dirty_cursor_row = cursor_row;
	}
}

void nokia5110_writeCommand(uint8_t command) {
	CLEAR_SCE_PIN;	          // enable LCD
	CLEAR_DC_PIN;             // set LCD in command mode
	SPDR = command;           // send data to display controller.
	while (!(SPSR & 0x80));   // wait until Tx register empty.
	SET_SCE_PIN;              // disable LCD
}

void nokia5110_gotoXY(uint8_t column, uint8_t row) {
	if (dirty_cursor_col != column && column < WIDTH) {
		nokia5110_writeCommand(0x80 | column);
	}
	if (dirty_cursor_row != row && row < HEIGHT) {
		nokia5110_writeCommand(0x40 | row);
	}

	cursor_col = dirty_cursor_col = column;
	cursor_row = dirty_cursor_row = row;
}

void nokia5110_clear(void) {
	nokia5110_gotoXY(0,0);

	for(i=0; i<(HEIGHT/8); i++) {
		for(j=0; j<WIDTH; j++) {
			nokia5110_writeData(0x00);
		}
	}
}

void nokia5110_drawSplash(void) {
	nokia5110_gotoXY(0,0);
	
	for(i=0; i<(HEIGHT/8); i++) {
		for(j=0; j<WIDTH; j++) {
			nokia5110_writeData(pgm_read_byte(&(splash[i*WIDTH+j])));
		}
	}
}

void nokia5110_writeChar(char ch) {
	nokia5110_writeData(0x00);
	
	for(j=0; j<5; j++) {
		nokia5110_writeData(pgm_read_byte(&(smallFont [(ch-32)*5 + j] )));
	}

	nokia5110_writeData(0x00);
}

/* Starts writing a character, but cuts out early... unless you wanted width>=7... then you're just being weird */
void nokia5110_write_char_beginning(char ch, uint8_t width) {
	if (width > 0) {
		nokia5110_writeData(0x00);
	}
	
	for(j=0; j<5; j++) {
		if (width > 1+j) {
			nokia5110_writeData(pgm_read_byte(&(smallFont [(ch-32)*5 + j] )));
		} else {
			break;
		}
	}
	if (width > 6) {
		nokia5110_writeData(0x00);
	}
}

/* Starts writing a character from it's middle, and continues to the end... unless you wanted width>=7... then you're just being weird */
void nokia5110_write_char_end(char ch, uint8_t width) {
	if (width >= 7) {
		nokia5110_writeData(0x00);
	}
	
	for(j=0; j<5; j++) {
		if (width >= 6-j) {
			nokia5110_writeData(pgm_read_byte(&(smallFont [(ch-32)*5 + j] )));
		}
	}
	if (width >= 1) {
		nokia5110_writeData(0x00);
	}
}

void nokia5110_writeString(const char *string) {
	while (*string) {
		nokia5110_writeChar(*string++);
	}
}

/* Cuts off the string at 12 characters */
void nokia5110_writeString_C(const char *string) {
	i = 0;
	while (*string && i < 12) {
		i ++;
		nokia5110_writeChar(*string++);
	}
}

/* Writes a line basically... and allows for partial characters to be printed... handy for smooth scrolling text */
void nokia5110_writeString_L(const char *string, uint8_t px_offset) {
	
	nokia5110_write_char_end(*string++, 7-px_offset);
	i = 0;
	while (*string && i < 11) {
		i ++;
		nokia5110_writeChar(*string++);
	}
	if (*string) {
		nokia5110_write_char_beginning(*string++, px_offset);
	}
}


#ifdef NOKIA4117_USING_LARGE_FONT
void nokia5110_writeChar_megaFont(char ch) {
	switch(ch) {
		case '.': ch = 10; break;
		case '+': ch = 11; break;
		case '-': ch = 12; break;
		case ':': ch = 13; break;
		case ' ': ch = 14; break;
		default:  ch &= 0x0f; break;
	}
	
	// Prints a Large character in three horizontal passes
	for(i=0;i<3;i++)
	{
		for(j=0; j<12; j++) {
			nokia5110_writeData(pgm_read_byte(&(number[(uint8_t)ch][i][j])));
		}
		if (i<3) {
			nokia5110_gotoXY(cursor_col-12, cursor_row+1); // Think of this as the return on a typewriter
		}
	}
	//Move the cursor to the next position to facilitate the writing of another mega-character
	if (ch == 10 || ch == 13) {
		nokia5110_gotoXY(cursor_col+8, cursor_row-3); // periods and colons should be narrower
	} else {
		nokia5110_gotoXY(cursor_col+12, cursor_row-3);
	}
}

void nokia5110_writeString_megaFont(const char *string) {
	while (*string) {
		nokia5110_writeChar_megaFont(*string++);
	}
}
#endif
