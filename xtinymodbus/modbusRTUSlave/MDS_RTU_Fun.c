/********************************************************************************
 * @File name: MD_RTU_Fun.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU Slave
 ********************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
/*********************************************************************/

#include "MDS_RTU_Fun.h"
#include "MD_RTU_CRC16.h"
#include "MDS_RTU_Serial.h"
#include "MDS_RTU_User_Fun.h"
/*************************************************************************/

void MDS_RTU_RecvByte(void *obj, uint8_t byte);
void MDS_RTU_TimeHandler(void *obj);

static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU, ANLCode anlCode, ErrorCode errCode);
uint8_t MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU, uint16_t reg, uint16_t regNum,
        uint8_t funCode);
uint8_t MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU, uint16_t reg, uint16_t regNum,
        uint8_t funCode, uint16_t *data, uint8_t byteCount);
/*************************************************************************/

/*******************************************************
 *
 * Function name :MDS_RTU_Init
 * Description        :
 * Parameter         :
 *        @pModbusRTU
 *        @mdRTUSerialInitFun
 *        @salveAddr
 *        @baud
 *        @dataBits
 *        @stopBits
 *        @parity
 * Return          :
 **********************************************************/
void MDS_RTU_Init(PModbusS_RTU pModbusRTU, MD_RTU_SerialInit mdRTUSerialInitFun, uint8_t salveAddr,
        uint32_t baud, uint8_t dataBits, uint8_t stopBits, uint8_t parity) {
    uint8_t i;
    float T;
    if (pModbusRTU == NULL) {
        return;
    }
    MDInitQueue(&(pModbusRTU->mdSqQueue));
    MDInitQueue(&(pModbusRTU->mdMsgSqQueue));
    pModbusRTU->salveAddr = salveAddr;
    pModbusRTU->serialReadCount = 0;
#if MDS_USE_SEND_CACHE
    pModbusRTU->serialSendCount = 0;
#endif
    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        pModbusRTU->pMapTableList[i] = NULL;
    }

    TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction = MDS_RTU_TimeHandler;
    TO_MDBase(pModbusRTU)->mdRTURecByteFunction = MDS_RTU_RecvByte;
    TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction = NULL;
    TO_MDBase(pModbusRTU)->mdRTURecSendConv = NULL;

    pModbusRTU->mdsWriteFun = NULL;
    pModbusRTU->lastTimesTick = 0xFFFFFFFF;
    pModbusRTU->lastSendTimes = 0x00000000;
    pModbusRTU->timesTick = 0x00000000;

    T = (1.0 / (float) baud) * 100000; // 100us
    uint16_t time = 0;
    time = T * (dataBits + (parity ? 1 : 0));
    if (stopBits == 0) {
        time += T;
    } else if (stopBits == 1) {
        time += T * 1.5f;
    } else if (stopBits == 2) {
        time += T * 2;
    }
    pModbusRTU->frameIntervalTime = time;/**/

    pModbusRTU->CRC16Update = 0xFFFF;

    if (mdRTUSerialInitFun != NULL) {
        mdRTUSerialInitFun(pModbusRTU, baud, dataBits, stopBits, parity);
    }
    return;
}
/*******************************************************
 *
 * Function name :MDS_RTU_SetWriteListenFun
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 *        @wFun
 * Return          :
 **********************************************************/
void MDS_RTU_SetWriteListenFun(PModbusS_RTU pModbus_RTU, MDSWriteFunciton wFun) {
    if (pModbus_RTU == NULL) {
        return;
    }
    pModbus_RTU->mdsWriteFun = wFun;
}
/*******************************************************
 *
 * Function name :MDS_RTU_TimeHandler
 * Description        :100US
 * Parameter         :
 *        @obj
 * Return          :
 **********************************************************/
void MDS_RTU_TimeHandler(void *obj) {
    uint32_t tempTick = 0;
    uint8_t overFlag = 0;
    PModbusS_RTU pModbusRTU = obj;
    if (!pModbusRTU) {
        return;
    }

    pModbusRTU->timesTick++;

    if (pModbusRTU->timesTick == 0xFFFFFFFF) {/* pModbusRTU->lastTimesTick==0xFFFFFFFF*/
        tempTick = 0xFFFFFFFF - pModbusRTU->lastSendTimes;
        pModbusRTU->timesTick = tempTick; /**/
        pModbusRTU->lastSendTimes = 0;/**/
        overFlag = 1;
    }

    if (pModbusRTU->lastTimesTick == 0xFFFFFFFF) {
        return;
    }/**/
    if (overFlag) { /**/
        pModbusRTU->timesTick += 0xFFFFFFFF - pModbusRTU->lastTimesTick;/**/
        pModbusRTU->lastTimesTick = tempTick;
    }
    if ((pModbusRTU->timesTick - pModbusRTU->lastTimesTick >= pModbusRTU->frameIntervalTime)) {
        uint16_t msgLen;
        uint16_t i;
        uint8_t byte;
        if (pModbusRTU->CRC16Update != 0x0000) {
            /*CRC*/
            MDResetQueue(&(pModbusRTU->mdSqQueue));
            pModbusRTU->lastTimesTick = 0xFFFFFFFF;
            return;
        }
        /**/
        /**/
        msgLen = MDQueueLength(&(pModbusRTU->mdSqQueue));
        for (i = 0; i < msgLen; i++) {
            /**/
            if (MDdeQueue(&(pModbusRTU->mdSqQueue), &byte) == false) {
                return;
            }
            /**/
            if (MDenQueue(&(pModbusRTU->mdMsgSqQueue), byte) == false) {
                return;
            }
        }
        pModbusRTU->lastTimesTick = 0xFFFFFFFF;
    }
}

/*******************************************************
 *
 * Function name :MDS_RTU_RecvByte
 * Description        :
 * Parameter         :
 *        @obj
 *        @byte
 * Return          :
 **********************************************************/
void MDS_RTU_RecvByte(void *obj, uint8_t byte) {
    PModbusS_RTU pModbusRTU = obj;
    if (!pModbusRTU) {
        return;
    }
    /**/
    if (MDenQueue(&(pModbusRTU->mdSqQueue), byte) == false) {
        return;
    }
    if (pModbusRTU->lastTimesTick == 0xFFFFFFFF) {
        pModbusRTU->CRC16Update = 0xFFFF;
    }
    pModbusRTU->CRC16Update = MD_CRC16Update(pModbusRTU->CRC16Update, byte);
    /**/
    pModbusRTU->lastTimesTick = pModbusRTU->timesTick;
}
/*******************************************************
 *
 * Function name :MDS_RTU_AddMapItem
 * Description        :
 * Parameter         :
 *        @obj
 *        @byte
 * Return          :
 **********************************************************/
bool MDS_RTU_AddMapItem(PModbusS_RTU pModbusRTU, PMapTableItem pMapTableItem) {
    if (pModbusRTU == NULL || pMapTableItem == NULL) {
        return false;
    }
    return MapTableAdd(pModbusRTU->pMapTableList, pMapTableItem, MDS_REG_COIL_ITEM_NUM);
}
#if    !MDS_USE_SEND_CACHE
/*******************************************************
*
* Function name :MDS_RTU_SendByte
* Description        :
* Parameter         :
*        @pModbusRTU
*        @byte
* Return          : true success , false fail
**********************************************************/
static void MDS_RTU_SendByte(PModbusS_RTU pModbusRTU,uint8 byte){
    if(!pModbusRTU){ return; }
    TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction(&byte,1);
}
#endif
/*******************************************************
 *
 * Function name :MDS_RTU_SerialProcess
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 * Return          :
 **********************************************************/
static bool MDS_RTU_SerialProcess(PModbusS_RTU pModbus_RTU) {
    uint8_t byte;
    if (!pModbus_RTU) {
        return false;
    }

    /**/
    if (MDdeQueue(&(pModbus_RTU->mdMsgSqQueue), &byte) == false) {
        return false;
    }
    pModbus_RTU->serialReadCache[pModbus_RTU->serialReadCount] = byte;/*save a byte*/

    if (pModbus_RTU->serialReadCount >= 1) {/**/
        if (pModbus_RTU->serialReadCache[1] >= 0x01 && pModbus_RTU->serialReadCache[1] <= 0x04) {
            /**/
            if (pModbus_RTU->serialReadCount > 5) {
                /*REG REG_NUM*/
                if (pModbus_RTU->serialReadCount >= 7) {
                    pModbus_RTU->serialReadCount++;
                    /**/
                    return true;
                }
            }
        } else if (pModbus_RTU->serialReadCache[1] == 5 || pModbus_RTU->serialReadCache[1] == 6) {
            /**/
            /**/
            if (pModbus_RTU->serialReadCount >= 7) {
                pModbus_RTU->serialReadCount++;
                /**/
                return true;
            }
        } else if (pModbus_RTU->serialReadCache[1] == 15 || pModbus_RTU->serialReadCache[1] == 16) {
            /**/
            if (pModbus_RTU->serialReadCount > 6) {
                if (pModbus_RTU->serialReadCount >= (6 + pModbus_RTU->serialReadCache[6] + 2)) {
                    pModbus_RTU->serialReadCount++;
                    /**/
                    return true;
                }
            }
        } else {/**/

        }
    }

    if (pModbus_RTU->serialReadCount <= MDS_RTU_CMD_SIZE) {
        pModbus_RTU->serialReadCount++;
    }

    return false;
}
/*******************************************************
 *
 * Function name :MDS_RTU_Process
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 * Return          :
 **********************************************************/
void MDS_RTU_Process(PModbusS_RTU pModbus_RTU) {
    bool res;
    if (!pModbus_RTU) {
        return;
    }

    if (pModbus_RTU->timesTick - pModbus_RTU->lastSendTimes < pModbus_RTU->frameIntervalTime) {
        /*3.5T*/
        return;
    }

    res = MDS_RTU_SerialProcess(pModbus_RTU);/**/
    if (!res) {
        return;
    }

    if (pModbus_RTU->serialReadCache[0] != 0x00 /**/
    && pModbus_RTU->serialReadCache[0] != pModbus_RTU->salveAddr) {
        /**/
        goto __exit;
    }

    /*01-04*/
    /*01 ON/OFF)*/
    /*02 (ON/OFF)*/
    /*03 */
    /*04 */
    if (MDS_RTU_FUN_CODE(pModbus_RTU) >= 0x1 && MDS_RTU_FUN_CODE(pModbus_RTU) <= 0x4) {
        uint16_t startReg = MDS_RTU_START_REG(pModbus_RTU);
        uint16_t regNum = MDS_RTU_REGS_NUM(pModbus_RTU);
        MDS_RTU_ReadDataProcess(pModbus_RTU, startReg, regNum, MDS_RTU_FUN_CODE(pModbus_RTU));
    } else if (
    MDS_RTU_FUN_CODE(pModbus_RTU) == 0x5 ||
    MDS_RTU_FUN_CODE(pModbus_RTU) == 0x6) {
        /*05 */
        /*06 */
        uint16_t startReg = MDS_RTU_START_REG(pModbus_RTU);
        uint16_t val = ((pModbus_RTU->serialReadCache[4]) << 8) | pModbus_RTU->serialReadCache[5];

        MDS_RTU_WriteDataProcess(pModbus_RTU, startReg, 1, MDS_RTU_FUN_CODE(pModbus_RTU), &val, 2);
    } else if (
    MDS_RTU_FUN_CODE(pModbus_RTU) == 15 ||
    MDS_RTU_FUN_CODE(pModbus_RTU) == 16) {
        /*15 */
        /*16 */
        uint16_t startReg = MDS_RTU_START_REG(pModbus_RTU);
        uint16_t regNum = MDS_RTU_REGS_NUM(pModbus_RTU);
        uint8_t bytesNum = MDS_RTU_BYTES_NUM(pModbus_RTU);
        if (MDS_RTU_FUN_CODE(pModbus_RTU) == 15) {
            if (((regNum >> 3) + ((regNum % 8) ? 1 : 0)) != bytesNum) {
                MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_COILS_MDS, ILLEGAL_DAT_VAL);
                /**/
                goto __exit0;
            }
        } else { //16
            if ((regNum << 1) != bytesNum) {
                MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_REGS_MDS, ILLEGAL_DAT_VAL);
                /**/
                goto __exit0;
            }
        }

        MDS_RTU_WriteDataProcess(pModbus_RTU, startReg, regNum, MDS_RTU_FUN_CODE(pModbus_RTU),
                (uint16_t*) (&(pModbus_RTU->serialReadCache[7])), pModbus_RTU->serialReadCache[6]);
    }

    __exit0: pModbus_RTU->lastSendTimes = pModbus_RTU->timesTick;
    __exit: pModbus_RTU->serialReadCount = 0;

    return;
}

/*******************************************************
 *
 * Function name :MDS_RTU_SendErrorCode
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 *        @anlCode [ANLCode]
 *        @errCode [ErrorCode]
 * Return          :
 **********************************************************/
static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU, ANLCode anlCode, ErrorCode errCode) {
    MD_RTU_SEND_MODE(pModbus_RTU);
    MDS_START_SEND(pModbus_RTU);
    MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
    MDS_SEND_BYTE(pModbus_RTU, anlCode);
    MDS_SEND_BYTE(pModbus_RTU, errCode);
    MDS_SEND_END(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
 *
 * Function name :MDS_RTU_ReadDataProcess
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 *        @reg
 *        @regNum
 *        @funCode
 * Return          : true success , false fail
 **********************************************************/
uint8_t MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU, uint16_t reg, uint16_t regNum,
        uint8_t funCode) {
    uint16_t i = 0;
    if (pModbus_RTU == NULL) {
        return false;
    }

    for (i = 0; i < MDS_REG_COIL_ITEM_NUM; i++) {
        if (pModbus_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        if (pModbus_RTU->pMapTableList[i]->modbusAddr <= reg
                && (pModbus_RTU->pMapTableList[i]->modbusAddr
                        + pModbus_RTU->pMapTableList[i]->modbusDataSize) >= (reg + regNum)) {
            /**/
            if ((funCode == 1 && pModbus_RTU->pMapTableList[i]->addrType == COILS_TYPE)
                    || (funCode == 2 && pModbus_RTU->pMapTableList[i]->addrType == INPUT_TYPE)) {
                /*bit*/
                /**/
                uint16_t offsetAddr = reg
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                uint16_t j;
                uint16_t lastIndex = 0;
                uint8_t tempByte = 0;
                MD_RTU_SEND_MODE(pModbus_RTU);
                MDS_START_SEND(pModbus_RTU);
                MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
                MDS_SEND_BYTE(pModbus_RTU, funCode);
                MDS_SEND_BYTE(pModbus_RTU, (regNum >> 3) + ((regNum % 8) > 0 ? 1 : 0));
                for (j = offsetAddr; j < offsetAddr + regNum; j++) {
                    if (((j - offsetAddr) >> 3) != lastIndex) {
                        MDS_SEND_BYTE(pModbus_RTU, tempByte);
                        tempByte = 0;
                        /**/
                        lastIndex = (j - offsetAddr) >> 3;
                    }
                    if (MD_GET_BIT(MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[j>>4],
                            j % 16)) {
                        MD_SET_BIT(tempByte, j % 8);
                    } else {
                        MD_CLR_BIT(tempByte, j % 8);
                    }
                }
                MDS_SEND_BYTE(pModbus_RTU, tempByte);
                MDS_SEND_END(pModbus_RTU);
                MD_RTU_RECV_MODE(pModbus_RTU);
            } else if ((funCode == 3 && pModbus_RTU->pMapTableList[i]->addrType == HOLD_REGS_TYPE)
                    || (funCode == 4 && pModbus_RTU->pMapTableList[i]->addrType == INPUT_REGS_TYPE)) {
                /*reg*/
                /*uint16*/
                uint16_t j = 0;
                uint16_t offsetAddr = reg
                        - MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                MD_RTU_SEND_MODE(pModbus_RTU);
                MDS_START_SEND(pModbus_RTU);
                MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
                MDS_SEND_BYTE(pModbus_RTU, funCode);
                MDS_SEND_BYTE(pModbus_RTU, regNum << 1);
                for (j = 0; j < regNum << 1; j += 2) {
                    uint16_t temp =
                    MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[offsetAddr + (j >> 1)];
                    MDS_SEND_BYTE(pModbus_RTU, (temp >> 8) & 0xff);
                    MDS_SEND_BYTE(pModbus_RTU, (temp) & 0xff);
                }
                MDS_SEND_END(pModbus_RTU);
                MD_RTU_RECV_MODE(pModbus_RTU);
            } else {
                /**/
                continue;
            }
            return true;
        }
    }
    /**/
    MDS_RTU_SendErrorCode(pModbus_RTU, (ANLCode) (0x80 + funCode), ILLEGAL_DAT_ADDR);
    return false;
}
/*******************************************************
 *
 * Function name :MDS_RTU_WriteDataProcess
 * Description        :
 * Parameter         :
 *        @pModbus_RTU
 *        @reg
 *        @regNum
 *        @funCode
 *        @data
 *        @byteCount
 * Return          : true success , false fail
 **********************************************************/
uint8_t MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU, uint16_t reg, uint16_t regNum,
        uint8_t funCode, uint16_t *data, uint8_t byteCount) {
    uint8_t res = false;
    switch (funCode) {
    case 5:/**/
    {
        if (data[0] == 0xFF00 || data[0] == 0x0000) {
            if (data[0] == 0xFF00) {
                res = MDS_RTU_WriteBit(pModbus_RTU, reg, 1, COILS_TYPE);
            } else {
                res = MDS_RTU_WriteBit(pModbus_RTU, reg, 0, COILS_TYPE);
            }
            if (res) {
                MD_RTU_SEND_MODE(pModbus_RTU);
                MDS_START_SEND(pModbus_RTU);
                MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
                MDS_SEND_BYTE(pModbus_RTU, funCode);
                MDS_SEND_BYTE(pModbus_RTU, (reg >> 8) & 0xff);
                MDS_SEND_BYTE(pModbus_RTU, (reg) & 0xff);
                MDS_SEND_BYTE(pModbus_RTU, ((*data) >> 8) & 0xff);
                MDS_SEND_BYTE(pModbus_RTU, (*data) & 0xff);
                MDS_SEND_END(pModbus_RTU);
                MD_RTU_RECV_MODE(pModbus_RTU);
                if (pModbus_RTU->mdsWriteFun) {
                    pModbus_RTU->mdsWriteFun(pModbus_RTU, reg, 1, COILS_TYPE);
                }
            } else {
                MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_SIN_COIL_MDS, ILLEGAL_DAT_ADDR);
            }
        } else {
            MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_SIN_COIL_MDS, ILLEGAL_DAT_VAL);
        }
    }
        break;
    case 15:/**/
        res = MDS_RTU_WriteBits(pModbus_RTU, reg, regNum, data, COILS_TYPE);
        if (res) {
            MD_RTU_SEND_MODE(pModbus_RTU);
            MDS_START_SEND(pModbus_RTU);
            MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
            MDS_SEND_BYTE(pModbus_RTU, funCode);
            MDS_SEND_BYTE(pModbus_RTU, (reg >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (reg) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, ((regNum) >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (regNum) & 0xff);
            MDS_SEND_END(pModbus_RTU);
            MD_RTU_RECV_MODE(pModbus_RTU);
            if (pModbus_RTU->mdsWriteFun) {
                pModbus_RTU->mdsWriteFun(pModbus_RTU, reg, regNum, COILS_TYPE);
            }
        } else {
            MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_COILS_MDS, ILLEGAL_DAT_ADDR);
        }
        break;
    case 6:/**/
        res = MDS_RTU_WriteReg(pModbus_RTU, reg, data[0], HOLD_REGS_TYPE);
        if (res) {
            MD_RTU_SEND_MODE(pModbus_RTU);
            MDS_START_SEND(pModbus_RTU);
            MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
            MDS_SEND_BYTE(pModbus_RTU, funCode);
            MDS_SEND_BYTE(pModbus_RTU, (reg >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (reg) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, ((*data) >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (*data) & 0xff);
            MDS_SEND_END(pModbus_RTU);
            MD_RTU_RECV_MODE(pModbus_RTU);
            if (pModbus_RTU->mdsWriteFun) {
                pModbus_RTU->mdsWriteFun(pModbus_RTU, reg, 1, HOLD_REGS_TYPE);
            }
        } else {
            MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_SIN_REG_MDS, ILLEGAL_DAT_ADDR);
        }
        break;
    case 16:/**/
        res = MDS_RTU_WriteRegs(pModbus_RTU, reg, regNum, data, 1, HOLD_REGS_TYPE);
        if (res) {
            MD_RTU_SEND_MODE(pModbus_RTU);
            MDS_START_SEND(pModbus_RTU);
            MDS_SEND_BYTE(pModbus_RTU, pModbus_RTU->salveAddr);
            MDS_SEND_BYTE(pModbus_RTU, funCode);
            MDS_SEND_BYTE(pModbus_RTU, (reg >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (reg) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, ((regNum) >> 8) & 0xff);
            MDS_SEND_BYTE(pModbus_RTU, (regNum) & 0xff);
            MDS_SEND_END(pModbus_RTU);
            MD_RTU_RECV_MODE(pModbus_RTU);
            if (pModbus_RTU->mdsWriteFun) {
                pModbus_RTU->mdsWriteFun(pModbus_RTU, reg, regNum, HOLD_REGS_TYPE);
            }
        } else {
            MDS_RTU_SendErrorCode(pModbus_RTU, WRITE_REGS_MDS, ILLEGAL_DAT_ADDR);
        }
        break;
    }
    if (!res) {
        return false;
    }
    return true;
}
