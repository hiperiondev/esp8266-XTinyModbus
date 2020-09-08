/********************************************************************************
* @File name: MD_RTU_APP.c
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
/*******************************************************************/
#include "MDM_RTU_APP.h"
#include "MDM_RTU_Fun.h"
#include "MDM_RTU_Serial.h"
#include "MDM_RTU_User_Fun.h"
/**********************************************************************/

/*************************************************************/
static uint16_t mapTableDataMaster0[4]={0};
static MapTableItem mapTableItemMaster0={
    .modbusAddr=0x0000,
    .modbusData=mapTableDataMaster0,
    .modbusDataSize=64,
    .addrType=COILS_TYPE,
    .devAddr=1,
};
static uint16_t regDataMaster1[32]={1,2,3,4,5,6,7,8,9,10,11,12};
static MapTableItem mapTableItemMaster1={
    .modbusAddr=0x0000,
    .modbusData=regDataMaster1,
    .modbusDataSize=32,
    .addrType=HOLD_REGS_TYPE,
    .devAddr=1,
};

Modbus_RTU modbus_RTU = {0};
Modbus_RTU_CB modbusRWRTUCB = {0};
Modbus_RTU_CB modbusRWRTUCB1 = {0};
Modbus_RTU_CB modbusRWRTUCB2 = {0};
Modbus_RTU_CB modbusRWRTUCB3 = {0};
Modbus_RTU_CB modbusRWRTUCB4 = {0};
/************************************************************************/

/*******************************************************
*
* Function name :MDM_RTU_APPInit
* Description        :
* Parameter         :
* Return          : true success , false fail
**********************************************************/
bool MDM_RTU_APPInit(void){
    if(MDM_RTU_Init(&modbus_RTU,MDMInitSerial,9600,8,1,0)!=ERR_NONE){
        return false;
    }

    if(MDM_RTU_AddMapItem(&modbus_RTU,&mapTableItemMaster0)==false){
        return false;
    }
    if(MDM_RTU_AddMapItem(&modbus_RTU,&mapTableItemMaster1)==false){
        return false;
    }

    MDM_RTU_CB_Init(&modbusRWRTUCB,&modbus_RTU,10000,30000,3);
    MDM_RTU_CB_Init(&modbusRWRTUCB1,&modbus_RTU,10000,30000,3);
    MDM_RTU_CB_Init(&modbusRWRTUCB2,&modbus_RTU,10000,30000,3);
    MDM_RTU_CB_Init(&modbusRWRTUCB3,&modbus_RTU,10000,30000,3);
    MDM_RTU_CB_Init(&modbusRWRTUCB4,&modbus_RTU,10000,30000,3);
    return true;
}
uint16_t    temp=0xAAAA;
uint16_t    temp2=0x5555;
uint16_t    temp1=1234;
uint16_t    data1[]={1,2,3,4,5,6};
#define MD_NB_MODE_TEST 1

static void MDM_RTUUserRead(void){

    uint16_t resTemp;
    #if MD_NB_MODE_TEST
    MDError res;
    res = MDM_RTU_NB_ReadCoil(&modbusRWRTUCB,0x1,0,16);
    if(res != ERR_IDLE){
        if(res != ERR_RW_FIN){
            if(res == ERR_RW_OV_TIME_ERR){
                MDM_RTU_CB_OverTimeReset(&modbusRWRTUCB);
            }
        }else {
            MDM_RTU_ReadBits(modbusRWRTUCB.pModbus_RTU,0x0000,16, (uint8_t*)&resTemp,COILS_TYPE,0x1);
            //resTemp=resTemp;
        }
    }
    #else

        if(MDM_RTU_ReadCoil(&modbusRWRTUCB,0x1,0x0000,16)==ERR_RW_FIN){
            MDM_RTU_ReadBits(modbusRWRTUCB.pModbus_RTU,0x0000,16, (uint8_t*)&resTemp,COILS_TYPE,0x1);
            resTemp=resTemp;
        }
    #endif

}

static void MDM_RTUUserWrite(void){
    MDError res;
    #if MD_NB_MODE_TEST
    res = MDM_RTU_NB_WriteCoils(&modbusRWRTUCB1,0x1,0,16,(uint8_t*)(&temp));
    if(res != ERR_IDLE){
        if(res != ERR_RW_FIN){
            if(res == ERR_RW_OV_TIME_ERR){
                MDM_RTU_CB_OverTimeReset(&modbusRWRTUCB1);
            }
        }
    }
    res = MDM_RTU_NB_WriteCoils(&modbusRWRTUCB4,0x1,0,16,(uint8_t*)(&temp2));
    if(res != ERR_IDLE){
        if(res != ERR_RW_FIN){
            if(res == ERR_RW_OV_TIME_ERR){
                MDM_RTU_CB_OverTimeReset(&modbusRWRTUCB4);
            }
        }
    }
    #else
        MDM_RTU_WriteCoils(&modbusRWRTUCB1,0x1,0,16,(uint8_t*)(&temp));
        MDM_RTU_WriteCoils(&modbusRWRTUCB4,0x1,0,16,(uint8_t*)(&temp2));
    #endif
}

static void MDM_RTU_UserUpdate(void){
    MDM_RTUUserRead();
    MDM_RTUUserWrite();
}


void MDM_RTU_Loop(void){
    MDM_RTU_UserUpdate();
}
