#include"reg51.h"
#include"1602.h"
#include"key_matrix.h"

char code tab[4][4]={	
						{'1','4','7','.'},
              			{'2','5','8','0'},
              			{'3','6','9','*'},
              			{'A','B','C','D'}
					};       //0µ½FµÄ16¸ö¼üÖ²

void delay_shiyan(unsigned char a)
	{
	unsigned char i;
	while(a--)
		for(i=100;i>0;i--)
			;
	}

char kbscan()          //¼üÅÌÉ¨Ãè
	{
 	unsigned char hang,lie,key;
	if(P1!=0x0f)
	delay_shiyan(5);
	if(P1!=0x0f)
		{
		switch(P1&0x0f)
				{
				case 0x0e:lie=0;break;
				case 0x0d:lie=1;break;
				case 0x0b:lie=2;break;
				case 7:lie=3;break;
				}
		P1=0xf0;
		P1=0xf0;
		switch(P1&0xf0)
				{
				case 0xe0:hang=0;break;
				case 0xd0:hang=1;break;
				case 0xb0:hang=2;break;
				case 0x70:hang=3;break;
				}
		P1=0x0f;
		while(P1!=0x0f);
		key=tab[hang][lie];
		}
	else
		key=0;
	return (key);
	}





