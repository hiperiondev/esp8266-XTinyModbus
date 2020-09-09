/********************************************************************************
 * @File name: MD_RTU_User_Fun.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 * */
/************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MDM_RTU_User_Fun.h"
#include "MDM_RTU_Fun.h"

/*******************************************************
 *
 * Function name: MDM_RTU_ReadBits
 * Description
 * Parameter:
 *        @obj
 *        @modbusAddr:
 *        @numOf:
 *        @opAddrType:
 *        @devAddr:
 * Return:
 *        @res: true success, false fail
 **********************************************************/
bool MDM_RTU_ReadBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res,
        AddrType opAddrType, uint8_t devAddr) {
    uint16_t i;
    PModbus_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE) {
        return false;
    }

    for (i = 0; i < MDM_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (devAddr != pModbusS_RTU->pMapTableList[i]->devAddr) {
            continue;
        }

        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + numOf)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {
                uint16_t j;
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                for (j = offsetAddr; j < offsetAddr + numOf; j++) {
                    if (MD_GET_BIT(MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[j>>4],
                            j % 16)) {
                        MD_SET_BIT(res[j >> 3], j % 8);
                    } else {
                        MD_CLR_BIT(res[j >> 3], j % 8);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

/*******************************************************
 *
 * Function name: MDM_RTU_ReadRegs
 * Description:
 * Parameter:
 *        @obj
 *        @modbusAddr
 *        @numOf
 *        @opAddrType
 *        @devAddr
 * Return:
 *        @res: true success, false fail
 **********************************************************/
bool MDM_RTU_ReadRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res,
        AddrType opAddrType, uint8_t devAddr) {
    uint16_t i;
    PModbus_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE) {
        return false;
    }

    for (i = 0; i < MDM_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }

        if (devAddr != pModbusS_RTU->pMapTableList[i]->devAddr) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + numOf)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {
                uint16_t j;
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                for (j = 0; j < numOf; j++) {
                    res[j] = MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[offsetAddr
                            + j];
                }
                return true;
            }
        }
    }
    return false;
}
