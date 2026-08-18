#include <iic.h>
#include <reg52.h>
#include <intrins.h>

sbit sda = P2^1;
sbit scl = P2^0;

#define DELAY_TIME	10

//
static void I2C_Delay(unsigned char n)
{
    do
    {
        _nop_();_nop_();_nop_();_nop_();_nop_();
        _nop_();_nop_();_nop_();_nop_();_nop_();
        _nop_();_nop_();_nop_();_nop_();_nop_();		
    }
    while(n--);      	
}

//
void I2CStart(void)
{
    sda = 1;
    scl = 1;
	I2C_Delay(DELAY_TIME);
    sda = 0;
	I2C_Delay(DELAY_TIME);
    scl = 0;    
}

//
void I2CStop(void)
{
    sda = 0;
    scl = 1;
	I2C_Delay(DELAY_TIME);
    sda = 1;
	I2C_Delay(DELAY_TIME);
}

//
void I2CSendByte(unsigned char byt)
{
    unsigned char i;
	
    for(i=0; i<8; i++){
        scl = 0;
		I2C_Delay(DELAY_TIME);
        if(byt & 0x80){
            sda = 1;
        }
        else{
            sda = 0;
        }
		I2C_Delay(DELAY_TIME);
        scl = 1;
        byt <<= 1;
		I2C_Delay(DELAY_TIME);
    }
	
    scl = 0;  
}

//
unsigned char I2CReceiveByte(void)
{
	unsigned char da;
	unsigned char i;
	for(i=0;i<8;i++){   
		scl = 1;
		I2C_Delay(DELAY_TIME);
		da <<= 1;
		if(sda) 
			da |= 0x01;
		scl = 0;
		I2C_Delay(DELAY_TIME);
	}
	return da;    
}

//
unsigned char I2CWaitAck(void)
{
	unsigned char ackbit;
	
    scl = 1;
	I2C_Delay(DELAY_TIME);
    ackbit = sda; 
    scl = 0;
	I2C_Delay(DELAY_TIME);
	
	return ackbit;
}

//
void I2CSendAck(unsigned char ackbit)
{
    scl = 0;
    sda = ackbit; 
	I2C_Delay(DELAY_TIME);
    scl = 1;
	I2C_Delay(DELAY_TIME);
    scl = 0; 
	sda = 1;
	I2C_Delay(DELAY_TIME);
}

//AD转换
unsigned char Ad_Read(unsigned char addr)
{
	unsigned char temp;
	I2CStart();
	I2CSendByte(0x90);//写数据
	I2CWaitAck();
	I2CSendByte(addr);//从哪获取数据
	I2CWaitAck();

	I2CStart();
	I2CSendByte(0x91);//读数据
	I2CWaitAck();
	temp = I2CReceiveByte();
	I2CSendAck(1);//非应答信号，读取结束
	I2CStop();
	return temp;
}

//DA转换
void Da_Write(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90);//写数据
	I2CWaitAck();
	I2CSendByte(0x41);//使能转换,0x4?皆可
	I2CWaitAck();
	I2CSendByte(dat);//写数据
	I2CWaitAck();
	I2CStop();
}

void EEPROM_Write(unsigned char* EEPROM_String,unsigned char addr,unsigned char num)
{
	I2CStart();
	I2CSendByte(0xa0);//写模式
	I2CWaitAck();
	
	I2CSendByte(addr);//要写入数据的地址
	I2CWaitAck();
	
	while(num--)
	{
		I2CSendByte(*EEPROM_String++);//写入
		I2CWaitAck();
		I2C_Delay(200);
	}
	I2CStop();
}

void EEPROM_Read(unsigned char* EEPROM_String,unsigned char addr,unsigned char num)
{
	I2CStart();
	I2CSendByte(0xa0);//写模式
	I2CWaitAck();

	I2CSendByte(addr);//要读取数据的地址
	I2CWaitAck();

	I2CStart();
	I2CSendByte(0xa1);//读模式
	I2CWaitAck();
	
	while(num--)
	{
		*EEPROM_String++ = I2CReceiveByte();//读取
		if(num) I2CSendAck(0);//继续传输数据
		else I2CSendAck(1);//读取完毕，停止传输
	}
	I2CStop();	
}