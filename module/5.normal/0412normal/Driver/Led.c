#include <Led.h>
#include <STC15F2K60S2.H>

idata unsigned char temp_1 = 0x00;
idata unsigned char temp_old_1 = 0xff;

void Led_Disp(unsigned char* ucLed)
{
	unsigned char temp;
	temp_1 = 0x00;
	temp_1 = (ucLed[0]<<0)|(ucLed[1]<<1)|(ucLed[2]<<2)|(ucLed[3]<<3)|(ucLed[4]<<4)|(ucLed[5]<<5)|(ucLed[6]<<6)|(ucLed[7]<<7);
	if(temp_1 != temp_old_1)
	{
		P0 = ~temp_1;
		temp = P2 & 0x1f;
		temp = temp | 0x80;
		P2 = temp;
		temp = P2 & 0x1f;
		P2 = temp;

		temp_old_1 = temp_1;
	}
}

void Led_Off()
{
	unsigned char temp;
	
	P0 = 0xff;
	temp = P2 & 0x1f;
	temp = temp | 0x80;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;
	
	temp_old_1 = 0x00;
}

idata unsigned char temp_0 = 0x00;
idata unsigned char temp_old_0 = 0xff;
void Beep(bit enable)
{
	unsigned char temp;
	
	if(enable)
		temp_0 |= 0x40;
	else
		temp_0 &= ~0x40;
	if(temp_old_0 != temp_0)
	{
		P0 = temp_0;
    temp = P2 & 0x1f;    // 保留P2的低5位
    temp = temp | 0xa0;  // 与0xa0进行或操作
    P2 = temp;           // 写入P2
    temp = P2 & 0x1f;    // 保留P2的低5位
    P2 = temp;           // 写入P2，关闭锁存器
    temp_old_0 = temp_0;
		
	}
}

void Relay(bit enable)
{
	unsigned char temp;
	
	if(enable)
		temp_0 |= 0x10;
	else
		temp_0 &= ~0x10;
	if(temp_old_0 != temp_0)
	{
		P0 = temp_0;
    temp = P2 & 0x1f;    // 保留P2的低5位
    temp = temp | 0xa0;  // 与0xa0进行或操作
    P2 = temp;           // 写入P2
    temp = P2 & 0x1f;    // 保留P2的低5位
    P2 = temp;           // 写入P2，关闭锁存器
    temp_old_0 = temp_0;
		
	}
}

void MOTOR(bit enable)
{
	unsigned char temp;
	
	if(enable)
		temp_0 |= 0x20;
	else
		temp_0 &= ~0x20;
	if(temp_old_0 != temp_0)
	{
		P0 = temp_0;
    temp = P2 & 0x1f;    // 保留P2的低5位
    temp = temp | 0xa0;  // 与0xa0进行或操作
    P2 = temp;           // 写入P2
    temp = P2 & 0x1f;    // 保留P2的低5位
    P2 = temp;           // 写入P2，关闭锁存器
    temp_old_0 = temp_0;
		
	}
}