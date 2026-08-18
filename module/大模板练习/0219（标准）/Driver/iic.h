#ifndef __DS1302_H
#define __DS1302_H

unsigned char Ad_Read(unsigned char addr);
void Da_Write(unsigned char dat);
void EEPROM_Write(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);
void EEPROM_Write(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);

#endif