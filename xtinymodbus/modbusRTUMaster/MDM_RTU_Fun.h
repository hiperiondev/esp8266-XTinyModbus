/********************************************************************************
 * @File name: MD_RTU_Fun.h
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 ********************************************************************************/

#ifndef _MEM_RTU_FUN_H__
#define _MEM_RTU_FUN_H__

#include <stdbool.h>
#include "MD_RTU_Queue.h"
#include "MD_RTU_Error.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_MapTable.h"
#include "MD_RTU_Config.h"

/****************************************************************/
/*modbus rtu*/
typedef enum {
         READ_COIL_MDM = 1,
        READ_INPUT_MDM = 2,
     READ_HOLD_REG_MDM = 3,
    READ_INPUT_REG_MDM = 4,
    WRITE_SIN_COIL_MDM = 5,
     WRITE_SIN_REG_MDM = 6,
       WRITE_COILS_MDM = 15,
        WRITE_REGS_MDM = 16
} ModbusFunCode;

typedef struct {
       ModbusBase modbusBase;
    PMapTableItem pMapTableList[MDM_REG_COIL_ITEM_NUM];
        MDSqQueue mdSqQueue;
#if MDM_USE_SEND_CACHE
          uint8_t serialSendCache[MDM_RTU_SEND_CACHE_SIZE];
         uint16_t serialSendCount;
#endif
         uint32_t lastTimesTick;
         uint32_t timesTick;
         uint16_t frameIntervalTime;
         uint16_t CRC16Update;
             void *parentObj;
          uint8_t recvFlag;
} *PModbus_RTU, Modbus_RTU;

typedef struct {
    PModbus_RTU pModbus_RTU;
       uint32_t sendIntervalTime;
       uint32_t sendTimeTick;
        uint8_t RTCount;
       uint32_t sendOverTime;
        uint8_t RTTimes;
        uint8_t sendFlag;
} *PModbus_RTU_CB, Modbus_RTU_CB;

MDError MDM_RTU_Init(PModbus_RTU pModbusRTU, MD_RTU_SerialInit mdRTUSerialInitFun, uint32_t baud,
        uint8_t dataBits, uint8_t stopBits, uint8_t parity);

   void MDM_RTU_CB_Init(PModbus_RTU_CB pModbusRTUCB, PModbus_RTU pModbusRTU, uint32_t sendIntervalTime,
        uint32_t sendOverTime, uint8_t RTTimes);
/**********************************************************************/

/*************************************************************/
#if MDM_USE_SEND_CACHE
#define MEM_RTU_START_EN(a)      {uint16_t CRCUpdate=0xFFFF;(a)->serialSendCount=0
#define MEM_RTU_EN_QUEUE(a,b)    (a)->serialSendCache[(a)->serialSendCount++]=(b);\
    CRCUpdate=MD_CRC16Update(CRCUpdate,(b))
#define MEM_RTU_END_EN(a)        a->serialSendCache[a->serialSendCount++]=(uint8_t)(CRCUpdate);\
    a->serialSendCache[a->serialSendCount++]=(uint8_t)(CRCUpdate>>8);\
    (TO_MDBase(a))->mdRTUSendBytesFunction(a->serialSendCache,a->serialSendCount);}
#else
#define MEM_RTU_START_EN()       {uint16_t CRCUpdate=0xFFFF;
#define MEM_RTU_EN_QUEUE(a,b)    MDM_RTU_SendByte((a),(b));\
CRCUpdate=MD_CRC16Update(CRCUpdate,(b))
#define MEM_RTU_END_EN(a)        MDM_RTU_SendByte(a,(uint8_t)CRCUpdate);\
    MDM_RTU_SendByte(a,(uint8_t)(CRCUpdate>>8));\
}
#endif
/***********************************************************************/

/***************************************************************/
/*00us*/
   void MDM_RTU_TimeHandler(void *obj);
   void MDM_RTU_RecvByte(void *obj, uint8_t byte);
   void MDM_RTU_CB_OverTimeReset(PModbus_RTU_CB pModbusRTUCB);
   bool MDM_RTU_AddMapItem(PModbus_RTU pModbusRTU, PMapTableItem pRegCoilItem);
MDError MDM_RTU_ReadByte(PModbus_RTU pModbusRTU, uint8_t *res, uint8_t len);
MDError MDM_RTU_ReadUint16(PModbus_RTU pModbusRTU, uint16_t *res, uint8_t len);
MDError MDM_RTU_NB_RW(PModbus_RTU_CB pModbus_RTU_CB, ModbusFunCode funCode, uint8_t slaveAddr,
        uint16_t startAddr, uint16_t numOf, void *wData);
MDError MDM_RTU_RW(PModbus_RTU_CB pModbus_RTU_CB, ModbusFunCode funCode, uint8_t slaveAddr,
        uint16_t startAddr, uint16_t numOf, void *wData);
MDError MDM_RTU_NB_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_NB_ReadInput(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_NB_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_NB_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr,
        uint16_t startAddr, uint16_t numOf);
MDError MDM_RTU_NB_WriteSingleCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr,
        uint16_t startAddr, bool boolVal);
MDError MDM_RTU_NB_WriteSingleReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr,
        uint16_t startAddr, uint16_t val);
MDError MDM_RTU_NB_WriteCoils(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf, uint8_t *val);
MDError MDM_RTU_NB_WriteRegs(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf, uint16_t *val);
MDError MDM_RTU_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_ReadInput(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf);
MDError MDM_RTU_WriteSingleCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr,
        uint16_t startAddr, bool boolVal);
MDError MDM_RTU_WriteSingleReg(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t val);
MDError MDM_RTU_WriteCoils(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf, uint8_t *val);
MDError MDM_RTU_WriteRegs(PModbus_RTU_CB pModbus_RTU_CB, uint8_t slaveAddr, uint16_t startAddr,
        uint16_t numOf, uint16_t *val);
/************************************************************************/

#endif
