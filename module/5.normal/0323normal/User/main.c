/*头文件声明区域*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include <Seg.h>
#include <Led.h>
#include <Key.h>

/*变量声明区域*/
unsigned char Key_Slow_Down;
unsigned int Seg_Slow_Down;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10}; 
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Pos;
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;
	
	Key_Val = Key_Read();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
}

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;
	
}

/*其他显示函数*/
void Led_Proc()
{
	
}

/*定时器0初始化函数*/
void Timer0Init(void)		//1毫秒@12MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x20;				//设置定时初始值
	TH0 = 0xD1;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;
	EA = 1;
}

/*定时器0中断服务函数*/
void Timer0Server() interrupt 1
{
	if(++Key_Slow_Down == 10) Key_Slow_Down = 0;
	if(++Seg_Slow_Down == 500) Seg_Slow_Down = 0;
	if(++Seg_Pos == 8) Seg_Pos = 0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	
}

/*Main*/
void main()
{
	System_Init();
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}