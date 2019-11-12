#include<reg51.h>
#include"1602.h"
#include"key_matrix.h"
 

#include<intrins.h>
#define   nop() _nop_()
#define uchar unsigned char
#define uint unsigned int
 
void delay(int t)
{
	uchar i;
	while (t--)
	{
		for (i=0;i<125;i++)
			{}
	}
}
 /*�����ݵĸߵ�λ����һ��*/
uchar daoxu(uchar dat)
{
	uchar temp;
   temp=dat;
	return temp;
}
void checkbusy()
{
/* 	uchar i=0x80;
	while(i&0x80)
	{
		lcdrs=0;
		lcdrw=1;
		lcden=1;
		i=daoxu(lcdb);
		lcden=0;
	} */
	delay(25);
}

 
void writecmd(uchar cmd)
{
	checkbusy();
	lcdrs=0;
	lcdrw=0;
	lcden=1;
	lcdb=daoxu(cmd);
	lcden=0;
	delay(10);	
}

void wirtecursor(uchar x, uchar y)
{
	 uchar addr;
	if(y==0)
	{
	        addr = 0x00 + x;
	}
	else
	{
	        addr = 0x40 + x;
	}
	writecmd(addr | 0x80);                //LCD����ָ�������0x80��ͷ
}


void writedata (uchar dat)
{
	//checkbusy();
	delay(5);
	lcdrs=1;
	lcdrw=0;
	delay(5);
	lcdb =dat;
	lcden=1;
	delay(5);
	lcden=0;
	//delay(5);
}
void writeshow(uchar x,uchar y,uchar *str)
{
        wirtecursor(x,y);                                        //LCD��ǰ���꺯��
        while(*str != '\0')                                        //�ַ���û�н���ʱִ��
        {
                writedata(*str++);                        //������STR�����ڶԵ�ַ�Ӽ�
                
        }
}
void lcdinit()
{
 
 
	//delay(2);
	writecmd(0x38);
	//delay(2);
	writecmd(0x38);
	//delay(2);
	writecmd(0x38);
	checkbusy();
	writecmd(0x08);
	checkbusy();
	writecmd(0x01);
	checkbusy();
	writecmd(0x06);
	checkbusy();
	writecmd(0x0c);
	checkbusy();
 
}  