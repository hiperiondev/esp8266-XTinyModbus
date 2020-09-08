/**
 * @File name: MD_RTU_APP.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-28
 * @Description: Modbus RTU Slave
 * https://github.com/lotoohe-space/XTinyModbus
 */

#include <stdint.h>

#include "MDS_RTU_APP.h"
#include "MD_RTU_MapTable.h"
#include "MDS_RTU_Fun.h"
#include "MDS_RTU_User_Fun.h"
#include "MDS_RTU_Serial.h"
#define SLAVE_ADDR    0x01

void MDSAPPWriteFunciton(void *obj, uint16_t modbusAddr, uint16_t wLen, AddrType addrType);

static uint16_t mapTableData0[32] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
static MapTableItem mapTableItem0 = {
        .modbusAddr = 0x0000,
        .modbusData = mapTableData0,
        .modbusDataSize = 32,
        .addrType = HOLD_REGS_TYPE
};
static uint16_t mapTableData2[32] = { 11, 21, 31, 41, 51, 61, 71, 81, 91, 101, 111, 121 };
static MapTableItem mapTableItem2 = {
        .modbusAddr = 0x0000,
        .modbusData = mapTableData2,
        .modbusDataSize = 32,
        .addrType = INPUT_REGS_TYPE
};

static uint16_t mapTableData1[4] = { 0 };
static MapTableItem mapTableItem1 = {
        .modbusAddr = 0x0000,
        .modbusData = mapTableData1,
        .modbusDataSize = 64,
        .addrType = COILS_TYPE
};

static uint16_t mapTableData3[4] = { 0x5555, 0x5555 };
static MapTableItem mapTableItem3 = {
        .modbusAddr = 0x0000,
        .modbusData = mapTableData3,
        .modbusDataSize = 64,
        .addrType = INPUT_TYPE
};

static ModbusS_RTU modbusS_RTU = { 0 };

bool MDS_RTU_APPInit(void) {

    MDS_RTU_Init(&modbusS_RTU, MDSInitSerial, SLAVE_ADDR, 115200, 8, 1, 0);

    if (MDS_RTU_AddMapItem(&modbusS_RTU, &mapTableItem0) == false) {
        return false;
    }
    if (MDS_RTU_AddMapItem(&modbusS_RTU, &mapTableItem1) == false) {
        return false;
    }
    if (MDS_RTU_AddMapItem(&modbusS_RTU, &mapTableItem2) == false) {
        return false;
    }
    if (MDS_RTU_AddMapItem(&modbusS_RTU, &mapTableItem3) == false) {
        return false;
    }

    MDS_RTU_SetWriteListenFun(&modbusS_RTU, MDSAPPWriteFunciton);
    return true;
}

static void MDS_RTU_UserUpdate(void) {
    uint16_t temp = 0xff;
    uint16_t temp1[] = { 1, 2, 3, 4, 5 };
    MDS_RTU_WriteCoils(&modbusS_RTU, 1, 5, &temp);
    MDS_RTU_WriteHoldReg(&modbusS_RTU, 11, temp);
    MDS_RTU_WriteHoldRegs(&modbusS_RTU, 5, 5, temp1);
}

void MDSAPPWriteFunciton(void *obj, uint16_t modbusAddr, uint16_t wLen, AddrType addrType) {
    uint16_t data[8];
    if ((&modbusS_RTU) != obj) {
        return;
    }

    switch (addrType) {
    case COILS_TYPE:
        MDS_RTU_ReadCoils(obj, modbusAddr, wLen, (uint8_t*) data);
        break;
    case INPUT_TYPE:
        break;
    case HOLD_REGS_TYPE:
        MDS_RTU_ReadHoldRegs(obj, modbusAddr, wLen < 8 ? wLen : 8, data);
        break;
    case INPUT_REGS_TYPE:
        break;
    }
}

void MDS_RTU_Loop(void) {
    MDS_RTU_Process(&modbusS_RTU);
    MDS_RTU_UserUpdate();
}
