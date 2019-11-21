#include <reg51.h>
#include<intrins.h>
#include"1602.h"
#include"key_matrix.h"

#define uchar unsigned char
#define uint unsigned int

#define FOSC	11059200		//��Ƶ��
#define BAUD	9600			//������
#define SMOD	1			//�Ƿ����ʼӱ�
#if SMOD
	#define TC_VAL	(256-FOSC/16/12/BAUD)
#else
	#define TC_VAL	(256-FOSC/32/12/BAUD)
#endif

sfr AUXR = 0x8e;               //�����Ĵ���
sfr P0M0 = 0X94;
sfr P0M1 = 0x93; 

sbit set_nums=P3^5;
sbit k2=P3^4;
sbit k3=P3^3;
sbit k4=P3^2;

uint Sysec;
uchar ErrorCode;
uchar stringLenth;//�����յ����ַ�������,���β����յ�28�ֽ�  ��������38���ֽ�
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

char aciiCount; //�յ����ַ�������

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
void Timer0Init(void)		//50����@6.000MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TMOD |= 0x01;		//���ö�ʱ��ģʽ
	TL0 = 0x58;		//���ö�ʱ��ֵ
	TH0 = 0x9E;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
	ET0=1;
	EA=1;
}*/
/*
void UartInit(void)		//115200bps@11.0592MHz
{
	PCON &= 0x80;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xFD;		//�趨��ʱ��ֵ
	TH1 = 0xFD;		//�趨��ʱ����װֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
	ES=1;
	EA=1;
}	*/
/*
void UartInit(void)		//9600bps@11.0592MHz
{
	PCON = 0x80;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	TMOD = 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TH1 = TC_VAL;		//�趨��ʱ����װֵ
	TL1 = TH1;		//�趨��ʱ��ֵ
	//ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
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

/*********** ***�����ַ�(ASCII)����*** **********/
/*
void sendAscii(uchar *b)
{
	ES = 0;                  //�ش����ж�
	for (b; *b!='\0';b++)
	{
		SBUF = *b;
		while (TI!=1);   //�ȴ��������
		TI = 0;          //��������жϱ�־λ
	}
	ES = 1;                  //�������ж�
} */
void sendcmdme(short *b)
{
	ES = 0;                  //�ش����ж�
	for (b; *b!='\0';b++)
	{
		SBUF = *b;
		while (TI!=1);   //�ȴ��������
		TI = 0;          //��������жϱ�־λ
	}
	ES = 1;                  //�������ж�
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
	P0M1=0; //�������
	
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
	refractivity = 1.359;		  // ��ʼ��������
	key_count = 0;
	set_real_num_int = 0;
	set_real_num = 0;
   
	while (1)
	{
		//����δ֪�������ʣ���Ҫ��ʼ��
	   //����ⲿ���̵�����

	   //���Ȱ�������ʼ�������ʣ������������ʣ���ʼ�����̵�����
	   //�������룬��ֵ����
	   //��⵽ȷ�ϼ�������
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


		// ������ͨ������ѭ������������
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
			aciiCount=0;//����38���ַ����͹���
			wirtecursor(0,1);
			writedata('M');
			//sendAscii("$00022426&");//��ʼ��������
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
	ES=0;//�ر��ж�

	if (RI) {
		RI=0;
		receivedata[aciiCount]=SBUF;
		aciiCount++;
		
		if (aciiCount>stringLenth) //���ݸ���ģʽ��ȡ���ʳ��ȵ��ַ���
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
	TL0 = 0x58;		//���ö�ʱ��ֵ
	TH0 = 0x9E;		//���ö�ʱ��ֵ
	i++;
	if (i==20) {
		i=0;
		Sysec++;
	}
}	 
