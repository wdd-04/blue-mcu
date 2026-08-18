#include <Init.h>

void System_Init()//关闭外设子函数
{
	P0 = 0xff;//关闭Led
	P2 = P2 & 0x1f | 0x80;//选中Y4C锁存器
	P2 &= 0x1f;//关闭Y4c锁存器
	
	P0 = 0x00;//关闭继电器、蜂鸣器
	P2 = P2 & 0x1f | 0xa0;//选中Y5C锁存器
	P2 &= 0x1f;//关闭Y5c锁存器
}