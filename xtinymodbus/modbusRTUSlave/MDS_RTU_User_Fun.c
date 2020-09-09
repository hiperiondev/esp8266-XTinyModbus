/********************************************************************************
 * @File name: MD_RTU_User_Fun.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-28
 * @Description: Modbus RTU
 * https://github.com/lotoohe-space/XTinyModbus
 ********************************************************************************/

#include <stdbool.h>
#include "MDS_RTU_User_Fun.h"
#include "MDS_RTU_Fun.h"

/*bits*/
bool MDS_RTU_ReadBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res,
        AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    /*bits*/
    if (opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + numOf)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {/**/
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

bool MDS_RTU_ReadRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res,
        AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    /**/
    if (opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
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

bool MDS_RTU_WriteBit(void *obj, uint16_t modbusAddr, uint8_t bit, AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + 1)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                if (bit) {
                    MD_SET_BIT(
                            MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[offsetAddr>>4],
                            offsetAddr % 16);
                } else {
                    MD_CLR_BIT(
                            MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[offsetAddr>>4],
                            offsetAddr % 16);
                }
                return true;
            }
        }
    }
    return false;
}

bool MDS_RTU_WriteBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit,
        AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + numOf)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {/*BIT*/
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                uint16_t j;
                for (j = 0; j < numOf; j++) {
                    uint8_t *byteData = (uint8_t*) bit;
                    if (MD_GET_BIT(byteData[j >> 3], j % 8)) {
                        MD_SET_BIT(
                                MDS_RTU_REG_COIL_ITEM_DATA( pModbusS_RTU->pMapTableList[i])[(offsetAddr+j)>>4],
                                (j + offsetAddr) % 16);
                    } else {
                        MD_CLR_BIT(
                                MDS_RTU_REG_COIL_ITEM_DATA( pModbusS_RTU->pMapTableList[i])[(offsetAddr+j)>>4],
                                (j + offsetAddr) % 16);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MDS_RTU_WriteReg(void *obj, uint16_t modbusAddr, uint16_t reg, AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + 1)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[offsetAddr] = reg;
                return true;
            }
        }
    }
    return false;
}

bool MDS_RTU_WriteRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg,
        uint8_t isBigE, AddrType opAddrType) {
    uint16_t i;
    PModbusS_RTU pModbusS_RTU = obj;
    if (pModbusS_RTU == NULL) {
        return false;
    }
    if (opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbusS_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbusS_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbusS_RTU->pMapTableList[i]->modbusAddr
                        + pModbusS_RTU->pMapTableList[i]->modbusDataSize) >= (modbusAddr + numOf)) {
            if (pModbusS_RTU->pMapTableList[i]->addrType == opAddrType) {/*REG*/
                uint16_t offsetAddr = modbusAddr
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pMapTableList[i]);
                uint16_t j = 0;
                for (j = 0; j < numOf; j++) {
                    MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pMapTableList[i])[offsetAddr + j] =
                            isBigE ? MD_SWAP_HL(reg[j]) : reg[j];
                }
                return true;
            }
        }
    }
    return false;
}

bool MDS_RTU_WriteCoil(void *obj, uint16_t modbusAddr, uint8_t bit) {
    return MDS_RTU_WriteBit(obj, modbusAddr, bit, COILS_TYPE);
}
bool MDS_RTU_WriteInput(void *obj, uint16_t modbusAddr, uint8_t bit) {
    return MDS_RTU_WriteBit(obj, modbusAddr, bit, INPUT_TYPE);
}
bool MDS_RTU_WriteCoils(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit) {
    return MDS_RTU_WriteBits(obj, modbusAddr, numOf, bit, COILS_TYPE);
}
bool MDS_RTU_WriteInputs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit) {
    return MDS_RTU_WriteBits(obj, modbusAddr, numOf, bit, INPUT_TYPE);
}
bool MDS_RTU_WriteHoldReg(void *obj, uint16_t modbusAddr, uint16_t reg) {
    return MDS_RTU_WriteReg(obj, modbusAddr, reg, HOLD_REGS_TYPE);
}
bool MDS_RTU_WriteHoldRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg) {
    return MDS_RTU_WriteRegs(obj, modbusAddr, numOf, reg, 0, HOLD_REGS_TYPE);
}
bool MDS_RTU_WriteInputReg(void *obj, uint16_t modbusAddr, uint16_t reg) {
    return MDS_RTU_WriteReg(obj, modbusAddr, reg, INPUT_REGS_TYPE);
}
bool MDS_RTU_WriteInputRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg) {
    return MDS_RTU_WriteRegs(obj, modbusAddr, numOf, reg, 0, INPUT_REGS_TYPE);
}
bool MDS_RTU_ReadCoils(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res) {
    return MDS_RTU_ReadBits(obj, modbusAddr, numOf, res, COILS_TYPE);
}
bool MDS_RTU_ReadInput(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res) {
    return MDS_RTU_ReadBits(obj, modbusAddr, numOf, res, INPUT_TYPE);
}
bool MDS_RTU_ReadHoldRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res) {
    return MDS_RTU_ReadRegs(obj, modbusAddr, numOf, res, HOLD_REGS_TYPE);
}
bool MDS_RTU_ReadInputRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res) {
    return MDS_RTU_ReadRegs(obj, modbusAddr, numOf, res, INPUT_REGS_TYPE);
}
