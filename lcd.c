#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "em_gpio.h"
#include "sl_udelay.h"
#include "lcd.h"

//user font
const char fch0[8] = {0b00000,0b00000,0b00000,0b10111,0b10101,0b10101,0b10101,0b10111}; //10 
const char fch1[8] = {0b00110,0b00001,0b11010,0b00001,0b00110,0b00000,0b00000,0b00000}; //exp-3
const char fch2[8] = {0b00000,0b00000,0b00000,0b11010,0b10101,0b10101,0b10001,0b10001}; //'m'
const char fch3[8] = {0b00010,0b00010,0b11010,0b00010,0b00010,0b00000,0b00000,0b00000}; //exp-1
const char fch4[8] = {0b00000,0b00000,0b00001,0b00010,0b00100,0b01000,0b10000,0b00000}; //'/'

void output_d(uint8_t val) 
{
    if(val & 0x01) GPIO_PinOutSet(gpioPortC, 0);
    else	       GPIO_PinOutClear(gpioPortC, 0);
    if(val & 0x02) GPIO_PinOutSet(gpioPortD, 0);
    else	       GPIO_PinOutClear(gpioPortD, 0);
    if(val & 0x04) GPIO_PinOutSet(gpioPortD, 1);
    else	       GPIO_PinOutClear(gpioPortD, 1);
    if(val & 0x08) GPIO_PinOutSet(gpioPortD, 2);
    else	       GPIO_PinOutClear(gpioPortD, 2);
}

void lcd_send_nibble( BYTE n ) 
{ 
   output_d(n); 
   sl_udelay_wait(1);   
   GPIO_PinOutSet(gpioPortC, 1);
   sl_udelay_wait(1);   
   GPIO_PinOutClear(gpioPortC, 1);
   sl_udelay_wait(1);   
} 

void lcd_send_byte( BYTE A0, BYTE n ) 
{    
    // A0:  0=instruction, 1=Data 
    if (A0==0){ GPIO_PinOutClear(gpioPortC, 3);} //0=Instruction/command and 1=Data 
    if (A0==1){GPIO_PinOutSet(gpioPortC, 3);}    //0=Instruction/command and 1=Data 
    sl_udelay_wait(10);   
    GPIO_PinOutClear(gpioPortC, 2); 
    sl_udelay_wait(10);   
    GPIO_PinOutClear(gpioPortC, 1); 
    lcd_send_nibble(n >> 4); 
    lcd_send_nibble(n & 0xf); 
} 

// location = 0..7
void lcd_CreateChar(uint8_t location, uint8_t charmap[])
{
   location &= 0x07;
   lcd_send_byte(0, LCD_SET_CGRAM_ADDR|(location<<3));
   for(int i=0; i<8; i++) {
      lcd_send_byte(1,charmap[i]);
   }        
}

void viUnitDisplay(void)
{  //10^-3m^-1 .... 10^-3/m
   lcd_putc(0);  //10
   lcd_putc(1);  //exp-3
   lcd_putc(2);  //m
   lcd_putc(3);  //exp-1
   lcd_putc(' ');
   lcd_putc(' ');
   lcd_putc(' ');
   lcd_putc(' ');
   lcd_putc(' ');
}

void lcd_init() 
{ 
   BYTE const LCD_INIT_STRING[4] = {0x20 | (lcd_type << 2), 0xc, 1, 6}; 
   // These bytes need to be sent to the LCD for initialization. 
   BYTE i; 
   sl_udelay_wait(20*1000); //wait 20ms for lcd power on reset
	 
   GPIO_PinModeSet(gpioPortC, 3, gpioModePushPull, 0); //LCD RS
   GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 0); //LCD EN
   GPIO_PinModeSet(gpioPortC, 2, gpioModePushPull, 0); //LCD RW

   GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1); //LCD_D4
   GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 1); //LCD_D5
   GPIO_PinModeSet(gpioPortD, 1, gpioModePushPull, 1); //LCD_D6
   GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 1); //LCD_D7
	 
   sl_udelay_wait(15*1000); //wait 15ms

   for(i=1;i<=3;++i) { 
    lcd_send_nibble(3); 
    sl_udelay_wait(20*1000); //wait 20ms
   } 
	 
   lcd_send_nibble(2); 
	 
   for(i=0; i<=3; ++i){ 
      lcd_send_byte(0,LCD_INIT_STRING[i]); 
   } 

   sl_udelay_wait(20*1000); //wait 20ms for setup complete
   lcd_CreateChar(0, fch0); //create font 10
   lcd_CreateChar(1, fch1); //create font exp-3
   lcd_CreateChar(2, fch2); //create font m
   lcd_CreateChar(3, fch3); //create font exp-1
   lcd_CreateChar(4, fch4); //create font /
} 

void lcd_gotoxy( BYTE x, BYTE y) 
{ 
   BYTE Data; 
   if(y!=1) 
      Data=lcd_line_two; 
   else 
      Data=0; 
   Data+=x-1; 
   lcd_send_byte(0,0x80|Data); 
} 
    
void lcd_putc( char c) 
{ 
   switch (c) { 
      case '\f'   : 
         lcd_send_byte(0,1); 
         sl_udelay_wait(2*1000); //wait 2ms
         break; 
      case '\n'   : 
         lcd_gotoxy(1,2);        
         break; 
      case '\r'   : 
         lcd_gotoxy(1,1);        
         break; 
      case '\b'   : 
         lcd_send_byte(0,0x10);  
         break; 
      case '/'   : 
         lcd_send_byte(1,4);  
         break; 
      default     : 
         lcd_send_byte(1,c);      
         break; 
   } 
} 

void cursor_xy(uint8_t x, uint8_t y, uint8_t blink)
{
	lcd_gotoxy(x,y);
	if(blink) lcd_send_byte(0,0x0f);	//cursor on & blink
	else	 lcd_send_byte(0,0x0c);	   //cursor off
}

void lput(uint8_t x, uint8_t y, char c) {
   lcd_gotoxy(x,y);
   lcd_putc(c);
}

void lcd_puts(char *s)
{
	while(*s)
	{
		lcd_putc(*s);
		s++;
	}
}    

void printf_lcd(char x, char y, char* szFormat, ...)
{
    char szBuf[32] = {0, };
 
    va_list lpStart;
    va_start(lpStart, szFormat);
 
    vsprintf(szBuf, szFormat, lpStart);
 
    lcd_gotoxy( x,y);
    lcd_puts(szBuf);
 
    va_end(lpStart);
}
