/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <Init.h>
#include <Led.h>
#include <onewire.h>
#include <ds1302.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;//按键专用变量
unsigned char Key_Slow_Down;//按键减速专用变量
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};//数码管显示数据存放数组
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};//数码管小数点数据存放数组
unsigned char Seg_Pos;//数码管扫描专用变量
unsigned int Seg_Slow_Down;//数码管减速专用变量
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};//Led显示数据存放数组
unsigned char Seg_Disp_Mode;
float T = 23.5;
unsigned char T_p = 23;
unsigned char time[3] = {0x23,0x59,0x52};
bit time_Mode;
bit Relay_Mode;
unsigned int Timer_5000Ms;
bit Alarm_Flag;
unsigned char Timer_100Ms;
bit L3_Star_Flag;

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;//按键减速程序
	
	Key_Val = Key_Read();//实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val);//捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//捕捉按键上升沿
	Key_Old = Key_Val;//辅助扫描变量
	
	if(Seg_Disp_Mode == 1)
	{
		if(Key_Old == 17) time_Mode = 1;
		else time_Mode = 0;
	}
	
	switch(Key_Down)
	{
		case 12:
			if(++Seg_Disp_Mode == 3) Seg_Disp_Mode = 0;
		break;

		case 16:
			if(Seg_Disp_Mode == 2)
			{
				if(++T_p == 100) T_p = 99;	
			}
		break;
			
		case 17:
			if(Seg_Disp_Mode == 2)
			{
				if(--T_p == 9) T_p = 10;	
			}
		break;
			
		case 13:
			Relay_Mode ^= 1;
		break;
	}
	
	
}

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;//数码管减速程序
	
	T = read_t();
	Read_Rtc(time);
	
	switch(Seg_Disp_Mode)
	{
		case 0://温度显示
			Seg_Buf[0] = 11;
			Seg_Buf[1] = 1;
			Seg_Buf[5] = (unsigned char)T / 10 % 10;
			Seg_Buf[6] = (unsigned char)T / 1 % 10;
			Seg_Buf[7] = (unsigned int)(T*10) % 10;
			Seg_Point[6] = 1;
		break;
		
		case 1://时间显示
			Seg_Point[6] = 0;
			if(time_Mode == 0)
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 2;
				Seg_Buf[3] = time[0] / 16 % 16;
				Seg_Buf[4] = time[0] / 1 % 16;
				Seg_Buf[5] = 12;
				Seg_Buf[6] = time[1] / 16 % 16;
				Seg_Buf[7] = time[1] / 1 % 16;	
			}
			else
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 2;
				Seg_Buf[3] = time[1] / 16 % 16;
				Seg_Buf[4] = time[1] / 1 % 16;
				Seg_Buf[5] = 12;
				Seg_Buf[6] = time[2] / 16 % 16;
				Seg_Buf[7] = time[2] / 1 % 16;	
			}				
		break;
		
		case 2://参数设置
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 3;
				Seg_Buf[3] = 10;
				Seg_Buf[4] = 10;
				Seg_Buf[5] = 10;
				Seg_Buf[6] = T_p / 10 % 10;
				Seg_Buf[7] = T_p % 10;
		break;
	}
	
}

/*其他显示函数*/
void Led_Proc()
{
	/*继电器*/
	if(Relay_Mode == 0)//温度控制
	{
		if(T > T_p)
		{
			ucLed[0] = 1;
			Relay(1);		
		}
		else
		{
			ucLed[0] = 0;
			Relay(0);		
		}
	}
	else//时间控制
	{
		if((time[1] == 0)&&(time[2] == 0)) Alarm_Flag = 1;
	  if(Alarm_Flag == 1)
		{
			ucLed[2] = L3_Star_Flag?1:0;
			ucLed[0] = 1;
			Relay(1);		
		}
		else
		{
			ucLed[2] = 0;
			ucLed[0] = 0;
			Relay(0);	
		}
	}
	
	/*Led*/
	if(Relay_Mode == 0)
		ucLed[1] = 1;
	else
		ucLed[1] = 0;
}

/*定时器0中断初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//定时器0中断打开
	EA = 1;				//总中断打开
}

/*定时器0中断服务函数*/
void Timer0Server() interrupt 1
{
	if(++Key_Slow_Down == 10) Key_Slow_Down = 0;//按键减速专用
	if(++Seg_Slow_Down == 500) Seg_Slow_Down = 0;//数码管减速专用
	if(++Seg_Pos == 8) Seg_Pos = 0;//数码管显示专用
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,ucLed[Seg_Pos]);

	if(Alarm_Flag) Timer_5000Ms++;
	else Timer_5000Ms = 0;
	if(Timer_5000Ms == 5000)
		{
			Timer_5000Ms = 0;
			Alarm_Flag = 0;
		}
		
	if(++Timer_100Ms == 100)
	{
		Timer_100Ms = 0;
		L3_Star_Flag ^= 1;
	}
}

void Delay750ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;
	i = 35;
	j = 51;
	k = 182;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


/*Main*/
void main()
{
	read_t();
	Delay750ms();
	System_Init();
	Timer0Init();
	Set_Rtc(time);
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}