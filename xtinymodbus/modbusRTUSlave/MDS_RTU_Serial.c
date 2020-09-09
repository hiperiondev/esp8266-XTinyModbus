/********************************************************************************
 * @File name: MD_RTU_Serial.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 *  https://github.com/lotoohe-space/XTinyModbus
 ********************************************************************************/

#include <stdint.h>
#include "MDS_RTU_Serial.h"
#include "MDS_RTU_Fun.h"
#include "modbus_uart.h"

/********************************************************************/
PModbusBase pModbusBase = NULL;
/***************************************************************************/

/*********************************************************************/
void MDSSerialSendBytes(uint8_t *bytes, uint16_t num);
void MDSSerialSWRecv_Send(uint8_t mode);
/***************************************************************************/

/*******************************************************
 *
 * Function name: MDSInitSerial
 * Description:
 * Parameter:
 *        @obj:
 *        @baud:
 *        @dataBits:
 *        @stopBit:
 *        @parity:
 * Return:
 **********************************************************/
void MDSInitSerial(void *obj, uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity) {
    pModbusBase = obj;
    if (obj == NULL) {
        return;
    }

    pModbusBase->mdRTUSendBytesFunction = MDSSerialSendBytes;
    pModbusBase->mdRTURecSendConv = MDSSerialSWRecv_Send;

    modbus_uart_init(baud, dataBits, stopBit, parity);
}

/*******************************************************
 *
 * Function name: MDSSerialRecvByte
 * Description:
 * Parameter:
 *        @byte:
 * Return:
 **********************************************************/
void MDSSerialRecvByte(uint8_t byte) {
    if (pModbusBase == NULL) {
        return;
    }
    pModbusBase->mdRTURecByteFunction(pModbusBase, byte);
}

/*******************************************************
 *
 * Function name: MDSSerialSWRecv_Send
 * Description:
 * Parameter:
 *        @mode:
 * Return:
 **********************************************************/
void MDSSerialSWRecv_Send(uint8_t mode) {

}

/*******************************************************
 *
 * Function name: MDSSerialSendBytes
 * Description:
 * Parameter:
 *        @bytes:
 *        @num:
 * Return:
 **********************************************************/
void MDSSerialSendBytes(uint8_t *bytes, uint16_t num) {
    modbus_uart_send_bytes_by_isr(bytes, num);
}

/*******************************************************
 *
 * Function name: MDSTimeHandler100US
 * Description:
 * Parameter:
 * Return:
 **********************************************************/
void MDSTimeHandler100US(void) {
    if (pModbusBase == NULL) {
        return;
    }
    pModbusBase->mdRTUTimeHandlerFunction(pModbusBase);
}
