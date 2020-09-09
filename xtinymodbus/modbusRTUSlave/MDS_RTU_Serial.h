/********************************************************************************
 * @File name: MD_RTU_Serial.c
 * @Author: zspace
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 ********************************************************************************/
#ifndef _MD_RTU_SERIAL_H__
#define _MD_RTU_SERIAL_H__

#include <stdint.h>

/*********************************************************************/
void MDSInitSerial(void *obj, uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity);
void MDSSerialRecvByte(uint8_t byte);
void MDSTimeHandler100US(void);
/***************************************************************************/

#endif
