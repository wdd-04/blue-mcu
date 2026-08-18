/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include <Key.h>
#include <Seg.h>
#include <Led.h>
#include <iic.h>
#include <ds1302.h>

/*变量声明区域*/
unsigned char Key_Slow_Down;
unsigned int Seg_Slow_Down;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Disp_Mode;//0-频率 1-参数 2-时间 3-回显
unsigned int Timer_1000Ms;
unsigned int Freq;
unsigned int Freq_max;
int Freq_last;
unsigned int PF = 2000;
int P_correct = 0;
bit Para_Mode;//0-超限参数 1-校准值
bit Back_Mode;//0-频率回显 1-时间回显
unsigned char ucRtc[3] = {0x13,0x03,0x05};
unsigned char occur[3];
float V;
unsigned char Timer_200Ms;
unsigned char Led_Flag;

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;
	
	Key_Val = Key_Read();
	Key_Down = Key_Val&(Key_Val^Key_Old);
	Key_Up = ~Key_Val&(Key_Val^Key_Old);
	Key_Old = Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Disp_Mode == 4)
				Seg_Disp_Mode = 0;
		break;
			
		case 5:
			if(Seg_Disp_Mode == 1)
				Para_Mode ^= 1;
			if(Seg_Disp_Mode == 3)
				Back_Mode ^= 1;
		break;
			
		case 8:
			if(Para_Mode == 0)
			{
				PF += 1000;
				if(PF == 10000) PF = 1000;
			}
			if(Para_Mode == 1)
			{
				P_correct += 100;
				if(P_correct == 1000) P_correct = -900;
			}
		break;
			
		case 9:
			if(Para_Mode == 0)
			{
				PF -= 1000;
				if(PF == 0) PF = 9000;
			}
			if(Para_Mode == 1)
			{
				P_correct -= 100;
				if(P_correct == -1000) P_correct = 900;
			}
		break;
	}
}

/*信息处理函数*/
void Seg_Proc()
{
	unsigned char i = 3;
	unsigned char j = 4;
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;
	
	Read_Rtc(ucRtc);
	Freq_last = Freq + P_correct;
	if(Freq_max <= Freq_last) 
	{
		Freq_max = Freq_last;
		occur[0] = ucRtc[0];
		occur[1] = ucRtc[1];
		occur[2] = ucRtc[2];
	}
	
	switch(Seg_Disp_Mode)
	{
		case 0://频率
			if(Freq_last >= 0)
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 10;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = Freq_last / 10000 % 10;
				Seg_Buf[4] = Freq_last / 1000 % 10;
				Seg_Buf[5] = Freq_last / 100 % 10;
				Seg_Buf[6] = Freq_last / 10 % 10;
				Seg_Buf[7] = Freq_last / 1 % 10;		
			}
			if(Freq_last < 0)
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 10;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = 10;
				Seg_Buf[4] = 10;
				Seg_Buf[5] = 10;
				Seg_Buf[6] = 15;
				Seg_Buf[7] = 15;	
				Da_Write(0);
			}
			while(Seg_Buf[i] == 0)
			{
				Seg_Buf[i] = 10;
				if(++i == 7) break;
			}
			Para_Mode = 0;
		break;
		
		case 1://参数
			if(Para_Mode == 0)//超限参数
			{			
				Seg_Buf[0] = 12;
				Seg_Buf[1] = 1;
				Seg_Buf[1] = 10;
				Seg_Buf[1] = 10;
				Seg_Buf[4] = PF / 1000 % 10;
				Seg_Buf[5] = PF / 100 % 10;
				Seg_Buf[6] = PF / 10 % 10;
				Seg_Buf[7] = PF / 1 % 10;
			}
			else//校准参数
			{
				if(P_correct >= 0)
				{
					Seg_Buf[0] = 12;
					Seg_Buf[1] = 2;
					Seg_Buf[4] = P_correct / 1000 % 10;
					Seg_Buf[5] = P_correct / 100 % 10;
					Seg_Buf[6] = P_correct / 10 % 10;
					Seg_Buf[7] = P_correct / 1 % 10;		
				}
				else
				{
					Seg_Buf[0] = 12;
					Seg_Buf[1] = 2;
					Seg_Buf[4] = 16;
					Seg_Buf[5] = P_correct*(-1) / 100 % 10;
					Seg_Buf[6] = P_correct*(-1) / 10 % 10;
					Seg_Buf[7] = P_correct*(-1) / 1 % 10;								
				}
				while(Seg_Buf[j] == 0)
				{
					Seg_Buf[j] = 10;
					if(++j == 7) break;
				}

			}
		
		break;
		
		case 2://时间
			Seg_Buf[0] = ucRtc[0] / 16 % 16;
			Seg_Buf[1] = ucRtc[0] / 1 % 16;
			Seg_Buf[2] = 16;
			Seg_Buf[3] = ucRtc[1] / 16 % 16;
			Seg_Buf[4] = ucRtc[1] / 1 % 16;
			Seg_Buf[5] = 16;
			Seg_Buf[6] = ucRtc[2] / 16 % 16;
			Seg_Buf[7] = ucRtc[2] / 1 % 16;
			Back_Mode = 0;
		break;
		
		case 3://回显
			if(Back_Mode == 0)//频率
			{
				Seg_Buf[0] = 13;
				Seg_Buf[1] = 11;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = Freq_max / 10000 % 10;
				Seg_Buf[4] = Freq_max / 1000 % 10;
				Seg_Buf[5] = Freq_max / 100 % 10;
				Seg_Buf[6] = Freq_max / 10 % 10;
				Seg_Buf[7] = Freq_max / 1 % 10;
				while(Seg_Buf[i] == 0)
				{
					Seg_Buf[i] = 10;
					if(++i == 7) break;
				}
			}
			else//时间
			{
				Seg_Buf[0] = 13;
				Seg_Buf[1] = 14;
				Seg_Buf[2] = occur[0] / 16 % 16;
				Seg_Buf[3] = occur[0] / 1 % 16;
				Seg_Buf[4] = occur[1] / 16 % 16;
				Seg_Buf[5] = occur[1] / 1 % 16;
				Seg_Buf[6] = occur[2] / 16 % 16;
				Seg_Buf[7] = occur[2] / 1 % 16;
				
			}
		break;
	}

}

/*其他显示函数*/
void Led_Proc()
{
	if(Freq_last <= 500) V = 1;
	if(Freq_last >= PF) 
	{
		V = 5;
	}
	if((Freq_last > 500)&&(Freq_last < PF)) V = (4/(PF-500))*(Freq_last-500) + 1;
	Da_Write(V*51.0);
	
	ucLed[0] = (Seg_Disp_Mode == 0)&Led_Flag;
	if(Freq_last > PF)
		ucLed[1] = Led_Flag;
	else
		ucLed[1] = 0;
	if(Freq_last < 0)
		ucLed[1] = 1;
}

/*定时器1初始化函数*/
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

/*定时器0计次函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0X05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

/*定时器0中断服务函数*/
void Timer1Server() interrupt 3
{
	if(++Key_Slow_Down == 10) Key_Slow_Down = 0;
	if(++Seg_Slow_Down == 500) Seg_Slow_Down = 0;
	if(++Seg_Pos == 8) Seg_Pos = 0;
	if(++Timer_1000Ms == 1000)
	{
		Timer_1000Ms = 0;
		Freq = TH0 << 8 | TL0;
		TH0 = TL0 = 0;
	}
	if(++Timer_200Ms == 200)
	{
		Timer_200Ms = 0;
		Led_Flag ^= 1;
	}
	
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	
}

/*Main*/
void main()
{
	Set_Rtc(ucRtc);
	Sys_Init();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}