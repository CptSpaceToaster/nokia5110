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

void nokia5110_spi_init( unsigned char reg ) {
	
}

void nokia5110_init ( void ) {
	
}

void nokia5110_writeData ( unsigned char data ) {
	
}

void nokia5110_writeCommand ( unsigned char command ) {
	
}

void nokia5110_gotoXY ( unsigned char x, unsigned char y ) {
	
}

void nokia5110_clear ( void ) {
	
}


void nokia5110_writeChar ( unsigned char character ) {
	
}

void nokia5110_drawSplash (void ) {
	
}

void nokia5110_writeString_F ( const char *string) {
	
}


#ifdef NOKIA4117_USING_LARGE_FONT
void nokia5110_writeChar_megaFont ( unsigned char character ) {
	
}

void nokia5110_writeString_megaFont ( char *string ) {
	
}
#endif
