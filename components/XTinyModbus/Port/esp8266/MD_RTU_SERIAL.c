/*
 * MD_RTU_SERIAL.c
 *
 *  Created on: 13 oct. 2020
 *      Author: egonzalez
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "MD_RTU_Tool.h"
#include "MDS_RTU_Serial.h"
#include "MD_UART.h"
#include "MD_TIMER.h"

#define SERIAL_SW_GPIO 2
#define SERIAL_SW_GPIO_OUTPUT_PIN_SEL  ((1ULL<<SERIAL_SW_GPIO))
static PModbusBase pModbusBase=NULL;

void MD_TimeHandler100US(void *arg);

/*******************************************************
 *
 * Function name: MD_InitSerial
 * Description: Hardware initialization function, you can initialize the serial port here
 * Parameter:
 *         @obj         Slave object pointer
 *         @baud        Baud rate
 *         @dataBits    Data bit
 *         @stopBit     Stop bit
 *         @parity      Parity bit
 * Return: None
 **********************************************************/
void MD_InitSerial(void *obj, uint32 baud, uint8 dataBits, uint8 stopBit, uint8 parity) {
    pModbusBase = obj;
    if (obj == NULL) {
        return;
    }

    SerialSW = true;

    pModbusBase->mdRTUSendBytesFunction = MDSSerialSendBytes;
    pModbusBase->mdRTURecSendConv = MDSSerialSWRecv_Send;

    uart_init(baud, dataBits, stopBit, parity, &MDSSerialRecvByte);
    md_timer_init(100, &MD_TimeHandler100US, true);

    if (SerialSW) {
        gpio_config_t io_conf = {
                .intr_type    = GPIO_INTR_DISABLE,
                .mode         = GPIO_MODE_OUTPUT,
                .pin_bit_mask = SERIAL_SW_GPIO_OUTPUT_PIN_SEL,
                .pull_down_en = 0,
                .pull_up_en   = 0
        };
        gpio_config(&io_conf);
        gpio_set_level(SERIAL_SW_GPIO, 1);
    }
}

/*******************************************************
 *
 * Function name: MD_SerialRecvByte
 * Description: Bsp layer interrupt receiving call this function
 * Parameter:
 *         @byte    A byte received
 * Return: None
 **********************************************************/
void MD_SerialRecvByte(uint8 byte) {
    if (pModbusBase == NULL)
        return;

    pModbusBase->mdRTURecByteFunction(pModbusBase, byte);
}

/*******************************************************
 *
 * Function name: MD_SerialSWRecv_Send
 * Description: Switch to receive or send
 * Parameter:
 *         @mode    TRUE send, FALSE receive
 * Return: None
 **********************************************************/
void MD_SerialSWRecv_Send(uint8 mode) {
    if (SerialSW) {
        gpio_set_level(SERIAL_SW_GPIO, !mode);
    }
}

/*******************************************************
 *
 * Function name: MD_SerialSendBytes
 * Description: Send function
 * Parameter:
 *         @bytes    Data sent
 *         @num      How many bytes to send
 * Return: None
 **********************************************************/
void MD_SerialSendBytes(uint8 *bytes, uint16 num) {
    uart_write_bytes(EX_UART_NUM, (const char*) bytes, num);
}

/*******************************************************
 *
 * Function name: MD_TimeHandler100US
 * Description: Call this function in the timer, the timing unit is 100US
 * Parameter:
 *         None
 * Return: None
 **********************************************************/
void MD_TimeHandler100US(void *arg) {
    if (pModbusBase == NULL)
        return;

    pModbusBase->mdRTUTimeHandlerFunction(pModbusBase);
}
