#ifndef __IIC_H
#define __IIC_H

unsigned char Ad_Read(unsigned char addr);
void Da_Write(unsigned char dat);
void EEPORM_Write(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);
void EEPORM_Read(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);

#endif