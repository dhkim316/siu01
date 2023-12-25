#ifndef LCD_H
#define LCD_H

#define lcd_type 	  2           // 0=5x7, 1=5x10, 2=2 lines 
#define lcd_line_two 0x40    // LCD RAM address for the second line 
#define LCD_SET_CGRAM_ADDR 0x40

typedef unsigned char BYTE;

void lcd_init();
void lcd_gotoxy( BYTE x, BYTE y);
void lcd_putc( char c);
void lcd_puts(char *s);
void printf_lcd(char x, char y, char* szFormat, ...);
void lput(uint8_t x, uint8_t y, char c);
void cursor_xy(uint8_t x, uint8_t y, uint8_t blink);
void lcd_CreateChar(uint8_t location, uint8_t charmap[]);
void viUnitDisplay(void);

#endif
