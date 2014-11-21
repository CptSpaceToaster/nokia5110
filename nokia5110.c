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
uint16_t cursor_row = 0;
uint16_t cursor_col = 0;

void nokia5110_spi_init(uint8_t reg) {
	//SPI initialize
	//clock rate: 250000hz
	DDRLCD |= _BV(LCD_DC_PIN) | _BV(LCD_CE_PIN) | _BV(SPI_MOSI_PIN) | _BV(LCD_RST_PIN) | _BV(SPI_CLK_PIN);
	SPCR = reg; //setup SPI with a given register value... read the data sheet to see what you can do!
}

void nokia5110_power_on(void) {
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
}

void nokia5110_writeData(uint8_t data) {
	static uint8_t lcd_buffer[HEIGHT/8][WIDTH]; // may need to go
	
	/* Only clock out the data if it differs from the buffer */
	if (lcd_buffer[cursor_row][cursor_col] != data) {
		CLEAR_SCE_PIN;            // enable LCD
		SET_DC_PIN;               // set LCD in Data mode
		SPDR = Data;              // send data to display controller.
		while ( !(SPSR & 0x80) ); // wait until Tx register empty.
		SET_SCE_PIN;              // disable LCD
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
}

void nokia5110_writeCommand(uint8_t command) {
	CLEAR_SCE_PIN;	          // enable LCD
	CLEAR_DC_PIN;             // set LCD in command mode
	SPDR = command;           // send data to display controller.
	while (!(SPSR & 0x80));   // wait until Tx register empty.
	SET_SCE_PIN;              // disable LCD
}

void nokia5110_gotoXY(uint8_t column, uint8_t row) {
	nokia5110_writeCommand(0x80 | column);
	nokia5110_writeCommand(0x40 | row);

	cursor_col = column;
	cursor_row = row;
}

void nokia5110_clear(void) {
	int i,j;
	
	nokia5110_gotoXY(0,0);

	for(i=0; i<(HEIGHT/8); i++) {
		for(j=0; j<WIDTH; j++) {
			nokia5110_writeData(0x00);
		}
	}
}

void nokia5110_drawSplash(void) {
	int i,j;
	
	nokia5110_gotoXY(0,0);
	
	for(i=0; i<(HEIGHT/8); i++) {
		for(j=0; j<WIDTH; j++) {
			nokia5110_writeData(pgm_read_byte(&(splash[i*WIDTH+j]));
		}
	}
}

void nokia5110_writeChar(char character) {
	
}

void nokia5110_writeString_F(const char *string) {
	
}


#ifdef NOKIA4117_USING_LARGE_FONT
void nokia5110_writeChar_megaFont(char character) {
	
}

void nokia5110_writeString_megaFont(const char *string) {
	
}
#endif
