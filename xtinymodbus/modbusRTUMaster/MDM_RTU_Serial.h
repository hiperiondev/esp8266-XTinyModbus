/********************************************************************************
* @File name: MD_RTU_Serial.h
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU
********************************************************************************/

#ifndef _MDM_RTU_SERIAL_H__
#define _MDM_RTU_SERIAL_H__

/**********************************************************************/
#include <stdint.h>

/***********************************************************/
void MDMInitSerial(void* obj,uint32_t baud,uint8_t dataBits,uint8_t stopBit,uint8_t parity);
void MDMSerialRecvByte(uint8_t byte);
void MDMTimeHandler100US(void);

void MDMSerialSendBytes(uint8_t *bytes,uint16_t num);
void MDMSerialSWRecv_Send(uint8_t mode);
/***********************************************************************/

#endif
