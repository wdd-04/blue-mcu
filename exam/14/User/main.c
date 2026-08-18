/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <Init.h>
#include <Led.h>
#include <onewire.h>
#include <ds1302.h>
#include <iic.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;//按键专用变量
unsigned char Key_Slow_Down;//按键减速专用变量
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};//数码管显示数据存放数组
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};//数码管小数点数据存放数组
unsigned char Seg_Pos;//数码管扫描专用变量
unsigned int xdata Seg_Slow_Down;//数码管减速专用变量
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};//Led显示数据存放数组
unsigned int xdata Timer_1000Ms;
unsigned int xdata Freq;
unsigned char Seg_Disp_Mode;//0-时间界面 1-回显界面 2-参数界面 3-温湿度界面
unsigned char Seg_Disp_Mode_Old;
unsigned char Back_Show_Mode;//0-温度 1-湿度 2-时间
unsigned char time[3] = {0x23,0x59,0x55};
float T;
unsigned char T_Para = 30;
float T_max;
float xdata T_aver;
float xdata T_data[10];
float Wet;
float Wet_max;
float xdata Wet_aver;
float xdata Wet_data[10];
bit Wet_Valid;//0无效 1有效
bit Trigger_Flag;//0不允许触发 1允许触发
unsigned char Trigger_Count = 0;
unsigned char V;
bit V_Flag = 1;
unsigned int xdata Timer_2s;//按键时间
bit Key_Flag;//按键使能
bit L4_Star;
unsigned char xdata Timer_100Ms;
float sum_T;
float sum_Wet;
unsigned char i;

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;//按键减速程序
	
	Key_Val = Key_Read();//实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val);//捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//捕捉按键上升沿
	Key_Old = Key_Val;//辅助扫描变量

	
	if((Seg_Disp_Mode == 1)&&(Back_Show_Mode == 2))
	{
		if(Key_Down == 9)
			Key_Flag = 1;//开始计时
	}
	if(Timer_2s > 2000)
	{
		if(Key_Up == 9)
		{
			Trigger_Count = 0;
			Key_Flag = Timer_2s = 0;
			T = T_max = T_aver = Wet = Wet_max = Wet_aver = 0;
		}
	}
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Disp_Mode == 3)
				Seg_Disp_Mode = 0;				
			Seg_Disp_Mode_Old = Seg_Disp_Mode;
		break;
			
		case 5:
			if(Seg_Disp_Mode == 1)
			{
				if(++Back_Show_Mode == 3)
					Back_Show_Mode = 0;			
			}
		break;
			
		case 8:
			if(Seg_Disp_Mode == 2)
			{
				if(++T_Para == 99)
					T_Para = 0;			
			}
		break;
			
		case 9:
			if(Seg_Disp_Mode == 2)
			{
				if(--T_Para == 255)
					T_Para = 99;			
			}
		break;
	}
	
}

void Delay3000ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;
	i = 137;
	j = 203;
	k = 232;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;//数码管减速程序
	
	/*信息获取区域*/
	Read_Rtc(time);
	V =	Ad_Read(0x01);

	
	if(V > 100)
	{
		if(T_max <= T) T_max = T;
		if(Wet_max <= Wet) Wet_max = Wet;
		Seg_Disp_Mode = Seg_Disp_Mode_Old;
		V_Flag = 1;
	}
	else if((V < 100)&&(V_Flag == 1))
	{
		T = read_t();
		if(T_max <= T) T_max = T;
		if(Wet_max <= Wet) Wet_max = Wet;

		if(Trigger_Count == 0)
		{
			T_aver = T;
			Wet_aver = Wet;	
		}
		
		if(Freq <= 200)
		{
			Wet_Valid = 0;
			Wet = 10;
		}
		else if(Freq >= 2000)
		{
			Wet_Valid = 0;
			Wet = 90;		
		}
		else
		{
			Wet_Valid = 1;
			Wet = (80.0/1800.0)*(Freq - 200) + 10;
		}

		V_Flag = 0;
		T_data[Trigger_Count] = T;
		Wet_data[Trigger_Count] = Wet;

		Trigger_Count++;
		Seg_Disp_Mode = 3;

		sum_T = 0.0;
		sum_Wet = 0.0;
		for(i=0;i<Trigger_Count;i++)
		{
			sum_T += T_data[i];
			sum_Wet += Wet_data[i];		
			T_aver = sum_T / (i+1);
			Wet_aver = sum_Wet / (i+1);		
		}

//		T_aver[Trigger_Count] = ((T_aver[Trigger_Count-1])*(Trigger_Count) + T_data[Trigger_Count]) / (Trigger_Count);
//		Wet_aver[Trigger_Count] = ((Wet_aver[Trigger_Count-1])*(Trigger_Count) + Wet_data[Trigger_Count]) / (Trigger_Count+1);		
	}
	
	
	
	
	/*信息处理区域*/
	switch(Seg_Disp_Mode)
	{
		case 0://时间界面
			ucLed[2] = 0;
			ucLed[0] = 1;
			Seg_Buf[0] = time[0] / 16 % 16;
			Seg_Buf[1] = time[0] / 1 % 16;
			Seg_Buf[2] = 11;
			Seg_Buf[3] = time[1] / 16 % 16;
			Seg_Buf[4] = time[1] / 1 % 16;
			Seg_Buf[5] = 11;		
			Seg_Buf[6] = time[2] / 16 % 16;
			Seg_Buf[7] = time[2] / 1 % 16;
		break;
		
		case 1://回显界面
			ucLed[2] = 0;
			ucLed[0] = 0;
			ucLed[1] = 1;
			switch(Back_Show_Mode)
			{
				case 0://温度
					Seg_Buf[0] = 12;
					Seg_Buf[1] = 10;
					if(Trigger_Count)
					{
						Seg_Buf[2] = (unsigned char)T_max / 10 % 10;
						Seg_Buf[3] = (unsigned char)T_max / 1 % 10;
						Seg_Buf[4] = 11;
						Seg_Buf[5] = (unsigned char)T_aver / 10 % 10;		
						Seg_Buf[6] = (unsigned char)T_aver / 1 % 10;;
						Seg_Buf[7] = (unsigned int)(T_aver*10) / 1 % 10;;			
						Seg_Point[6] = 1;					
					}
					else
					{
						Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10;
						Seg_Point[6] = 0;					
					}
				break;
				
				case 1://湿度
					Seg_Buf[0] = 13;
					Seg_Buf[1] = 10;
					if(Trigger_Count)
					{
						Seg_Buf[2] = (unsigned char)Wet_max / 10 % 10;
						Seg_Buf[3] = (unsigned char)Wet_max / 1 % 10;
						Seg_Buf[4] = 11;
						Seg_Buf[5] = (unsigned char)Wet_aver / 10 % 10;		
						Seg_Buf[6] = (unsigned char)Wet_aver / 1 % 10;;
						Seg_Buf[7] = (unsigned int)(Wet_aver*10) / 1 % 10;;			
						Seg_Point[6] = 1;					
					}
					else
					{
						Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10;
						Seg_Point[6] = 0;					
					}
				break;
				
				case 2://时间
					Seg_Buf[0] = 14;
					Seg_Buf[1] = Trigger_Count / 10 % 10;
					Seg_Buf[2] = Trigger_Count / 1 % 10;
					if(Trigger_Count)
					{
						Seg_Buf[3] = time[0] / 16 % 16;
						Seg_Buf[4] = time[0] / 1 % 16;
						Seg_Buf[5] = 11;		
						Seg_Buf[6] = time[1] / 16 % 16;
						Seg_Buf[7] = time[1] / 1 % 16;								
						Seg_Point[6] = 0;					
					}
					else
					{
						Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10;				
					}
				break;
			}
		break;
			
		case 2://参数界面
			ucLed[2] = 0;
			ucLed[1] = 0;
			Seg_Buf[0] = 15;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;
			Seg_Buf[5] = 10;		
			Seg_Buf[6] = T_Para / 10 % 10;
			Seg_Buf[7] = T_Para % 10;								
			Seg_Point[6] = 0;
		break;
		
		case 3://温湿度界面
			Seg_Point[6] = 0;					
			ucLed[0] = 0;
			ucLed[1] = 0;
			ucLed[2] = 1;
			Seg_Buf[0] = 16;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = (unsigned char)T / 10 % 10;
			Seg_Buf[4] = (unsigned char)T / 1 % 10;
			Seg_Buf[5] = 11;
			if(Wet_Valid == 1)
			{
				ucLed[4] = 0;
				Seg_Buf[6] = (unsigned char)Wet / 10 % 10;
				Seg_Buf[7] = (unsigned char)Wet / 1 % 10;														
			}
			else if(Wet_Valid == 0)
			{
				ucLed[4] = 1;
				Seg_Buf[6] = Seg_Buf[7] = 17;				
			}
		Delay3000ms();
		break;
	}
}

/*其他显示函数*/
void Led_Proc()
{
	if(T > T_Para)
		ucLed[3] = L4_Star?1:0;
	
	if((Trigger_Count>=2)&&(T_data[Trigger_Count]>T_data[Trigger_Count-1])&&(Wet_data[Trigger_Count]>Wet_data[Trigger_Count-1]))
		ucLed[5] = 1;
	else
		ucLed[5] = 0;
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
	
	if(Key_Flag == 1)
	{
		if(++Timer_2s == 2100)
			Timer_2s = 2100;
	}
	
	if(++Timer_100Ms == 100)
	{
		Timer_100Ms = 0;
		L4_Star ^= 1;
	}
}

/*延时函数*/
void Delay750ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;
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
	Set_Rtc(time);
	Read_Rtc(time);
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
