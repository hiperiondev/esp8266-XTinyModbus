/*
 * MDS_RTU_Serial.c
 * Port for:
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_SERIAL.h"

#define SERIAL_SW_GPIO 2
#define SERIAL_SW_GPIO_OUTPUT_PIN_SEL  ((1ULL<<SERIAL_SW_GPIO))
//static PModbusBase pModbusBase=NULL;

/*******************************************************
 *
 * Function name: MDSInitSerial
 * Description: Hardware initialization function, you can initialize the serial port here
 * Parameter:
 *         @obj         Slave object pointer
 *         @baud        Baud rate
 *         @dataBits    Data bit
 *         @stopBit     Stop bit
 *         @parity      Parity bit
 * Return: None
 **********************************************************/
void MDSInitSerial(void *obj, uint32 baud, uint8 dataBits, uint8 stopBit, uint8 parity) {
    MD_InitSerial(obj, baud, dataBits, stopBit, parity);
}

/*******************************************************
 *
 * Function name: MDSSerialRecvByte
 * Description: Bsp layer interrupt receiving call this function
 * Parameter:
 *         @byte    A byte received
 * Return: None
 **********************************************************/
void MDSSerialRecvByte(uint8 byte) {
    MD_SerialRecvByte(byte);
}

/*******************************************************
 *
 * Function name: MDSSerialSWRecv_Send
 * Description: Switch to receive or send
 * Parameter:
 *         @mode    TRUE send, FALSE receive
 * Return: None
 **********************************************************/
void MDSSerialSWRecv_Send(uint8 mode) {
    MD_SerialSWRecv_Send(mode);
}

/*******************************************************
 *
 * Function name: MDSSerialSendBytes
 * Description: Send function
 * Parameter:
 *         @bytes    Data sent
 *         @num      How many bytes to send
 * Return: None
 **********************************************************/
void MDSSerialSendBytes(uint8 *bytes, uint16 num) {
    MD_SerialSendBytes(bytes, num);
}

/*******************************************************
 *
 * Function name: MDSTimeHandler100US
 * Description: Call this function in the timer, the timing unit is 100US
 * Parameter:
 *         None
 * Return: None
 **********************************************************/
void MDSTimeHandler100US(void) {
}
