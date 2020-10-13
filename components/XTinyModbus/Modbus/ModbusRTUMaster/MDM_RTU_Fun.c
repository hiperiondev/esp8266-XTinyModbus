/********************************************************************************
 * @File name: MDM_RTU_Fun.c
 * @Author: zspace
 * @Email: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU host function module
 * Open source address: https://github.com/lotoohe-space/XTinyModbus
 ********************************************************************************/

/*********************************THE HEADER FILE CONTAINS************************************/
#include "MDM_RTU_Fun.h"
#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_CRC16.h"
/*********************************END******************************************/

/*********************************FUNCTION DECLARATION************************************/
#if !MDM_USE_SEND_CACHE
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8 byte);
#endif
void MDM_RTU_RecvByte(void *obj, uint8 byte);
/*********************************END******************************************/

/*******************************************************
 * Function name :MDM_RTU_Init
 * Description: Modbus RTU Host initialization
 * Parameter:
 *         @pModbusRTU            Host object pointer
 *         @mdRTUSerialInitFun    Serial port initialization function
 *         @baud                  Baud rate
 *         @dataBits              Data bit
 *         @stopBits              Stop bit
 *         @parity                Parity bit
 * Return: reference[MDError]
 **********************************************************/
MDError MDM_RTU_Init(PModbus_RTU pModbusRTU, MD_RTU_SerialInit mdRTUSerialInitFun, uint32 baud, uint8 dataBits,
        uint8 stopBits, uint8 parity) {
    float T;
    uint8 i;
    if (pModbusRTU == NULL) {
        return ERR_VOID;
    }

    MDInitQueue(&(pModbusRTU->mdSqQueue));
    for (i = 0; i < MDM_REG_COIL_ITEM_NUM; i++) {
        pModbusRTU->pMapTableList[i] = NULL;
    }
    TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction = MDM_RTU_TimeHandler;
    // Data sending and receiving related functions
    TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction = NULL;
    TO_MDBase(pModbusRTU)->mdRTURecByteFunction = MDM_RTU_RecvByte;
    TO_MDBase(pModbusRTU)->mdRTURecSendConv = NULL;
#if MDM_USE_SEND_CACHE
    pModbusRTU->serialSendCount = 0;
#endif
    // The time of the last reception, 0xFFFFFFF means that the detection frame has not started
    pModbusRTU->lastTimesTick = 0xFFFFFFFF;
    // Current real-time time unit 100US
    pModbusRTU->timesTick = 0;

    T = (1.0 / (float) baud) * 100000;
    uint16 time = 0;
    time = T * (dataBits + (parity ? 1 : 0));
    if (stopBits == 0) {
        time += T;
    } else if (stopBits == 1) {
        time += T * 1.5f;
    } else if (stopBits == 2) {
        time += T * 2;
    }
    pModbusRTU->frameIntervalTime = time; // This parameter needs to be set according to the baud rate

    pModbusRTU->recvFlag = 0; // Receive flag
    if (mdRTUSerialInitFun != NULL) {
        mdRTUSerialInitFun(pModbusRTU, baud, dataBits, stopBits, parity);
    }

    return ERR_NONE;
}

/*******************************************************
 * Function name: MDM_RTU_CB_Init
 * Description: Modbus RTU host sends the control block initialization,
 *     the control block mainly contains information maintenance
 *     such as the time and frequency of slave transmission
 * Parameter:
 *         @pModbusRTUCB        Send control block object pointer
 *         @pModbusRTU          Host object pointer
 *         @sendIntervalTime    Send interval time
 *         @sendOverTime        Send timeout
 *         @RTTimes             Number of retransmissions
 * Return: None
 **********************************************************/
void MDM_RTU_CB_Init(PModbus_RTU_CB pModbusRTUCB, PModbus_RTU pModbusRTU, uint32 sendIntervalTime, uint32 sendOverTime,
        uint8 RTTimes) {
    if (pModbusRTUCB == NULL) {
        return;
    }
    pModbusRTUCB->sendIntervalTime = sendIntervalTime;
    pModbusRTUCB->pModbus_RTU = pModbusRTU;
    pModbusRTUCB->sendTimeTick = 0;
    pModbusRTUCB->sendOverTime = sendOverTime;
    pModbusRTUCB->RTTimes = RTTimes;
    pModbusRTUCB->sendFlag = 0;
    pModbusRTU->parentObj = NULL;
}

/*******************************************************
 * Function name: MDM_RTU_CB_OverTimeReset
 * Description: Send control block timeout reset
 * Parameter:
 *         @pModbusRTUCB    Send control block object pointer
 * Return: None
 **********************************************************/
void MDM_RTU_CB_OverTimeReset(PModbus_RTU_CB pModbusRTUCB) {
    if (pModbusRTUCB == NULL) {
        return;
    }
    pModbusRTUCB->RTCount = 0;
    pModbusRTUCB->sendFlag = 0;
}

/*******************************************************
 * Function name :MDM_RTU_CB_ClrDisFlag
 * Description: Clear disconnection signs
 * Parameter:
 *         @pModbusRTUCB    Send control block object pointer
 * Return: None
 **********************************************************/
void MDM_RTU_CB_ClrDisFlag(PModbus_RTU_CB pModbusRTUCB) {
    if (pModbusRTUCB == NULL) {
        return;
    }
    MD_CB_CLR_DIS_FLAG(pModbusRTUCB);
}

/*******************************************************
 * Function name: MDM_RTU_CB_SetDisPollEnFlag
 * Description: Set the offline polling enable flag bit.
 * Parameter:
 *         @pModbusRTUCB    Send control block object pointer.
 *         @state           The state to be set.
 * Return: None
 **********************************************************/
void MDM_RTU_CB_SetDisPollEnFlag(PModbus_RTU_CB pModbusRTUCB, BOOL state) {
    if (pModbusRTUCB == NULL) {
        return;
    }
    if (state) {
        MD_CB_SET_DIS_FLAG_EN(pModbusRTUCB);
    } else {
        MD_CB_CLR_DIS_FLAG_EN(pModbusRTUCB);
    }
}

/*******************************************************
 * Function name: MDM_RTU_TimeHandler
 * Description: Timing processing function, timing unit 100US
 * Parameter:
 *         @obj    Host object pointer
 * Return: None
 **********************************************************/
void MDM_RTU_TimeHandler(void *obj) {
    PModbus_RTU pModbusRTU = obj;
    if (!pModbusRTU) {
        return;
    }
    pModbusRTU->timesTick++;
    // No need to deal with
    if (pModbusRTU->lastTimesTick == 0xFFFFFFFF) {
        return;
    }
    if (pModbusRTU->timesTick - pModbusRTU->lastTimesTick >= pModbusRTU->frameIntervalTime) {
        if (pModbusRTU->CRC16Update != 0x0000) {
            // CRC error
            MDResetQueue(&(pModbusRTU->mdSqQueue));
            pModbusRTU->lastTimesTick = 0xFFFFFFFF;
            return;
        }
        // End of one frame
        pModbusRTU->recvFlag = 1;
        pModbusRTU->lastTimesTick = 0xFFFFFFFF;
    }
}

/*******************************************************
 * Function name: MDM_RTU_RecvByte
 * Description: This function receives data and puts it in the queue
 * Parameter:
 *         @obj     Host object pointer
 *         @byte    A byte received
 * Return: None
 **********************************************************/
void MDM_RTU_RecvByte(void *obj, uint8 byte) {
    PModbus_RTU pModbusRTU = obj;
    if (!pModbusRTU) {
        return;
    }
    if (MDenQueue(&(pModbusRTU->mdSqQueue), byte) == FALSE) {
        return;
    }
    if (pModbusRTU->lastTimesTick == 0xFFFFFFFF) {
        pModbusRTU->CRC16Update = 0xFFFF;
    }
    pModbusRTU->CRC16Update = MD_CRC16Update(pModbusRTU->CRC16Update, byte);
    // Save the timestamp of the last character received
    pModbusRTU->lastTimesTick = pModbusRTU->timesTick;
}

#if !MDM_USE_SEND_CACHE
/*******************************************************
 * Function name: MDM_RTU_SendByte
 * Description: Send one byte
 * Parameter:
 *        @pModbus_RTU    Host structure pointer
 *        @byte           Data
 * Return: None
 **********************************************************/
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8 byte) {
    if(!pModbus_RTU) {return;}
    TO_MDBase(pModbus_RTU)->mdRTUSendBytesFunction(&byte,1);
}
#endif

/*******************************************************
 * Function name: MDM_RTU_AddMapItem
 * Description: This function adds a mapping record to the discrete mapping table
 * Parameter:
 *         @pModbusRTU      Host structure pointer
 *         @pRegCoilItem    Items to be added
 * Return: None
 **********************************************************/
BOOL MDM_RTU_AddMapItem(PModbus_RTU pModbusRTU, PMapTableItem pMapTableItem) {
    if (pModbusRTU == NULL || pMapTableItem == NULL) {
        return FALSE;
    }
    return MapTableAdd(pModbusRTU->pMapTableList, pMapTableItem, MDM_REG_COIL_ITEM_NUM);
}

/*******************************************************
 * Function name: MDM_RTU_ReadByte
 * Description: Get data from the receiving queue
 * Parameter:
 *         @pModbus_RTU    Host object pointer
 *         @res            Get the data cache
 *         @len            The length of the data obtained
 * Return: None
 **********************************************************/
MDError MDM_RTU_ReadByte(PModbus_RTU pModbusRTU, uint8 *res, uint8 len) {
    uint8 i;
    if (res == NULL) {
        return ERR_VOID;
    }
    for (i = 0; i < len; i++) {
        if (!MDdeQueue(&(pModbusRTU->mdSqQueue), (res + i))) {
            return ERR_QUEUE;
        }
    }
    return ERR_NONE;
}

/*******************************************************
 * Function name: MDM_RTU_ReadUint16
 * Description: Get data from the receiving queue
 * Parameter:
 *         @pModbus_RTU    Host object pointer
 *         @res            Get data cache
 *         @len            The length of the data obtained
 * Return: None
 **********************************************************/
MDError MDM_RTU_ReadUint16(PModbus_RTU pModbusRTU, uint16 *res, uint8 len) {
    uint8 i;
    uint8 byte;
    if (res == NULL) {
        return ERR_VOID;
    }
    for (i = 0; i < len; i++) {
        if (!MDdeQueue(&(pModbusRTU->mdSqQueue), &byte)) {
            return ERR_QUEUE;
        }
        res[i] = byte << 8;
        if (!MDdeQueue(&(pModbusRTU->mdSqQueue), &byte)) {
            return ERR_QUEUE;
        }
        res[i] |= byte;
    }
    return ERR_NONE;
}

/*******************************************************
 * Function name: MDM_RTU_ReadFun
 * Description: This function sends commands related to the read function
 * Parameter:
 *         @pModbus_RTU    Host object pointer
 *         @funCode        function code
 *         @slaveAddr      Slave address
 *         @startAddr      Read start address
 *         @numOf          Number of reads
 * Return: None
 **********************************************************/
static void MDM_RTU_ReadFun(PModbus_RTU pModbus_RTU, uint8 funCode, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU, slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU, funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(numOf));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(numOf));
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}

/*******************************************************
 * Function name: MDM_RTU_WriteSingleFun
 * Description: Write single coil and register function
 * Parameter:
 *         @pModbus_RTU    Host object pointer
 *         @funCode        function code
 *         @slaveAddr      Slave address
 *         @startAddr      Write start address
 *         @value          Value written
 * Return: None
 **********************************************************/
static void MDM_RTU_WriteSingleFun(PModbus_RTU pModbus_RTU, uint8 funCode, uint8 slaveAddr, uint16 startAddr,
        uint16 value) {
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU, slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU, funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(value));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(value));
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}

/*******************************************************
 * Function name: MDM_RTU_WriteFun
 * Description: Write multiple coils and multiple registers
 * Parameter:
 *         @pModbus_RTU    Host object pointer
 *         @funCode        function code
 *         @slaveAddr      Slave address
 *         @startAddr      Write start address
 *         @numOf          Number of data written
 *         @data           Data written
 * Return: None
 **********************************************************/
static void MDM_RTU_WriteFun(PModbus_RTU pModbus_RTU, uint8 funCode, uint8 slaveAddr, uint16 startAddr, uint8 numOf,
        uint8 *data) {
    uint16 i;
    uint8 wLen = 0;
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU, slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU, funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_H_BYTE(numOf));
    MEM_RTU_EN_QUEUE(pModbus_RTU, MD_L_BYTE(numOf));
    if (funCode == 15) {
        wLen = (numOf >> 3) + ((numOf % 8) ? 1 : 0);
    } else if (funCode == 16) {
        wLen = numOf << 1;
    }
    MEM_RTU_EN_QUEUE(pModbus_RTU, wLen);
    if (funCode == 15) {
        for (i = 0; i < wLen; i++) {
            MEM_RTU_EN_QUEUE(pModbus_RTU, data[i]);
        }
    } else if (funCode == 16) {
        uint16 *tempData = (uint16*) data;
        for (i = 0; i < numOf; i++) {
            MEM_RTU_EN_QUEUE(pModbus_RTU, (tempData[i] >> 8));
            MEM_RTU_EN_QUEUE(pModbus_RTU, (tempData[i] & 0xff));
        }
    }
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}

/*******************************************************
 * Function name: MDM_RTU_InsideWriteBits
 * Description: Write bits to the discrete map
 * Parameter:
 *         @obj           Host object pointer
 *         @modbusAddr    Discretely mapped modbus address
 *         @numOf         Write number
 *         @bit           Data written
 *         @opAddrType    Write address type (COILS_TYPE, INPUT_TYPE), see [AddrType]
 * Return: TRUE, FALSE
 **********************************************************/
BOOL MDM_RTU_InsideWriteBits(void *obj, uint16 modbusAddr, uint16 numOf, uint8 *bit, AddrType opAddrType, uint8 devAddr) {
    uint16 i;
    PModbus_RTU pModbus_RTU = obj;
    if (pModbus_RTU == NULL) {
        return FALSE;
    }
    if (opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE) {
        return FALSE;
    }

    for (i = 0; i < MDM_REG_COIL_ITEM_NUM; i++) {
        if (pModbus_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        // Check the device number
        if (devAddr != pModbus_RTU->pMapTableList[i]->devAddr) {
            continue;
        }

        if (pModbus_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbus_RTU->pMapTableList[i]->modbusAddr + pModbus_RTU->pMapTableList[i]->modbusDataSize)
                        >= (modbusAddr + numOf)) {
            if (pModbus_RTU->pMapTableList[i]->addrType == opAddrType) { //Must be BIT type
                uint16 offsetAddr = modbusAddr - MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                uint16 j;
                for (j = 0; j < numOf; j++) {
                    if (MD_GET_BIT(bit[j >> 3], j % 8)) {
                        MD_SET_BIT(MDS_RTU_REG_COIL_ITEM_DATA( pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4],
                                (j + offsetAddr) % 16);
                    } else {
                        MD_CLR_BIT(MDS_RTU_REG_COIL_ITEM_DATA( pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4],
                                (j + offsetAddr) % 16);
                    }
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*******************************************************
 * Function name: MDM_RTU_InsideWriteRegs
 * Description: Write discrete register
 * Parameter:
 *         @obj           Host object pointer
 *         @modbusAddr    Discretely mapped modbus address
 *         @numOf         Write number
 *         @reg           Data written
 *         @isBigE        Big-endian or little-endian
 *         @opAddrType    Write address type (HOLD_REGS_TYPE, INPUT_REGS_TYPE), see [AddrType]
 * Return: TRUE, FALSE
 **********************************************************/
BOOL MDM_RTU_InsideWriteRegs(void *obj, uint16 modbusAddr, uint16 numOf, uint16 *reg, uint8 isBigE, AddrType opAddrType,
        uint8 devAddr) {
    uint16 i;
    PModbus_RTU pModbus_RTU = obj;
    if (pModbus_RTU == NULL) {
        return FALSE;
    }
    if (opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE) {
        return FALSE;
    }

    for (i = 0; i < MDM_REG_COIL_ITEM_NUM; i++) {
        if (pModbus_RTU->pMapTableList[i] == NULL) {
            continue;
        }
        // Check the device number
        if (devAddr != pModbus_RTU->pMapTableList[i]->devAddr) {
            continue;
        }

        if (pModbus_RTU->pMapTableList[i]->modbusAddr <= modbusAddr
                && (pModbus_RTU->pMapTableList[i]->modbusAddr + pModbus_RTU->pMapTableList[i]->modbusDataSize)
                        >= (modbusAddr + numOf)) {
            if (pModbus_RTU->pMapTableList[i]->addrType == opAddrType) { // ������REG����
                uint16 offsetAddr = modbusAddr - MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                uint16 j = 0;
                for (j = 0; j < numOf; j++) {
                    MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[offsetAddr + j] =
                            isBigE ? MD_SWAP_HL(reg[j]) : reg[j];
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*******************************************************
 * Function name: MDM_RTU_NB_RW
 * Description: Non-blocking read and write
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @funCode           Function code, see [ModbusFunCode]
 *         @slaveAddr         Slave address
 *         @startAddr         Read and write start address
 *         @numOf             Number of read and write data
 *         @wData             If it is a write function code, then it is the written data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_RW(PModbus_RTU_CB pModbus_RTU_CB, ModbusFunCode funCode, uint8 slaveAddr, uint16 startAddr,
        uint16 numOf, void *wData) {
    MDError errRes;
    uint8 index;
    uint16 wAddr;
    errRes = ERR_NONE;
    if (pModbus_RTU_CB == NULL) {
        return ERR_VOID;
    }
    if (pModbus_RTU_CB->pModbus_RTU == NULL) {
        return ERR_VOID;
    }

    if (MD_CB_GET_DIS_FLAG_EN(pModbus_RTU_CB) // Enable the drop-off non-polling flag
    && MD_CB_GET_DIS_FLAG(pModbus_RTU_CB)) { // Device dropped
        return ERR_DEV_DIS;
    }

    if (pModbus_RTU_CB->pModbus_RTU->parentObj != NULL && pModbus_RTU_CB != pModbus_RTU_CB->pModbus_RTU->parentObj) {
        return ERR_IDLE;
    }

    if (pModbus_RTU_CB->sendFlag == 0) { // Has not been sent, or has been sent successfully
        // Clear the receive queue
        MDResetQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue));
        if (funCode >= 1 && funCode <= 4) {
            // Have not sent, then send
            MDM_RTU_ReadFun(pModbus_RTU_CB->pModbus_RTU, funCode, slaveAddr, startAddr, numOf);
        } else if (funCode == 5 || funCode == 6) {
            if (numOf >= 1) { // The length must be greater than or equal to 1
                MDM_RTU_WriteSingleFun(pModbus_RTU_CB->pModbus_RTU, funCode, slaveAddr, startAddr,
                        ((uint16*) (wData))[0]);
            }
        } else if (funCode == 15 || funCode == 16) {
            MDM_RTU_WriteFun(pModbus_RTU_CB->pModbus_RTU, funCode, slaveAddr, startAddr, numOf, (uint8*) (wData));
        }
        // Set the start point of timeout
        pModbus_RTU_CB->sendTimeTick = pModbus_RTU_CB->pModbus_RTU->timesTick;

        // Data has been sent, waiting for timeout detection
        //MD_SET_SENDED_FLAG(pModbus_RTU_CB);
        pModbus_RTU_CB->sendFlag = 1;
        // Set the block to be working
        pModbus_RTU_CB->pModbus_RTU->parentObj = pModbus_RTU_CB;
        return ERR_SEND_FIN;
    } else if (pModbus_RTU_CB->RTCount < pModbus_RTU_CB->RTTimes) { // It has been sent, and there is no sending error

        if (pModbus_RTU_CB->pModbus_RTU->recvFlag) { // Data received
            uint8 byte;
            uint8 funCodeByte = 0;
            // Clear sign
            pModbus_RTU_CB->pModbus_RTU->recvFlag = 0;

            if (!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &byte)) {
                errRes = ERR_DATA_LEN;
                goto _exit;
            }
            // If data is received, process the data
            if (slaveAddr != byte) {
                errRes = ERR_SLAVE_ADDR;
                goto _exit;
            }
            if (!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &funCodeByte)) {
                errRes = ERR_DATA_LEN;
                goto _exit;
            }

            switch (funCodeByte) {
            case 0x1: // Read the coil successfully
            case 0x2: // Read input discrete
            {
                uint16 i;
                MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &byte);
                if ((byte + 2) != MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))) {
                    // Wrong length
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                index = numOf;
                wAddr = startAddr;
                for (i = 0; i < byte; i++) {
                    uint8 rByte;
                    if (!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &rByte)) {
                        // Wrong length
                        errRes = ERR_DATA_LEN;
                        goto _exit;
                    }
                    // Single deposit is less than or equal to 8bit
                    if (!MDM_RTU_InsideWriteBits(pModbus_RTU_CB->pModbus_RTU, wAddr, ((index < 8) ? index : 8), &rByte,
                            (AddrType) funCodeByte, slaveAddr)) {
                        errRes = ERR_DATA_SAVE;
                        goto _exit;
                    }
                    wAddr += ((index < 8) ? index : 8);
                    index -= 8;
                }
                // Release channel
                // pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
                // Received from the machine
                pModbus_RTU_CB->sendFlag = 2;
                errRes = ERR_RW_FIN;
                goto _exit;
            }
            case 0x3: // Read holding register
            case 0x4: // Read input register
            {
                uint16 i;
                uint16 len;
                MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &byte);
                if ((byte + 2) != MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))) {
                    // Wrong length
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                len = byte >> 1;
                for (i = 0; i < len; i++) {
                    uint16 wTemp;
                    uint8 rByte;
                    MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &rByte);
                    wTemp = (rByte << 8);
                    MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue), &rByte);
                    wTemp |= rByte;
                    if (!MDM_RTU_InsideWriteRegs(pModbus_RTU_CB->pModbus_RTU, startAddr + i, 1, &wTemp, 0,
                            (AddrType) funCodeByte, slaveAddr)) {
                        errRes = ERR_DATA_SAVE;
                        goto _exit;
                    }
                }
                pModbus_RTU_CB->sendFlag = 2;
                errRes = ERR_RW_FIN;
                goto _exit;
            }

            case 0x5: // Write single coil
            case 0x6: // Write a single register
            {
                uint16 res;
                if (MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU, &res, 1) != ERR_NONE) {
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                // Return address mismatch error
                if (res != startAddr) {
                    errRes = ERR_WRITE_COIL;
                    goto _exit;
                }
                if (MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU, &res, 1) != ERR_NONE) {
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                // Return data does not match
                if (res != *((uint16*) wData)) {
                    errRes = ERR_WRITE_COIL;
                    goto _exit;
                }
                pModbus_RTU_CB->sendFlag = 2;
                errRes = ERR_RW_FIN;
                goto _exit;
            }
            case 0x0F: // Write multiple coils
            case 0x10: { // Write multiple registers
                uint16 res;
                if (MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU, &res, 1) != ERR_NONE) {
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                // Return address mismatch error
                if (res != startAddr) {
                    errRes = ERR_WRITE_COIL;
                    goto _exit;
                }
                if (MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU, &res, 1) != ERR_NONE) {
                    errRes = ERR_DATA_LEN;
                    goto _exit;
                }
                // Return data does not match
                if (res != numOf) {
                    errRes = ERR_WRITE_COIL;
                    goto _exit;
                }
                pModbus_RTU_CB->sendFlag = 2;
                errRes = ERR_RW_FIN;
                goto _exit;
            }
            case 0x81:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_READ_COIL;
                goto _exit;
                // Abnormal reading coil
            case 0x82:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_READ_INPUT;
                goto _exit;
                // Read input discrete quantity abnormal
            case 0x83:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_READ_HOLD_REG;
                goto _exit;
                // Read holding register error
            case 0x84:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_READ_INPUT_REG;
                goto _exit;
                // Error reading input register
            case 0x85:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_WRITE_COIL;
                goto _exit;
                // Write single coil error
            case 0x86:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_WRITE_REG;
                goto _exit;
                // Error writing order
            case 0x8F:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_WRITE_COIL;
                goto _exit;
                // Write multiple coil error
            case 0x90:
                pModbus_RTU_CB->sendFlag = 3;
                errRes = ERR_WRITE_REG;
                goto _exit;
                // Write multiple register error
            }

        }
        if (pModbus_RTU_CB->sendFlag == 2) { // If the sending is successful, send again
            // Timing resend processing
            if (pModbus_RTU_CB->pModbus_RTU->timesTick - pModbus_RTU_CB->sendTimeTick
                    >= pModbus_RTU_CB->sendIntervalTime) {
                // Resend
                //MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
                pModbus_RTU_CB->sendFlag = 0;
                // Release channel
                //pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
                errRes = ERR_NONE;
                goto _exit;
            }
        } else if (pModbus_RTU_CB->sendFlag == 1 || pModbus_RTU_CB->sendFlag == 3) { // Has been sent, but has not received feedback, then perform timeout detection
            // Receive timeout detection
            if (pModbus_RTU_CB->pModbus_RTU->timesTick - pModbus_RTU_CB->sendTimeTick >= pModbus_RTU_CB->sendOverTime) {
                // Set the start point of timeout
                pModbus_RTU_CB->sendTimeTick = pModbus_RTU_CB->pModbus_RTU->timesTick;
                // Resend
                pModbus_RTU_CB->sendFlag = 0;
                // Increase the number of retransmissions by 1
                pModbus_RTU_CB->RTCount++;

                // Release channel
                pModbus_RTU_CB->pModbus_RTU->parentObj = NULL;

                if (pModbus_RTU_CB->RTCount >= pModbus_RTU_CB->RTTimes) {
                    // The number of retransmissions exceeded
                    errRes = ERR_RW_OV_TIME_ERR;
                    if (MD_CB_GET_DIS_FLAG_EN(pModbus_RTU_CB)) { // No polling when disconnected is enabled
                        MD_CB_SET_DIS_FLAG(pModbus_RTU_CB); // Device disconnection
                    }
                    goto _exit;
                }
                // Timed out
                errRes = ERR_OVER_TIME;
                goto _exit;
            }
        }
    } else {
        // The number of retransmissions exceeded
        errRes = ERR_RW_OV_TIME_ERR;
        goto _exit;
    }

    if (pModbus_RTU_CB->sendFlag == 1) { // If it has been sent, it will return the sending completion flag
        return ERR_SEND_FIN;
    }
    _exit:
    //MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
    //if(errRes == ERR_RW_FIN){
    //    pModbus_RTU_CB->RTCount=0;
    //}
    // Release channel
    pModbus_RTU_CB->pModbus_RTU->parentObj = NULL;
    return errRes;
}

/*******************************************************
 * Function name: MDM_RTU_RW
 * Description: Blocking read and write
 * Parameter:
 *         @pModbus_RTU_CB     Write control block object pointer
 *         @funCode            Function code, see [ModbusFunCode]
 *         @slaveAddr          Slave address
 *         @startAddr          Read and write start address
 *         @numOf              Number of read and write data
 *         @wData              If it is a write function code, then it is the written data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_RW(PModbus_RTU_CB pModbus_RTU_CB, ModbusFunCode funCode, uint8 slaveAddr, uint16 startAddr,
        uint16 numOf, void *wData) {
    MDError res;
    void *tempObj = pModbus_RTU_CB->pModbus_RTU->parentObj;
    pModbus_RTU_CB->pModbus_RTU->parentObj = NULL; // Set to empty, so that non-blocking and blocking can be mixed calls
    do {
        res = MDM_RTU_NB_RW(pModbus_RTU_CB, funCode, slaveAddr, startAddr, numOf, wData);
        if (res != ERR_RW_FIN) { // An error occurred
            if (res == ERR_RW_OV_TIME_ERR) { // Retransmission timed out
                MDM_RTU_CB_OverTimeReset(pModbus_RTU_CB); // Enable retransmission
                goto exit;
            } else if (res == ERR_DEV_DIS) {
                goto exit;
            }
        }
    } while (res != ERR_RW_FIN);
    pModbus_RTU_CB->pModbus_RTU->parentObj = tempObj; // Restore settings
    exit: return res;
}

/*******************************************************
 * Function name: MDM_RTU_ReadCoil
 * Description: Read coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_READ_COIL, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadInput
 * Description: Read input
 * Parameter         :
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_ReadInput(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_READ_INPUT, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadHoldReg
 * Description: Read holding register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_READ_HOLD_REG, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadInputReg
 * Description: Read input register
 * Parameter         :
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_READ_INPUT_REG, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_WriteSingleCoil
 * Description: Write a single coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_WriteSingleCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, BOOL boolVal) {
    uint16 temp;
    temp = boolVal ? 0xFF00 : 0x0000;
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_WRITE_SIN_COIL, slaveAddr, startAddr, 1, (void*) (&temp));
}

/*******************************************************
 * Function name: MDM_RTU_WriteSingleReg
 * Description: Write a single register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_WriteSingleReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 val) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_WRITE_SIN_REG, slaveAddr, startAddr, 1, (void*) (&val));
}

/*******************************************************
 * Function name: MDM_RTU_WriteCoils
 * Description: Write coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of write data
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_WriteCoils(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf, uint8 *val) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_WRITE_COILS, slaveAddr, startAddr, numOf, (void*) (val));
}

/*******************************************************
 * Function name: MDM_RTU_WriteRegs
 * Description: Write register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of write data
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_WriteRegs(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf, uint16 *val) {
    return MDM_RTU_RW(pModbus_RTU_CB, MDM_WRITE_REGS, slaveAddr, startAddr, numOf, (void*) (val));
}

/*******************************************************
 * Function name: MDM_RTU_ReadCoil
 * Description: Non-blocking read coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_READ_COIL, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadInput
 * Description: Non-blocking read input
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_ReadInput(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_READ_INPUT, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadHoldReg
 * Description: Non-blocking read holding register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_READ_HOLD_REG, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_ReadHoldReg
 * Description: Non-blocking read input register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of read data
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_READ_INPUT_REG, slaveAddr, startAddr, numOf, NULL);
}

/*******************************************************
 * Function name: MDM_RTU_WriteSingleCoil
 * Description: Non-blocking write single coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_WriteSingleCoil(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, BOOL boolVal) {
    uint16 temp;
    temp = boolVal ? 0xFF00 : 0x0000;
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_WRITE_SIN_COIL, slaveAddr, startAddr, 1, (void*) (&temp));
}

/*******************************************************
 * Function name: MDM_RTU_WriteSingleReg
 * Description: Non-blocking write to a single register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_WriteSingleReg(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 val) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_WRITE_SIN_REG, slaveAddr, startAddr, 1, (void*) (&val));
}

/*******************************************************
 * Function name: MDM_RTU_WriteCoils
 * Description: Non-blocking write coil
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of write data
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_WriteCoils(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf,
        uint8 *val) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_WRITE_COILS, slaveAddr, startAddr, numOf, (void*) (val));
}

/*******************************************************
 * Function name: MDM_RTU_WriteRegs
 * Description: Non-blocking write register
 * Parameter:
 *         @pModbus_RTU_CB    Write control block object pointer
 *         @slaveAddr         Slave address
 *         @startAddr         Read start address
 *         @numOf             Number of write data
 *         @boolVal           TRUE, FALSE
 * Return: See [MDError]
 **********************************************************/
MDError MDM_RTU_NB_WriteRegs(PModbus_RTU_CB pModbus_RTU_CB, uint8 slaveAddr, uint16 startAddr, uint16 numOf,
        uint16 *val) {
    return MDM_RTU_NB_RW(pModbus_RTU_CB, MDM_WRITE_REGS, slaveAddr, startAddr, numOf, (void*) (val));
}
