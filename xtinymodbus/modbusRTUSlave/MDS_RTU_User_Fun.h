/********************************************************************************
 * @File name: MD_RTU_User_Fun.h
 * @Author: zspace
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 ********************************************************************************/

#ifndef _MD_RTU_USER_FUN_H__
#define _MD_RTU_USER_FUN_H__

#include "MD_RTU_MapTable.h"

/*******************************************************************/
bool MDS_RTU_ReadBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res,
        AddrType opAddrType);
bool MDS_RTU_ReadRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res,
        AddrType opAddrType);
bool MDS_RTU_WriteBit(void *obj, uint16_t modbusAddr, uint8_t bit, AddrType opAddrType);
bool MDS_RTU_WriteBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit,
        AddrType opAddrType);
bool MDS_RTU_WriteReg(void *obj, uint16_t modbusAddr, uint16_t reg, AddrType opAddrType);
bool MDS_RTU_WriteRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg,
        uint8_t isBigE, AddrType opAddrType);

bool MDS_RTU_WriteCoil(void *obj, uint16_t modbusAddr, uint8_t bit);
bool MDS_RTU_WriteInput(void *obj, uint16_t modbusAddr, uint8_t bit);
bool MDS_RTU_WriteCoils(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit);
bool MDS_RTU_WriteInputs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *bit);
bool MDS_RTU_WriteHoldReg(void *obj, uint16_t modbusAddr, uint16_t reg);
bool MDS_RTU_WriteHoldRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg);
bool MDS_RTU_WriteInputReg(void *obj, uint16_t modbusAddr, uint16_t reg);
bool MDS_RTU_WriteInputRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *reg);
bool MDS_RTU_ReadCoils(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res);
bool MDS_RTU_ReadInput(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res);
bool MDS_RTU_ReadHoldRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res);
bool MDS_RTU_ReadInputRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res);
/*************************************************************************/

#endif
