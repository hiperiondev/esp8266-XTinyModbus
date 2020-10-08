/*
 * MD_UART.h
 *
 *  Created on: 7 oct. 2020
 *      Author: egonzalez
 */

#ifndef _MD_UART_H_
#define _MD_UART_H_

#define EX_UART_NUM UART_NUM_0
#include "driver/uart.h"

bool SerialSW;
void (* _SerialRecvByte)(uint8_t byte);
void uart_init(uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity, void (* _SerialRecvByte)(uint8_t byte));

#endif
