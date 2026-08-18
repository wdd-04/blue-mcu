/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <Init.h>
#include <Led.h>
#include <ds1302.h>
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
unsigned int Freq = 0;
int Freq_Output = 0;
unsigned int Freq_Max;
unsigned int  PF = 2000;//超限参数
int Correct = 0;//校准值参数
unsigned char Seg_Disp_Mode;
bit Para_Mode = 0;
bit Back_Mode = 0;
unsigned char ucRtc[3] = {0x13,0x30,0x55};
unsigned char ucRtc_Freq[3];
float V;
unsigned char Timer_200Ms;
bit Star_Flag;

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
			if(++Seg_Disp_Mode == 4) Seg_Disp_Mode = 0;
		break;
		
		case 5:
			if(Seg_Disp_Mode == 1)
				Para_Mode ^= 1;
			if(Seg_Disp_Mode == 3)
				Back_Mode ^= 1;
		break;
			
		case 8:
			if(Seg_Disp_Mode == 1)
			{
				if(Para_Mode == 0)
				{
					PF += 1000;
					if(PF == 10000) PF = 9000;
				}
				if(Para_Mode == 1)
				{
					Correct += 100;
					if(Correct == 1000) Correct = 900;
				}				
			}
		break;
			
		case 9:
			if(Seg_Disp_Mode == 1)
			{
				if(Para_Mode == 0)
				{
					PF -= 1000;
					if(PF == 0) PF = 1000;
				}
				if(Para_Mode == 1)
				{
					Correct -= 100;
					if(Correct == -1000) Correct = -900;
				}				
			}
		break;
	}
}

/*信息处理函数*/
void Seg_Proc()
{
	unsigned char i = 3;
	unsigned char j = 4;
	unsigned char k = 5;
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;//数码管减速程序
	
	Freq_Output = Freq + Correct;
	Read_Rtc(ucRtc);
	if(Freq_Max <= Freq_Output) 
	{
		Freq_Max = Freq_Output;		
		Read_Rtc(ucRtc_Freq);
	}
	switch(Seg_Disp_Mode)
	{
		case 0://频率界面
			if(Freq_Output >= 0)
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 10;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = Freq_Output / 10000 % 10;
				Seg_Buf[4] = Freq_Output / 1000 % 10;
				Seg_Buf[5] = Freq_Output / 100 % 10;
				Seg_Buf[6] = Freq_Output / 10 % 10;
				Seg_Buf[7] = Freq_Output / 1 % 10;
			}
			else
			{
				Seg_Buf[0] = 11;
				Seg_Buf[1] = 10;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = 10;
				Seg_Buf[4] = 10;
				Seg_Buf[5] = 10;
				Seg_Buf[6] = 17;
				Seg_Buf[7] = 17;	
				Da_Write(0);				
			}
			while(Seg_Buf[i] == 0)				
			{
				Seg_Buf[i] = 10;
				if(++i == 7) break;
			}
			Para_Mode = 0;
		break;
		
		case 1://参数界面
			if(Para_Mode == 0)//超极限参数界面
			{
				Seg_Buf[0] = 12;
				Seg_Buf[1] = 1;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = 10;
				Seg_Buf[4] = PF / 1000 % 10;
				Seg_Buf[5] = PF / 100 % 10;
				Seg_Buf[6] = PF / 10 % 10;
				Seg_Buf[7] = PF / 1 % 10;		
				while(Seg_Buf[j] == 0)
				{
					Seg_Buf[j] = 10;
					if(++j == 7) break;
				}						
			}
			else//校准值参数界面
			{
				if(Correct >= 0)
				{
					Seg_Buf[0] = 12;
					Seg_Buf[1] = 2;
					Seg_Buf[2] = 10;
					Seg_Buf[3] = 10;
					Seg_Buf[4] = 10;
					Seg_Buf[5] = Correct / 100 % 10;
					Seg_Buf[6] = Correct / 10 % 10;
					Seg_Buf[7] = Correct / 1 % 10;				
				}
				else
				{
					Seg_Buf[0] = 12;
					Seg_Buf[1] = 2;
					Seg_Buf[2] = 10;
					Seg_Buf[3] = 10;
					Seg_Buf[4] = 16;
					Seg_Buf[5] = Correct*(-1) / 100 % 10;
					Seg_Buf[6] = Correct*(-1) / 10 % 10;
					Seg_Buf[7] = Correct*(-1) / 1 % 10;								
				}
				while(Seg_Buf[k] == 0)
				{
					Seg_Buf[k] = 10;
					if(++k == 7) break;
				}						
			}
		break;
			
		case 2://时间界面
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
		
		case 3:
			if(Back_Mode == 0)//频率回显界面
			{
				Seg_Buf[0] = 13;
				Seg_Buf[1] = 11;
				Seg_Buf[2] = 10;
				Seg_Buf[3] = Freq_Max / 10000 % 10;
				Seg_Buf[4] = Freq_Max / 1000 % 10;
				Seg_Buf[5] = Freq_Max / 100 % 10;
				Seg_Buf[6] = Freq_Max / 10 % 10;
				Seg_Buf[7] = Freq_Max / 1 % 10;
				while(Seg_Buf[i] == 0)
				{
					Seg_Buf[i] = 10;
					if(++i == 7) break;
				}
			}
			else//时间回显界面
			{
				Seg_Buf[0] = 13;
				Seg_Buf[1] = 14;
				Seg_Buf[2] = ucRtc_Freq[0] / 16 % 16;
				Seg_Buf[3] = ucRtc_Freq[0] / 1 % 16;
				Seg_Buf[4] = ucRtc_Freq[1] / 16 % 16;
				Seg_Buf[5] = ucRtc_Freq[1] / 1 % 16;
				Seg_Buf[6] = ucRtc_Freq[2] / 16 % 16;
				Seg_Buf[7] = ucRtc_Freq[2] / 1 % 16;	
			}
		break;
	}
}

/*其他显示函数*/
void Led_Proc()
{
	if(Freq_Output <= 500) V = 1;
	if(Freq_Output >= PF) V = 5;
	if((Freq_Output > 500) && (Freq_Output < PF))
	{
		V = (4 / (PF-500))*(Freq_Output - 500) + 1;
	}
	Da_Write(V*51.0);
	
	ucLed[0] = (Seg_Disp_Mode == 0) & Star_Flag;
	if(Freq_Output > PF)
		ucLed[1] = Star_Flag;
	else
		ucLed[1] = 0;
	if(Freq_Output < 0)
		ucLed[1] = 1;
}

void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
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
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	
	if(++Timer_1000Ms == 1000)//获取频率值
	{
		Timer_1000Ms = 0;
		Freq = TH0 << 8 | TL0;
		TH0 = 0;
		TL0 = 0;
	}	
	
	if(++Timer_200Ms == 200)
	{
		Timer_200Ms = 0;
		Star_Flag ^= 1;
	}
}

/*Main*/
void main()
{
	Set_Rtc(ucRtc);
	Read_Rtc(ucRtc);
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
