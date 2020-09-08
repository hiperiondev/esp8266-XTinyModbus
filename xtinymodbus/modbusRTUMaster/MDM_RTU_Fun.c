/********************************************************************************
* @File name: MDM_RTU_Fun.c
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU
* https://github.com/lotoohe-space/XTinyModbus
********************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
/*****************************************************************/
#include "MDM_RTU_Fun.h"
#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_CRC16.h"
#include "modbus_uart.h"
/*************************************************************************/

/****************************************************************/
#if !MDM_USE_SEND_CACHE
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8_t byte);
#endif
void MDM_RTU_RecvByte(void *obj,uint8_t byte);
/*************************************************************************/

/*******************************************************
*
* Function name :MDM_RTU_Init
* Description        :Modbus RTU
* Parameter         :
*        @pModbusRTU
*        @mdRTUSerialInitFun
*        @baud
*        @dataBits
*        @stopBits
*        @parity
* Return          :[MDError]
**********************************************************/
MDError MDM_RTU_Init(
    PModbus_RTU pModbusRTU,
    MD_RTU_SerialInit mdRTUSerialInitFun,
    uint32_t baud,
    uint8_t dataBits,
    uint8_t stopBits,
    uint8_t parity
){
    float T;
    uint8_t i;
    if(pModbusRTU==NULL){return ERR_VOID;}

    MDInitQueue(&(pModbusRTU->mdSqQueue));
    for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
        pModbusRTU->pMapTableList[i] = NULL;
    }
    TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction=MDM_RTU_TimeHandler;

    TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction=NULL;
    TO_MDBase(pModbusRTU)->mdRTURecByteFunction=MDM_RTU_RecvByte;
    TO_MDBase(pModbusRTU)->mdRTURecSendConv=NULL;
#if MDM_USE_SEND_CACHE
    pModbusRTU->serialSendCount=0;
#endif
    /*0xFFFFFFF*/
    pModbusRTU->lastTimesTick=0xFFFFFFFF;
    /*100US*/
    pModbusRTU->timesTick=0;

    T=(1.0/(float)baud)*100000;
    uint16_t time=0;
    time=T*(dataBits+(parity?1:0));
    if(stopBits==0){
        time+=T;
    }else if(stopBits==1){
        time+=T*1.5f;
    }else if(stopBits==2){
        time+=T*2;
    }
    pModbusRTU->frameIntervalTime=time;

    pModbusRTU->recvFlag=0;
    if(mdRTUSerialInitFun!=NULL){
        mdRTUSerialInitFun(pModbusRTU,baud, dataBits,stopBits,parity);
    }

    return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_CB_Init
* Description        :Modbus RTU
* Parameter         :
*        @pModbusRTUCB
*        @pModbusRTU
*        @sendIntervalTime
*        @sendOverTime
*        @RTTimes
* Return          :
**********************************************************/
void MDM_RTU_CB_Init(PModbus_RTU_CB pModbusRTUCB, PModbus_RTU pModbusRTU, uint32_t sendIntervalTime,
        uint32_t sendOverTime, uint8_t RTTimes) {
    if(pModbusRTUCB==NULL){return ;}
    pModbusRTUCB->sendIntervalTime=sendIntervalTime;
    pModbusRTUCB->pModbus_RTU=pModbusRTU;
    pModbusRTUCB->sendTimeTick=0;
    pModbusRTUCB->sendOverTime=sendOverTime;
    pModbusRTUCB->RTTimes=RTTimes;
    pModbusRTUCB->sendFlag=0;
    pModbusRTU->parentObj=NULL;
}
/*******************************************************
*
* Function name :MDM_RTU_CB_OverTimeReset
* Description        :
* Parameter         :
*        @pModbusRTUCB
* Return          :
**********************************************************/
void MDM_RTU_CB_OverTimeReset(PModbus_RTU_CB     pModbusRTUCB){
    if(pModbusRTUCB==NULL){return ;}
    pModbusRTUCB->RTCount=0;
    pModbusRTUCB->sendFlag=0;
}
/*******************************************************
*
* Function name :MDM_RTU_TimeHandler
* Description        :100US
* Parameter         :
*        @obj
* Return          :
**********************************************************/
void MDM_RTU_TimeHandler(void *obj){
    PModbus_RTU pModbusRTU=obj;
    if(!pModbusRTU){ return; }
    pModbusRTU->timesTick++;
    if(pModbusRTU->lastTimesTick==0xFFFFFFFF){return ;}
    if(pModbusRTU->timesTick-pModbusRTU->lastTimesTick>=pModbusRTU->frameIntervalTime){
        if(pModbusRTU->CRC16Update!=0x0000){
            /*CRC*/
            MDResetQueue(&(pModbusRTU->mdSqQueue));
            pModbusRTU->lastTimesTick=0xFFFFFFFF;
            return ;
        }
        pModbusRTU->recvFlag=1;
        pModbusRTU->lastTimesTick=0xFFFFFFFF;
    }
}
/*******************************************************
*
* Function name :MDM_RTU_RecvByte
* Description        :
* Parameter         :
*        @obj
*        @byte
* Return          :
**********************************************************/
void MDM_RTU_RecvByte(void *obj,uint8_t byte){
    PModbus_RTU pModbusRTU=obj;
    if(!pModbusRTU){ return; }
    if(MDenQueue(&(pModbusRTU->mdSqQueue),byte)==false){
        return ;
    }
    if(pModbusRTU->lastTimesTick==0xFFFFFFFF){
        pModbusRTU->CRC16Update=0xFFFF;
    }
    pModbusRTU->CRC16Update=MD_CRC16Update(pModbusRTU->CRC16Update,byte);
    pModbusRTU->lastTimesTick=pModbusRTU->timesTick;
}
#if !MDM_USE_SEND_CACHE
/*******************************************************
*
* Function name :MDM_RTU_SendByte
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @byte
* Return          :
**********************************************************/
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8_t byte){
    if(!pModbus_RTU){ return; }
    TO_MDBase(pModbus_RTU)->mdRTUSendBytesFunction(&byte,1);
}
#endif
/*******************************************************
*
* Function name :MDM_RTU_AddMapItem
* Description        :
* Parameter         :
*        @pModbusRTU
*        @pRegCoilItem
* Return          :
**********************************************************/
bool MDM_RTU_AddMapItem(PModbus_RTU pModbusRTU,PMapTableItem pMapTableItem){
    if(pModbusRTU==NULL || pMapTableItem==NULL){
            return false;
    }
    return MapTableAdd(pModbusRTU->pMapTableList, pMapTableItem,MDM_REG_COIL_ITEM_NUM);
}

/*******************************************************
*
* Function name :MDM_RTU_ReadByte
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @res
*        @len
* Return          :
**********************************************************/
MDError MDM_RTU_ReadByte(PModbus_RTU pModbusRTU,uint8_t *res,uint8_t len){
    uint8_t i;
    if(res==NULL){return ERR_VOID;}
    for(i=0;i<len;i++){
        if(!MDdeQueue(&(pModbusRTU->mdSqQueue),(res+i))){
            return ERR_QUEUE;
        }
    }
    return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadUint16
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @res
*        @len
* Return          :
**********************************************************/
MDError MDM_RTU_ReadUint16(PModbus_RTU pModbusRTU,uint16_t *res,uint8_t len){
    uint8_t i;
    uint8_t byte;
    if(res==NULL){return ERR_VOID;}
    for(i=0;i<len;i++){
        if(!MDdeQueue(&(pModbusRTU->mdSqQueue),&byte)){
            return ERR_QUEUE;
        }
        res[i]=byte<<8;
        if(!MDdeQueue(&(pModbusRTU->mdSqQueue),&byte)){
            return ERR_QUEUE;
        }
        res[i]|=byte;
    }
    return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadFun
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @funCode
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          :
**********************************************************/
static void MDM_RTU_ReadFun(PModbus_RTU pModbus_RTU,uint8_t funCode,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(numOf));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(numOf));
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleFun
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @funCode
*        @slaveAddr
*        @startAddr
*        @value
* Return          :
**********************************************************/
static void MDM_RTU_WriteSingleFun(PModbus_RTU pModbus_RTU,uint8_t funCode,uint8_t slaveAddr,uint16_t startAddr,uint16_t value){
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(value));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(value));
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_WriteFun
* Description        :
* Parameter         :
*        @pModbus_RTU
*        @funCode
*        @slaveAddr
*        @startAddr
*        @numOf
*        @data
* Return          :
**********************************************************/
static void MDM_RTU_WriteFun(PModbus_RTU pModbus_RTU,
    uint8_t funCode,uint8_t slaveAddr,uint16_t startAddr,uint8_t numOf,uint8_t *data){
    uint16_t i;
    uint8_t wLen = 0;
    MD_RTU_SEND_MODE(pModbus_RTU);
    MEM_RTU_START_EN(pModbus_RTU);
    MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
    MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(numOf));
    MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(numOf));
    if(funCode==15){
        wLen=(numOf>>3) + ((numOf%8)?1:0);
    }else if(funCode==16){
        wLen=numOf<<1;
    }
    MEM_RTU_EN_QUEUE(pModbus_RTU,wLen);
    if(funCode == 15){
        for(i=0;i<wLen;i++){
            MEM_RTU_EN_QUEUE(pModbus_RTU,data[i]);
        }
    }else if(funCode == 16){
        uint16_t *tempData= (uint16_t*)data;
        for(i=0;i<numOf;i++){
            MEM_RTU_EN_QUEUE(pModbus_RTU,(tempData[i]>>8));
            MEM_RTU_EN_QUEUE(pModbus_RTU,(tempData[i]&0xff));
        }
    }
    MEM_RTU_END_EN(pModbus_RTU);
    MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_InsideWriteBits
* Description        :
* Parameter         :
*        @obj
*        @modbusAddr
*        @numOf
*        @bit
*        @opAddrType      (COILS_TYPE,INPUT_TYPE) [AddrType]
* Return          : true , false
**********************************************************/
bool MDM_RTU_InsideWriteBits(
    void* obj,
    uint16_t modbusAddr,
    uint16_t numOf,
    uint8_t *bit,
    AddrType opAddrType,
    uint8_t devAddr
){
    uint16_t i;
    PModbus_RTU pModbus_RTU = obj;
    if(pModbus_RTU==NULL){return false;}
    if(opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE){return false;}

    for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
        if(pModbus_RTU->pMapTableList[i]==NULL){
            continue;
        }
        if(devAddr!=pModbus_RTU->pMapTableList[i]->devAddr){continue;}

        if(pModbus_RTU->pMapTableList[i]->modbusAddr<=modbusAddr&&
        (pModbus_RTU->pMapTableList[i]->modbusAddr+pModbus_RTU->pMapTableList[i]->modbusDataSize)>=(modbusAddr+numOf)
        ){
            if(pModbus_RTU->pMapTableList[i]->addrType==opAddrType){
                uint16_t offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                uint16_t j;
                for(j=0;j<numOf;j++){
                    if(
                        MD_GET_BIT( bit[j>>3] ,j%8)
                    ){
                        MD_SET_BIT(
                            MDS_RTU_REG_COIL_ITEM_DATA(
                            pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4]
                        ,(j+offsetAddr)%16);
                    }else{
                        MD_CLR_BIT(
                            MDS_RTU_REG_COIL_ITEM_DATA(
                            pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4]
                        ,(j+offsetAddr)%16);
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
* Function name :MDM_RTU_InsideWriteRegs
* Description        :
* Parameter         :
*        @obj
*        @modbusAddr�ַ
*        @numOf
*        @reg
*        @isBigE
*        @opAddrType (HOLD_REGS_TYPE,INPUT_REGS_TYPE) [AddrType]
* Return          : true , false
**********************************************************/
bool MDM_RTU_InsideWriteRegs(
void* obj,
uint16_t modbusAddr,
uint16_t numOf, 
uint16_t *reg,
uint8_t isBigE, 
AddrType opAddrType,
uint8_t devAddr
){
    uint16_t i;
    PModbus_RTU pModbus_RTU = obj;
    if(pModbus_RTU==NULL){return false;}
    if(opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE){return false;}

    for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
        if(pModbus_RTU->pMapTableList[i]==NULL){
            continue;
        }
        if(devAddr!=pModbus_RTU->pMapTableList[i]->devAddr){continue;}

        if(pModbus_RTU->pMapTableList[i]->modbusAddr<=modbusAddr&&
        (pModbus_RTU->pMapTableList[i]->modbusAddr+pModbus_RTU->pMapTableList[i]->modbusDataSize)>=(modbusAddr+numOf)
        ){
            if(pModbus_RTU->pMapTableList[i]->addrType==opAddrType){
                uint16_t offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
                uint16_t j=0;
                for(j=0;j<numOf;j++){
                    MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[offsetAddr+j]=
                    isBigE?MD_SWAP_HL(reg[j]):reg[j];
                }
                return true;
            }
        }
    }
    return false;
}
/*******************************************************
*
* Function name :MDM_RTU_NB_RW
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @funCode [ModbusFunCode]
*        @slaveAddr
*        @startAddr
*        @numOf
*        @wData
* Return          :[MDError]
**********************************************************/
MDError MDM_RTU_NB_RW(
    PModbus_RTU_CB pModbus_RTU_CB,
    ModbusFunCode funCode,
    uint8_t slaveAddr,
    uint16_t startAddr,
    uint16_t numOf,
    void *wData
){
    MDError errRes;
    uint8_t index;
    uint16_t wAddr;
    errRes = ERR_NONE;
    if(pModbus_RTU_CB==NULL){return ERR_VOID;}
    if(pModbus_RTU_CB->pModbus_RTU==NULL){return ERR_VOID;}

    if(    pModbus_RTU_CB->pModbus_RTU->parentObj!=NULL &&
            pModbus_RTU_CB!=pModbus_RTU_CB->pModbus_RTU->parentObj){
            return ERR_IDLE;
    }

    if(pModbus_RTU_CB->sendFlag==0){
        MDResetQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue));
        if(funCode>=1 && funCode<=4){
            MDM_RTU_ReadFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,numOf);
        }
        else if(funCode==5||funCode==6){
            if(numOf>=1){
                MDM_RTU_WriteSingleFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,((uint16_t*)(wData))[0]);
            }
        }else if(funCode==15||funCode==16){
            MDM_RTU_WriteFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,numOf,(uint8_t*)(wData));
        }
        pModbus_RTU_CB->sendTimeTick=pModbus_RTU_CB->pModbus_RTU->timesTick;

        //MD_SET_SENDED_FLAG(pModbus_RTU_CB);
        pModbus_RTU_CB->sendFlag=1;
        pModbus_RTU_CB->pModbus_RTU->parentObj=pModbus_RTU_CB;
        return ERR_SEND_FIN;
    }else if(pModbus_RTU_CB->RTCount<pModbus_RTU_CB->RTTimes){

        if(pModbus_RTU_CB->pModbus_RTU->recvFlag){
            uint8_t byte;
            uint8_t funCodeByte=0;
            pModbus_RTU_CB->pModbus_RTU->recvFlag=0;

            if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte)){
                errRes =  ERR_DATA_LEN;
                goto _exit;
            }

            if(slaveAddr!=byte){
                errRes =  ERR_SLAVE_ADDR;
                goto _exit;
            }
            if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&funCodeByte)){
                errRes =  ERR_DATA_LEN;
                goto _exit;
            }

            switch(funCodeByte){
                case 0x1:
                case 0x2:
                {
                    uint16_t i;
                    MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte);
                    if((byte+2)!=MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))){
                        errRes =  ERR_DATA_LEN;
                        goto _exit;
                    }
                    index = numOf;
                    wAddr=startAddr;
                    for(i=0;i<byte;i++){
                        uint8_t rByte;
                        if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte)){
                                errRes =  ERR_DATA_LEN;
                                goto _exit;
                        }
                        if(!MDM_RTU_InsideWriteBits(pModbus_RTU_CB->pModbus_RTU,wAddr,((index<8)?index:8), &rByte,(AddrType)funCodeByte,slaveAddr)){
                            errRes= ERR_DATA_SAVE;
                            goto _exit;
                        }
                        wAddr += ((index<8)?index:8);
                        index-=8;
                    }
//                    pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
                        pModbus_RTU_CB->sendFlag=2;
                    errRes= ERR_RW_FIN;
                    goto _exit;
                }
                case 0x3:
                case 0x4:
                {
                    uint16_t i;
                    uint16_t len;
                    MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte);
                    if((byte+2)!=MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))){
                        errRes= ERR_DATA_LEN;
                        goto _exit;
                    }
                    len=byte>>1;
                    for(i=0;i<len ;i++){
                        uint16_t     wTemp;
                        uint8_t        rByte;
                        MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte);
                        wTemp=(rByte<<8);
                        MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte);
                        wTemp|=rByte;
                        if(!MDM_RTU_InsideWriteRegs(pModbus_RTU_CB->pModbus_RTU,startAddr+i,1,&wTemp,0,(AddrType)funCodeByte,slaveAddr)){
                            errRes= ERR_DATA_SAVE;
                            goto _exit;
                        }
                    }
                        pModbus_RTU_CB->sendFlag=2;
                    errRes= ERR_RW_FIN;
                    goto _exit;
                }

                case 0x5:
                case 0x6:
                {
                    uint16_t res;
                    if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
                        errRes= ERR_DATA_LEN;
                        goto _exit;
                    }
                    if(res!=startAddr){
                        errRes= ERR_WRITE_COIL;
                        goto _exit;
                    }
                    if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
                        errRes= ERR_DATA_LEN;
                        goto _exit;
                    }
                    if(res!=*((uint16_t*)wData)){
                        errRes= ERR_WRITE_COIL;
                        goto _exit;
                    }
                        pModbus_RTU_CB->sendFlag=2;
                    errRes= ERR_RW_FIN;
                    goto _exit;
                }
                case 0x0F:
                case 0x10:{
                    uint16_t res;
                    if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
                        errRes= ERR_DATA_LEN;
                        goto _exit;
                    }
                    if(res!=startAddr){
                        errRes= ERR_WRITE_COIL;
                        goto _exit;
                    }
                    if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
                        errRes= ERR_DATA_LEN;
                        goto _exit;
                    }

                    if(res!=numOf){
                        errRes= ERR_WRITE_COIL;
                        goto _exit;
                    }
                    pModbus_RTU_CB->sendFlag=2;
                    errRes= ERR_RW_FIN;
                    goto _exit;
                }
                case 0x81: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_COIL; goto _exit;
                case 0x82: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_INPUT; goto _exit;
                case 0x83: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_HOLD_REG; goto _exit;
                case 0x84: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_INPUT_REG; goto _exit;
                case 0x85: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_COIL; goto _exit;
                case 0x86: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_REG; goto _exit;
                case 0x8F: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_COIL; goto _exit;
                case 0x90: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_REG; goto _exit;
            }

        }
        if(pModbus_RTU_CB->sendFlag==2){
            if(pModbus_RTU_CB->pModbus_RTU->timesTick-pModbus_RTU_CB->sendTimeTick
                >=pModbus_RTU_CB->sendIntervalTime){
                //MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
                pModbus_RTU_CB->sendFlag=0;
    //            pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
                errRes= ERR_NONE;
                goto _exit;
            }
        }else if(pModbus_RTU_CB->sendFlag==1 || pModbus_RTU_CB->sendFlag==3){
            if(pModbus_RTU_CB->pModbus_RTU->timesTick-pModbus_RTU_CB->sendTimeTick
                >=pModbus_RTU_CB->sendOverTime){
                    pModbus_RTU_CB->sendTimeTick=pModbus_RTU_CB->pModbus_RTU->timesTick;
                    pModbus_RTU_CB->sendFlag=0;
                    pModbus_RTU_CB->RTCount++;
                    pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;

                    if(pModbus_RTU_CB->RTCount>=pModbus_RTU_CB->RTTimes){
                        errRes= ERR_RW_OV_TIME_ERR;
                        goto _exit;
                    }
                    errRes= ERR_OVER_TIME;
                    goto _exit;
            }
        }
    }else {
        errRes= ERR_RW_OV_TIME_ERR;
        goto _exit;
    }

    if(pModbus_RTU_CB->sendFlag==1){
        return ERR_SEND_FIN;
    }
_exit:
//    MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
//    if(errRes == ERR_RW_FIN){
//        pModbus_RTU_CB->RTCount=0;
//    }
    pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
    return errRes;
}
/*******************************************************
*
* Function name :MDM_RTU_RW
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @funCode [ModbusFunCode]
*        @slaveAddr
*        @startAddr
*        @numOf
*        @wData
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_RW(
    PModbus_RTU_CB pModbus_RTU_CB,
    ModbusFunCode funCode,
    uint8_t slaveAddr,
    uint16_t startAddr,
    uint16_t numOf,
    void *wData
){
    MDError res;
    do{
        res = MDM_RTU_NB_RW(pModbus_RTU_CB,funCode,slaveAddr,startAddr,numOf,wData);
        if(res != ERR_RW_FIN){
            if(res == ERR_RW_OV_TIME_ERR){
                MDM_RTU_CB_OverTimeReset(pModbus_RTU_CB);
                return res;
            }
        }
    }while(res!=ERR_RW_FIN);
    //pModbus_RTU_CB->sendFlag=0;
    return res;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadCoil
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_RW(pModbus_RTU_CB,READ_COIL_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInput
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_ReadInput(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_RW(pModbus_RTU_CB,READ_INPUT_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_RW(pModbus_RTU_CB,READ_HOLD_REG_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInputReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_RW(pModbus_RTU_CB,READ_INPUT_REG_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleCoil
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_WriteSingleCoil(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,bool boolVal){
    uint16_t temp;
    temp=boolVal?0xFF00:0x0000;
    return MDM_RTU_RW(pModbus_RTU_CB,WRITE_SIN_COIL_MDM,slaveAddr,startAddr,1,(void*)(&temp));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_WriteSingleReg(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t val){
    return MDM_RTU_RW(pModbus_RTU_CB,WRITE_SIN_REG_MDM,slaveAddr,startAddr,1,(void*)(&val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteCoils
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_WriteCoils(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf,uint8_t* val){
    return MDM_RTU_RW(pModbus_RTU_CB,WRITE_COILS_MDM,slaveAddr,startAddr,numOf,(void*)(val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteRegs
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_WriteRegs(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf,uint16_t* val){
    return MDM_RTU_RW(pModbus_RTU_CB,WRITE_REGS_MDM,slaveAddr,startAddr,numOf,(void*)(val));
};

/*******************************************************
*
* Function name :MDM_RTU_ReadCoil
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_COIL_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInput
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadInput(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_INPUT_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_HOLD_REG_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_INPUT_REG_MDM,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleCoil
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteSingleCoil(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,bool boolVal){
    uint16_t temp;
    temp=boolVal?0xFF00:0x0000;
    return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_SIN_COIL_MDM,slaveAddr,startAddr,1,(void*)(&temp));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleReg
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteSingleReg(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t val){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_SIN_REG_MDM,slaveAddr,startAddr,1,(void*)(&val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteCoils
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteCoils(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf,uint8_t* val){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_COILS_MDM,slaveAddr,startAddr,numOf,(void*)(val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteRegs
* Description        :
* Parameter         :
*        @pModbus_RTU_CB
*        @slaveAddr
*        @startAddr
*        @numOf
*        @boolVal      true , false  
* Return          : [MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteRegs(
    PModbus_RTU_CB pModbus_RTU_CB,uint8_t slaveAddr,uint16_t startAddr,uint16_t numOf,uint16_t* val){
    return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_REGS_MDM,slaveAddr,startAddr,numOf,(void*)(val));
};
