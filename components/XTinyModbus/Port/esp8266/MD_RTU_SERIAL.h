/*
 * MD_RTU_SERIAL.h
 *
 *  Created on: 13 oct. 2020
 *      Author: egonzalez
 */

#ifndef _MD_RTU_SERIAL_H_
#define _MD_RTU_SERIAL_H_

void MD_InitSerial(void *obj, uint32 baud, uint8 dataBits, uint8 stopBit, uint8 parity);
void MD_SerialRecvByte(uint8 byte);
void MD_SerialSWRecv_Send(uint8 mode);
void MD_SerialSendBytes(uint8 *bytes, uint16 num);
void MD_TimeHandler100US(void *arg);

#endif
