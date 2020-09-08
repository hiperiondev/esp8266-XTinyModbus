/********************************************************************************
 * @File name: MD_RTU_Fun.h
 * @Author: zspace
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU Slave
 ********************************************************************************/

#ifndef _MD_RTU_FUN_H__
#define _MD_RTU_FUN_H__
/**************************/
#include <stdbool.h>
#include <stdint.h>
/***************************************/
#include "MD_RTU_Queue.h"
#include "MD_RTU_MapTable.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_Config.h"
/**********************************************************************/

/****************************************************************/
typedef void (*MDSWriteFunciton)(void *obj, uint16_t modbusAddr, uint16_t wLen, AddrType addrType);

typedef struct {
    ModbusBase modbusBase;
    MDSWriteFunciton mdsWriteFun;
    MDSqQueue mdSqQueue;
    MDSqQueue mdMsgSqQueue;
    uint8_t salveAddr;

    uint8_t serialReadCache[MDS_RTU_CMD_SIZE];
    uint16_t serialReadCount; /**/
#if MDS_USE_SEND_CACHE
    uint8_t serialSendCache[MDS_RTU_SEND_CACHE_SIZE];
    uint16_t serialSendCount;
#endif
    PMapTableItem pMapTableList[MDS_REG_COIL_ITEM_NUM];

    /*0xFFFFFFFF*/
    uint32_t lastTimesTick;
    /*100US*/
    uint32_t timesTick;

    /**/
    uint32_t lastSendTimes;

    /*3.5T*/
    uint16_t frameIntervalTime;

    /*CRC16*/
    uint16_t CRC16Update;

///CRC16
//uint16 CRC16SendUpdate;
} *PModbusS_RTU, ModbusS_RTU;

/**/
typedef enum {
      READ_COIL_ANL_MDS = 0x81,
     READ_INPUT_ANL_MDS = 0x82,
     READ_HOLD_REGS_MDS = 0x83,
    READ_INPUT_REGS_MDS = 0X84,
     WRITE_SIN_COIL_MDS = 0X85,
      WRITE_SIN_REG_MDS = 0x86,
        WRITE_COILS_MDS = 0X8F,
         WRITE_REGS_MDS = 0X90
} ANLCode;

/**/
typedef enum {
    ILLEGAL_FUN = 0x01,
    ILLEGAL_DAT_ADDR = 0x2,
    ILLEGAL_DAT_VAL = 0x3,
    SLAVE_DEV_FAILURE = 0x4,
} ErrorCode;
/***********************************************************************/

/***********************************************************************/
#define MDS_RTU_FUN_CODE(a)                    (((PModbusS_RTU)(a))->serialReadCache[1])
#define MDS_RTU_CRC16(a)                    (((((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount-1])<<8)|\
    (((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount]))

#define MDS_RTU_START_REG(a)                (((((PModbusS_RTU)(a))->serialReadCache[2])<<8)|\
    (((PModbusS_RTU)(a))->serialReadCache[3]))
#define MDS_RTU_REGS_NUM(a)                    (((((PModbusS_RTU)(a))->serialReadCache[4])<<8)|\
    (((PModbusS_RTU)(a))->serialReadCache[5]))
#define MDS_RTU_BYTES_NUM(a)                ((a)->serialReadCache[6])

#if    MDS_USE_SEND_CACHE

#define MDS_START_SEND(a)        {uint16_t CRC16=0xFFFF;\
    a->serialSendCount=0

#define MDS_SEND_BYTE(a,b)    CRC16=MD_CRC16Update(CRC16,(b));\
    a->serialSendCache[a->serialSendCount++]=b

#define MDS_SEND_END(a)            a->serialSendCache[a->serialSendCount++]=(uint8_t)(CRC16);\
    a->serialSendCache[a->serialSendCount++]=(uint8_t)(CRC16>>8);\
(TO_MDBase(a))->mdRTUSendBytesFunction(a->serialSendCache,a->serialSendCount);}
#else
#define MDS_START_SEND(a)        {uint8 CRC16Temp;uint16 CRC16=0xFFFF;
#define MDS_SEND_BYTE(a,b)    CRC16=MD_CRC16Update(CRC16,(b));MDS_RTU_SendByte(a,b)
#define MDS_SEND_END(a)            CRC16Temp=CRC16&0xFF;\
    (TO_MDBase(a))->mdRTUSendBytesFunction((uint8*)(&(CRC16Temp)),1);\
    CRC16Temp=(CRC16>>8)&0xFF;\
    (TO_MDBase(a))->mdRTUSendBytesFunction((uint8*)(&(CRC16Temp)),1);\
}
#endif
/************************************************************************/

/*******************************************************************/
void MDS_RTU_Init(PModbusS_RTU pModbusRTU, MD_RTU_SerialInit mdRTUSerialInitFun, uint8_t salveAddr,
        uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity);
void MDS_RTU_SetWriteListenFun(PModbusS_RTU pModbus_RTU, MDSWriteFunciton wFun);
bool MDS_RTU_AddMapItem(PModbusS_RTU pModbusRTU, PMapTableItem pRegCoilItem);
void MDS_RTU_Process(PModbusS_RTU pModbus_RTU);

/*************************************************************************/

#endif
