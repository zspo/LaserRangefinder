#include <reg51.h>
#include<intrins.h>
#include"1602.h"
#include"key_matrix.h"

#define uchar unsigned char
#define uint unsigned int

#define FOSC	11059200		//振荡频率
#define BAUD	9600			//波特率
#define SMOD	1			//是否波特率加倍
#if SMOD
	#define TC_VAL	(256-FOSC/16/12/BAUD)
#else
	#define TC_VAL	(256-FOSC/32/12/BAUD)
#endif

sfr AUXR = 0x8e;               //辅助寄存器
sfr P0M0 = 0X94;
sfr P0M1 = 0x93; 

sbit set_nums=P3^5;
sbit k2=P3^4;
sbit k3=P3^3;
sbit k4=P3^2;

uint Sysec;
uchar ErrorCode;
uchar stringLenth;//串口收到的字符串长度,单次测量收到28字节  连续测量38个字节
uchar i;
uchar temp_list[4];
char temp_count;
unsigned char temp;
float refractivity;
float set_real_num;
float measure_num;
float measure_num_tmp;
float adjust_num;
float tmp_re;
int set_real_num_int;
int measure_num_int;
int adjust_num_int;

int key_count;

char aciiCount; //收到的字符数计算

short receivedata[11]={0};
uchar uartSbuf[11];
uchar tmpSbuf[4];

//short laserOn[5] = {0x80,0x06,0x05,0x01,0x74};
short singleTest[4] ={0x80,0x06,0x02,0x78};	
//short shutdown[4] = {0x80,0x04,0x02,0x7A};
//short multipleTest[4] ={0x80,0x06,0x03,0x77}; 

bit recOver=0, recStart=0,tick=0;
uchar contrast=0xb0;
uchar eepAddr=0x00;

/*
void Timer0Init(void)		//50毫秒@6.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x58;		//设置定时初值
	TH0 = 0x9E;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}*/
/*
void UartInit(void)		//115200bps@11.0592MHz
{
	PCON &= 0x80;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;		//设定定时初值
	TH1 = 0xFD;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
	ES=1;
	EA=1;
}	*/
/*
void UartInit(void)		//9600bps@11.0592MHz
{
	PCON = 0x80;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	TMOD = 0x20;		//设定定时器1为8位自动重装方式
	TH1 = TC_VAL;		//设定定时器重装值
	TL1 = TH1;		//设定定时初值
	//ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
	ES=1;
	EA=1;
} */
void UartInit(void)		//9600bps@11.0592MHz
{
	SCON = 0x50; //2015-05-04	
	TMOD = 0x20;	
	TH1 = 0XFD; // 9600BPS 2015-05-04	
	TL1 = 0XFD;	
	TR1 = 1; // start TImer0	
	ES = 1; // Enable serial interrupt	
	EA = 1; // Enable global interrupt
}

/*********** ***发送字符(ASCII)函数*** **********/
/*
void sendAscii(uchar *b)
{
	ES = 0;                  //关串口中断
	for (b; *b!='\0';b++)
	{
		SBUF = *b;
		while (TI!=1);   //等待发送完成
		TI = 0;          //清除发送中断标志位
	}
	ES = 1;                  //开串口中断
} */
void sendcmdme(short *b)
{
	ES = 0;                  //关串口中断
	for (b; *b!='\0';b++)
	{
		SBUF = *b;
		while (TI!=1);   //等待发送完成
		TI = 0;          //清除发送中断标志位
	}
	ES = 1;                  //开串口中断
}
/*
void clearUartSbuf()
{
	uchar i;
	for (i=0;i<aciiCount;i++)
	{
		uartSbuf[i]=0;
	}
}*/
/*
void setIoMode()
{
	P0M0=0xff;
	P0M1=0; //推挽输出
	
}*/

uchar GetASCII(short ch)
{
	/*if(x>0x0f)
	{
		return 0;
	}
	if(x>9)
	{
		return ('A'+x-10);
	}
	return ('0'+x);*/
	if (ch == 0x2e) {
		return '.';
	}
	if (ch == 0x30) {
		return '0';
	}
	if (ch == 0x31) {
		return '1';
	}
	if (ch == 0x32) {
		return '2';
	}
	if (ch == 0x33) {
		return '3';
	}
	if (ch == 0x34) {
		return '4';
	}
	if (ch == 0x35) {
		return '5';
	}
	if (ch == 0x36) {
		return '6';
	}
	if (ch == 0x37) {
		return '7';
	}
	if (ch == 0x38) {
		return '8';
	}
	if (ch == 0x39) {
		return '9';
	}
	return '0';
}

void main()
{

	UartInit();
    lcdinit();
	//sendMulCheck();
	//writecmd(0x80);
	//writedata('m');	
	writeshow(0,0,"Fiber Length:");
	//wirtecursor(0,1);
	//writedata('m');
	refractivity = 1.359;		  // 初始化折射率
	key_count = 0;
	set_real_num_int = 0;
	set_real_num = 0;
   
	while (1)
	{
		//对于未知的折射率，需要初始化
	   //检测外部键盘的输入

	   //首先按键，初始化折射率，请输入折射率，开始检测键盘的输入
	   //键盘输入，数值保存
	   //检测到确认键，结束
	   if (set_nums==0) {
			lcdinit();
			writeshow(0,0,"PLE SET: 'D' ACK");
			temp_count = 0;
			temp = kbscan();
			while (temp!='D') {
				temp = kbscan();		
				if((temp!=0) && (temp!='D')) {
					wirtecursor(temp_count,1);
					writedata(temp);
					temp_list[temp_count] = temp;
					temp_count++;
				}
				if (temp=='D') {
					lcdinit();
					writeshow(0,1,"SET OK !");
				} 
			}
			set_real_num = (temp_list[0]-'0')*1 + (temp_list[2]-'0')*0.1 + + (temp_list[3]-'0') * 0.01;

			//set_real_num_int = (int)(set_real_num*100);

		}


		// 以下是通过按键循环设置折射率
		if (k4==0) {
			lcdinit();
			writeshow(0,0,"SELECT RF");
			if (key_count == 4) {
				key_count = 0;
			}
			if (key_count == 0) {
				//writeshow(0,0,"WL: 850nm");
				writeshow(0,1,"RF: 1.477");
				refractivity = 1.354;
			}
			if (key_count == 1) {
				//writeshow(0,0,"WL: 1300nm");
				writeshow(0,1,"RF: 1.4719");
				refractivity = 1.359;
			}
			if (key_count == 2) {
				//writeshow(0,0,"WL: 1310nm");
				writeshow(0,1,"RF: 1.4680");
				refractivity = 1.362;
			}
			if (key_count == 3) {
				//writeshow(0,0,"WL: 1550nm");
				writeshow(0,1,"RF: 1.4685");
				refractivity = 1.360;
			}
	   		key_count++;
		}


		if (k3==0){
			lcdinit();
			writeshow(0,0,"Reset: Wait.");
			writeshow(0,1,"-------");
			//sendcmdme(shutdown);
			delay(300);
			//sendcmdme(laserOn);
			lcdinit();
			writeshow(0,0,"Reset OK.");
			delay(300);
			writeshow(0,0,"Fiber Length:");
		}


		if (k2==0){
			lcdinit();
			writeshow(0,0,"Fiber Length:");
			stringLenth=38;
			aciiCount=0;//接收38个字符串就够了
			wirtecursor(0,1);
			writedata('M');
			//sendAscii("$00022426&");//开始连续测量
			sendcmdme(singleTest);

			for(i = 0; i<11;i++)
		    {
				uartSbuf[i] = GetASCII(receivedata[i]);
		    }
			
			measure_num = (uartSbuf[4]-'0') * 10 + (uartSbuf[5]-'0') * 1 + (uartSbuf[7]-'0') * 0.1 + (uartSbuf[8]-'0') * 0.01;		 //		0.779985
			// 1.359
			if (set_real_num != 0) {
			    measure_num_tmp = measure_num *  refractivity;
				//measure_num_int = (int)(measure_num * 100);
				tmp_re = set_real_num / measure_num_tmp;					// 1.00 / 0.7799
				refractivity = refractivity * tmp_re;
			}
			set_real_num = 0;

			adjust_num = measure_num * refractivity;
			adjust_num_int = (int)(adjust_num*100);
						
			tmpSbuf[0] = ((adjust_num_int/1000) % 10) + '0';
			tmpSbuf[1] = ((adjust_num_int/100) % 10)  + '0';
			tmpSbuf[2] = ((adjust_num_int/10) % 10)  + '0';
			tmpSbuf[3] = (adjust_num_int% 10)  + '0' ;
			
			wirtecursor(2,1);
			writedata(tmpSbuf[0]);
			writedata(tmpSbuf[1]);
			writedata('.');
			//writedata(tmpSbuf[2]);
			writedata(tmpSbuf[2]);
			writedata(tmpSbuf[3]);
			writedata('m');

		}

	}

}

void uartRec()interrupt 4{
	ES=0;//关闭中断

	if (RI) {
		RI=0;
		receivedata[aciiCount]=SBUF;
		aciiCount++;
		
		if (aciiCount>stringLenth) //根据各个模式截取合适长度的字符串
		{
			aciiCount=0;
		}  
	}
	if (TI) {
		TI=0;
	}
	ES=1;
}

void t0() interrupt 1
{
	static char i;
	TL0 = 0x58;		//设置定时初值
	TH0 = 0x9E;		//设置定时初值
	i++;
	if (i==20) {
		i=0;
		Sysec++;
	}
}	 
