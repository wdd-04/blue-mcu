/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <Init.h>
#include <Led.h>
#include <iic.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;//按键专用变量
unsigned char Key_Slow_Down;//按键减速专用变量
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};//数码管显示数据存放数组
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};//数码管小数点数据存放数组
unsigned char Seg_Pos;//数码管扫描专用变量
unsigned int Seg_Slow_Down;//数码管减速专用变量
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};//Led显示数据存放数组
unsigned int Timer_1000Ms;
unsigned int Freq;
bit Seg_Disp_Mode;//数码管显示模式 0-频率显示 1-电压显示
float V;
float V_Output;
bit Output_Mode;
bit Seg_Flag = 1;
bit Led_Flag = 1;

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;//按键减速程序
	
	Key_Val = Key_Read();//实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val);//捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//捕捉按键上升沿
	Key_Old = Key_Val;//辅助扫描变量
	
	switch(Key_Down)
	{
		case 4:
			Seg_Disp_Mode ^= 1;
		break;
		
		case 5:
			Output_Mode ^= 1;
		break;
		
		case 6:
			Led_Flag ^= 1;
		break;
		
		case 7:
			Seg_Flag ^= 1;
		break;
	}
}

/*信息处理函数*/
void Seg_Proc()
{
	unsigned char i = 3;
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;//数码管减速程序
	
	V = Ad_Read(0x43)/51.0;
	if(Output_Mode == 0)
		V_Output = 2;
	else
		V_Output = V;
	
	Seg_Point[5] = Seg_Disp_Mode;
	if(Seg_Disp_Mode == 0)//频率显示
	{
		Seg_Buf[0] = 11;
		Seg_Buf[3] = Freq / 10000 % 10;
		Seg_Buf[4] = Freq / 1000 % 10;
		Seg_Buf[5] = Freq / 100 % 10;
		Seg_Buf[6] = Freq / 10 % 10;
		Seg_Buf[7] = Freq / 1 % 10;
		while(Seg_Buf[i] == 0)
		{
			Seg_Buf[i] = 10;
			if(++i == 7) break;
		}
	}
	else
	{
		Seg_Buf[0] = 12;
		Seg_Buf[3] = 10;
		Seg_Buf[4] = 10;	
		Seg_Buf[5] = (unsigned char)V;
		Seg_Buf[6] = (unsigned int)(V*100) / 10 % 10;
		Seg_Buf[7] = (unsigned int)(V*100) / 1 % 10;
	}
}

/*其他显示函数*/
void Led_Proc()
{
	unsigned char i;
	Da_Write(V_Output*51);
	for(i=0;i<2;i++)
		ucLed[i] = (i == Seg_Disp_Mode);
	ucLed[2] = ((V >= 1.5 && V < 2.5)||(V >= 3.5));
	ucLed[3] = ((Freq >= 1000 && Freq < 5000)||(Freq >= 10000));
	ucLed[5] = Output_Mode;
}

/*定时器0计数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计数
}

/*定时器1中断初始化函数*/
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;
	EA = 1;
}


/*定时器1中断服务函数*/
void Timer1Server() interrupt 3
{
	if(++Key_Slow_Down == 10) Key_Slow_Down = 0;//按键减速专用
	if(++Seg_Slow_Down == 500) Seg_Slow_Down = 0;//数码管减速专用
	if(++Seg_Pos == 8) Seg_Pos = 0;//数码管显示专用
	
	if(Seg_Flag == 1)
		Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	else
		Seg_Disp(Seg_Pos,10,0);		

	if(Led_Flag == 1)	
		Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	else
		Led_Disp(Seg_Pos,0);
	
	if(++Timer_1000Ms == 1000)//获取频率值
	{
		Timer_1000Ms = 0;
		Freq = TH0 << 8 | TL0;
		TH0 = 0;
		TL0 = 0;
	}	
}

/*Main*/
void main()
{
	System_Init();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}
