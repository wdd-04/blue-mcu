/*头文件*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include <Key.h>
#include <Seg.h>
#include <Led.h>
#include <iic.h>

/*变量*/
unsigned char Key_Slow_Down;
unsigned int Seg_Slow_Down;
unsigned char Seg_Pos;
unsigned char Led_Pos;
unsigned char Key_Val,Key_Down,Key_Up,Key_Old;
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};
unsigned char Seg_Point[8] = {0,0,0,0,0,0,0,0};
unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Disp_Mode;//0-熄灭 1-模式 2-流转时间
unsigned char Led_Mode;//0正 1负 2聚拢 3发散
unsigned int Timeflow = 400;
unsigned char Lightlevel;
bit Star_Flag;
unsigned int Timer_400Ms;
unsigned char PWM;
bit System_Flag;//0停止 1开始
unsigned int Tick;

/*按键*/
void Key_Proc()
{
	if(Key_Slow_Down) return;
	Key_Slow_Down = 1;
	
	Key_Val = Key_Read();
	Key_Down = Key_Val&(Key_Val^Key_Old);
	Key_Up = ~Key_Val&(Key_Val^Key_Old);
	Key_Old = Key_Val;
	
	if((Key_Old == 4)&&(Seg_Disp_Mode == 0))
	{
		Seg_Buf[6] = 0;
		Seg_Buf[7] = Lightlevel + 1;
	}
	
	switch(Key_Down)
	{
		case 6:
			if(++Seg_Disp_Mode == 3)
				Seg_Disp_Mode = 0;
		break;
			
		case 5:
			if(Seg_Disp_Mode == 1)
			{
				if(++Led_Mode == 4) Led_Mode = 0;
			}
			if(Seg_Disp_Mode == 2)
			{
				Timeflow += 100;
				if(Timeflow == 1300)
					Timeflow = 400;
			}
		break;
			
		case 4:
			if(Seg_Disp_Mode == 1)
			{
					if(--Led_Mode == 255) Led_Mode = 0;
			}
			if(Seg_Disp_Mode == 2)
			{
				Timeflow -= 100;
				if(Timeflow == 300)
					Timeflow = 1200;
			}
		break;
			
		case 7:
			System_Flag ^= 1;
		break;
	}
}

/*信息*/
void Seg_Proc()
{
	unsigned char i = 4;
	if(Seg_Slow_Down) return;
	Seg_Slow_Down = 1;
	
	Lightlevel = Ad_Read(0x43)/64;
	
	switch(Seg_Disp_Mode)
	{
		case 0://熄灭
			Seg_Buf[0] = 10;
			Seg_Buf[1] = 10;
			Seg_Buf[2] = 10;
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;
			Seg_Buf[5] = 10;
			Seg_Buf[6] = 10;
			Seg_Buf[7] = 10;							
		break;
		
		case 1://模式设置
			Seg_Buf[0] = Star_Flag?0:10;
			Seg_Buf[1] = Star_Flag?(Led_Mode + 1):10;
			Seg_Buf[2] = Star_Flag?0:10;
			Seg_Buf[4] = Timeflow / 1000 % 10;
			Seg_Buf[5] = Timeflow / 100 % 10;
			Seg_Buf[6] = Timeflow / 10 % 10;
			Seg_Buf[7] = Timeflow / 1 % 10;						
		break;
			
		case 2://流转时间
			Seg_Buf[0] = 0;
			Seg_Buf[1] = Led_Mode + 1;
			Seg_Buf[2] = 0;
			Seg_Buf[4] = Star_Flag?10:Timeflow / 1000 % 10;
			Seg_Buf[5] = Star_Flag?10:Timeflow / 100 % 10;
			Seg_Buf[6] = Star_Flag?10:Timeflow / 10 % 10;
			Seg_Buf[7] = Star_Flag?10:Timeflow / 1 % 10;
		break;		
	}
			while(Seg_Buf[i] == 0)
			{
				Seg_Buf[i] = 10;
				if(++i == 7) i = 0;
			}			
}

/*其他*/
void Led_Proc()
{
	unsigned char i;
	if(System_Flag == 1)
	{
		if(Tick == Timeflow)
		{
			Tick = 0;
			switch(Led_Mode)
			{
				case 0:
					if(++Led_Pos == 8)
					{
						Led_Pos = 7;
						Led_Mode = 1;
					}					
				break;				
				
				case 1:
					if(--Led_Pos == 255)
					{
						Led_Pos = 7;
						Led_Mode = 2;
					}										
				break;				
				
				case 2:
					Led_Pos += 9;
					if(Led_Pos > 34)
					{
						Led_Pos = 34;
						Led_Mode = 3;
					}					
				break;
				
				case 3:
					Led_Pos -= 9;
					if(Led_Pos > 200)
					{
						Led_Pos = 0;
						Led_Mode = 0;
					}					
				break;
			}
		}
	}
	if(Led_Mode < 2)//前两种模式
	{
		for(i=0;i<8;i++)
			ucLed[i] = (i == Led_Pos);	
	}
	else
	{
		for(i=0;i<8;i++)
			ucLed[i] = ((i == Led_Pos / 10) || (i == Led_Pos % 10));
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

/*定时器0服务*/
void Timer0Server() interrupt 1
{
	if(++Seg_Slow_Down == 500) Seg_Slow_Down = 0;
	if(++Key_Slow_Down == 10) Key_Slow_Down = 0;
	if(++Seg_Pos == 8) Seg_Pos = 0;
	if(++PWM == 12) PWM = 0;
	
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	
	if(++Timer_400Ms == 400)
	{
		Timer_400Ms = 0;
		Star_Flag ^= 1;
	}
	
	if(PWM < (Lightlevel + 1)*4)
		Led_Disp(Seg_Pos,ucLed[Seg_Pos]);
	else
		Led_Disp(Seg_Pos,0);
	
	if(System_Flag == 1)
		Tick++;
}

/*Main*/
void main()
{
	Sys_Init();
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}