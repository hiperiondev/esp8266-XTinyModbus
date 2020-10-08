/********************************************************************************
 * @File name: MDM_RTU_Serial.h
 * @Author: zspace
 * @Email: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU Serial related modules
 ********************************************************************************/

#ifndef _MDM_RTU_SERIAL_H__
#define _MDM_RTU_SERIAL_H__

/*********************************HEAD FILE************************************/
#include "MD_RTU_Type.h"
/*********************************END******************************************/

/*********************************Function declaration************************************/
void MDMInitSerial(void *obj, uint32 baud, uint8 dataBits, uint8 stopBit, uint8 parity);
void MDMSerialRecvByte(uint8 byte);
void MDMTimeHandler100US(void);
void MDMSerialSendBytes(uint8 *bytes, uint16 num);
void MDMSerialSWRecv_Send(uint8 mode);
/*********************************END******************************************/

#endif
