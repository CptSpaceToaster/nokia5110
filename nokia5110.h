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

void nokia5110_spi_init(uint8_t reg);
void nokia5110_power_on(void);
void nokia5110_writeData(uint8_t data);
void nokia5110_writeCommand(uint8_t command);
void nokia5110_gotoXY(uint8_t column, uint8_t row);
void nokia5110_clear(void);
void nokia5110_drawSplash(void);
void nokia5110_writeChar(char ch);
void nokia5110_write_char_beginning(char ch, uint8_t width);
void nokia5110_write_char_end(char ch, uint8_t width);
void nokia5110_writeString(const char *string);
void nokia5110_writeString_C(const char *string);
void nokia5110_writeString_L(const char *string, uint8_t px_offset);

#ifdef NOKIA4117_USING_LARGE_FONT
void nokia5110_writeChar_megaFont(char ch);
void nokia5110_writeString_megaFont(const char *string);
#endif

#endif  //NOKIA_5110_H_
