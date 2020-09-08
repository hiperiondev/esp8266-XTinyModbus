/********************************************************************************
 * @File name: MD_RTU_RegCoil.h
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 ********************************************************************************/
#ifndef _MD_RTU_REGCOIL_H__
#define _MD_RTU_REGCOIL_H__

#include <stdint.h>

/*******************************************************************/
typedef enum {
         COILS_TYPE = 1,
         INPUT_TYPE = 2,
     HOLD_REGS_TYPE = 3,
    INPUT_REGS_TYPE = 4
} AddrType;

typedef struct {
    uint16_t modbusAddr;
    uint16_t *modbusData;
    uint16_t modbusDataSize;
    AddrType addrType;
     uint8_t devAddr;
    } *PMapTableItem, MapTableItem;

/***************************************************************************/

/********************************************************************/
uint8_t MapTableAdd(void *obj, PMapTableItem pMapTableItem, uint16_t tabSize);
uint8_t MapTableDel(void *obj, PMapTableItem pMapTableItem, uint16_t tabSize);
/***************************************************************************/

#endif
