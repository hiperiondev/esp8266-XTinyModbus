/********************************************************************************
 * @File name: MD_RTU_Serial.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 * https://github.com/lotoohe-space/XTinyModbus
 ********************************************************************************/

#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "modbus_uart.h"

/*****************************************************************/
PModbusBase pModbusMBase = NULL;
/*******************************************************************/

/*****************************************************************/

//void MDMTimeHandler100US(void);
void MDMSerialSendBytes(uint8_t *bytes, uint16_t num);
void MDMSerialSWRecv_Send(uint8_t mode);
/**********************************************************************/

/*******************************************************
 *
 * Function name: MDMInitSerial
 * Description:
 * Parameter:
 *        @obj:
 *        @baud:
 *        @dataBits:
 *        @stopBit:
 *        @parity:
 * Return:
 **********************************************************/
void MDMInitSerial(void *obj, uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity) {
    pModbusMBase = obj;
    if (obj == NULL) {
        return;
    }

    pModbusMBase->mdRTUSendBytesFunction = MDMSerialSendBytes;
    pModbusMBase->mdRTURecSendConv = MDMSerialSWRecv_Send;

    modbus_uart_init(baud, dataBits, stopBit, parity);

}

/*******************************************************
 *
 * Function name: MDMTimeHandler100US
 * Description:
 * Parameter:
 * Return:
 **********************************************************/
void MDMTimeHandler100US(void) {
    if (pModbusMBase == NULL) {
        return;
    }
    pModbusMBase->mdRTUTimeHandlerFunction(pModbusMBase);
}

/*******************************************************
 *
 * Function name: MDMSerialRecvByte
 * Description:
 * Parameter:
 *        @byte:
 * Return:
 **********************************************************/
void MDMSerialRecvByte(uint8_t byte) {
    if (pModbusMBase == NULL) {
        return;
    }
    pModbusMBase->mdRTURecByteFunction(pModbusMBase, byte);
}

/*******************************************************
 *
 * Function name: MDMSerialRecvByte
 * Description:
 * Parameter:
 *        @mode:
 * Return:
 **********************************************************/
void MDMSerialSWRecv_Send(uint8_t mode) {

}

/*******************************************************
 *
 * Function name: MDMSerialSendBytes
 * Description:
 * Parameter:
 *        @bytes:
 *        @num:
 * Return:
 **********************************************************/
void MDMSerialSendBytes(uint8_t *bytes, uint16_t num) {
    modbus_uart_send_bytes(bytes, num);
}
