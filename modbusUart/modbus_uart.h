/*
 * modbus_uart.h
 *
 *  Created on: 7 sep. 2020
 *      Author: Emiliano Augusto Gonzalez (comercial@hiperion.com.ar)
 */

#ifndef _UART_MODBUS_UART_H_
#define _UART_MODBUS_UART_H_

typedef enum {
    MB_PAR_NONE, // No parity
    MB_PAR_ODD,  // Odd parity
    MB_PAR_EVEN  // Even parity
} eMBParity;

void modbus_uart_init(uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity);
void modbus_uart_send_bytes_by_isr(uint8_t *bytes, uint16_t num);
void modbus_uart_send_bytes(uint8_t *bytes,uint16_t num);

#endif
