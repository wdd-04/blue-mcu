#ifndef __IIC_H
#define __IIC_H

unsigned char Ad_Read(unsigned char addr);
void Da_Write(unsigned char dat);

void I2CStart(void);
void I2CStop(void);
void I2CSendByte(unsigned char byt);
unsigned char I2CReceiveByte(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(unsigned char ackbit);



#endif