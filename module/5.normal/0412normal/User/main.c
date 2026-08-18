/*头文件*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include <Led.h>
#include <Seg.h>
#include <Key.h>
#include <string.h>
#include <Uart.h>

/*变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Key_Slow_Down;
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Pos;
idata unsigned int Seg_Slow_Down;
idata unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};
idata unsigned char Uart_Recv[10];//串口接收数据储存数组 默认10个字节 若接收数据较长 可更改最大字节数
idata unsigned char Uart_Recv_Index;//串口接收数组指针
idata unsigned char Uart_Recv_Tick;     // 串口接收时间标志
idata unsigned char Uart_Rx_Flag;

/*按键*/
void Key_Proc()
{
	if(Key_Slow_Down <10) return;
	Key_Slow_Down = 0;
	
  Key_Val = Key_Read();//实时读取键码值
  Key_Down = Key_Val & (Key_Old ^ Key_Val);//捕捉按键下降沿
  Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//捕捉按键上降沿
  Key_Old = Key_Val;//辅助扫描变量
}

/*数码管*/
void Seg_Proc()
{
	if(Seg_Slow_Down <500) return;
	Seg_Slow_Down = 0;

}
/*其他*/
void Led_Proc()
{

}

/*串口处理函数*/
void Uart_Proc()
{
	if(Uart_Recv_Index == 0) return;
	if(Uart_Recv_Tick >= 10)
	{
		Uart_Rx_Flag = 0;
		Uart_Recv_Tick = 0;
		
		memset(Uart_Recv,0,Uart_Recv_Index);
		Uart_Recv_Index = 0;
	}
}

/*串口1中断服务*/
void Uart1Server() interrupt 4
{
	if(RI == 1)
	{
		Uart_Rx_Flag = 1;
		Uart_Recv_Tick = 0;
		Uart_Recv[Uart_Recv_Index] = SBUF;
		Uart_Recv_Index++;
		RI = 0;
		if(Uart_Recv_Index>10)
		{
			Uart_Recv_Index = 0;
			memset(Uart_Recv,0,10);		
		}
		
	}
}

/*定时器0初始化*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;
	EA = 1;
}

/*定时器0中断服务*/
void Timer0Server() interrupt 1
{
	Key_Slow_Down++;
	Seg_Slow_Down++;
	
	if(++Seg_Pos == 8) Seg_Pos = 0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(ucLed);
}

/*Main*/
void main()
{
	Sys_Init();
	Timer0Init();
	Uart1_Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}