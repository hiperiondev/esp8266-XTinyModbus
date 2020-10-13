/********************************************************************************
 * @File name: MDM_RTU_User_Fun.h
 * @Author: zspace
 * @Email: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: ????
 ********************************************************************************/

#ifndef _MDM_RTU_USER_FUN_H_
#define _MDM_RTU_USER_FUN_H_

#include "MD_RTU_Type.h"
#include "MD_RTU_MapTable.h"

BOOL MDM_RTU_ReadBits(void *obj, uint16 modbusAddr, uint16 numOf, uint8 *res, AddrType opAddrType, uint8 devAddr);
BOOL MDM_RTU_ReadRegs(void *obj, uint16 modbusAddr, uint16 numOf, uint16 *res, AddrType opAddrType, uint8 devAddr);

#endif
