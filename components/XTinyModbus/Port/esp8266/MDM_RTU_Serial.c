/*
 * MDM_RTU_Serial.c
 * Port for:
 */

#include "MD_RTU_Tool.h"
#include "MDS_RTU_Serial.h"
static PModbusBase pModbusBase = NULL;

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
void MDMInitSerial(void *obj, uint32 baud, uint8 dataBits, uint8 stopBit, uint8 parity) {

}

/*******************************************************
 *
 * Function name: MDMSerialRecvByte
 * Description: Bsp layer serial port interrupt receiving call this function
 * Parameter:
 *         @byte    Byte received
 * Return: None
 **********************************************************/
void MDMSerialRecvByte(uint8 byte) {

}

/*******************************************************
 *
 * Function name: MDMTimeHandler100US
 * Description: Call this function in the timer
 * Parameter:
 *         None
 * Return: None
 **********************************************************/
void MDMTimeHandler100US(void) {

}

/*******************************************************
 *
 * Function name: MDMSerialSendBytes
 * Description: Send function
 * Parameter:
 *         @bytes    Data sent
 *         @num      How many bytes to send
 * Return: None
 **********************************************************/
void MDMSerialSendBytes(uint8 *bytes, uint16 num) {
}

/*******************************************************
 *
 * Function name: MDMSerialRecvByte
 * Description: Switch to receive or send
 * Parameter:
 *         @mode    TRUE send, FALSE receive
 * Return: None
 **********************************************************/
void MDMSerialSWRecv_Send(uint8 mode) {

}
