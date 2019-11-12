   #ifndef __Nokia5110Lcd_H__        
#define __Nokia5110Lcd_H__    
#define lcdb P2 
#define uchar unsigned char 
#define uint unsigned int
sbit lcdrs=P0^7;
sbit lcdrw=P0^6;
sbit lcden=P0^5;

	  
extern void delay(int t); 
void lcdinit();
void writecmd(uchar cmd);
void writedata (uchar dat);
void wirtecursor(uchar x, uchar y);
void writeshow(uchar x,uchar y,uchar *str);

 #endif