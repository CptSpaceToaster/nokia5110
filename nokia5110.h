/*
 * nokia5110.h
 *
 * Created: 11/20/2014 8:15:16 PM
 * Author: CaptainSpaceToaster
 */ 
#ifndef _NOKIA_5110_H_
#define _NOKIA_5110_H_
#define NOKIA4117_USING_LARGE_FONT // comment out to save space, and remove large font functionality

/* Alias to point to the register that implements SPI */
#define DDRLCD                     DDRB
#define PORTLCD                    PORTB

/*  Pin placement in the registers - Change accordingly */
#define LCD_DC_PIN                 0
#define LCD_CE_PIN                 2
#define SPI_MOSI_PIN               3
#define LCD_RST_PIN                4
#define SPI_CLK_PIN                5

#define SET_DC_PIN                 PORTLCD |= _BV(LCD_DC_PIN)  
#define CLEAR_DC_PIN               PORTLCD &= ~_BV(LCD_DC_PIN)
#define SET_SCE_PIN                PORTLCD |= _BV(LCD_CE_PIN)
#define CLEAR_SCE_PIN              PORTLCD &= ~_BV(LCD_CE_PIN)
#define SET_RST_PIN                PORTLCD |= _BV(LCD_RST_PIN)
#define CLEAR_RST_PIN              PORTLCD &= ~_BV(LCD_RST_PIN)

void nokia5110_init ( void );
void nokia5110_spi_init( unsigned char reg );
void nokia5110_writeData ( unsigned char data );
void nokia5110_writeCommand ( unsigned char command );
void nokia5110_gotoXY ( unsigned char x, unsigned char y );
void nokia5110_clear ( void );

void nokia5110_writeChar ( unsigned char character );
void nokia5110_drawSplash (void );
void nokia5110_writeString_F ( const char *string);

#ifdef NOKIA4117_USING_LARGE_FONT
void nokia5110_writeChar_megaFont ( unsigned char character );
void nokia5110_writeString_megaFont ( char *string );
#endif

#endif  //NOKIA_5110_H_
