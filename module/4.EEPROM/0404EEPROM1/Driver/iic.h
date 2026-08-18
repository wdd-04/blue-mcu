#ifndef __IIC_H
#define __IIC_H

void EEPROM_Write(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);
void EEPROM_Read(unsigned char* EEPROM_String,unsigned char addr,unsigned char num);

#endif