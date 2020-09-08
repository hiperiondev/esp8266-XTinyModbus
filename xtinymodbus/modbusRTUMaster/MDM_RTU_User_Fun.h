#ifndef _MDM_RTU_USER_FUN_H_
#define _MDM_RTU_USER_FUN_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "MD_RTU_MapTable.h"

bool MDM_RTU_ReadBits(void *obj, uint16_t modbusAddr, uint16_t numOf, uint8_t *res,
        AddrType opAddrType, uint8_t devAddr);
bool MDM_RTU_ReadRegs(void *obj, uint16_t modbusAddr, uint16_t numOf, uint16_t *res,
        AddrType opAddrType, uint8_t devAddr);

#endif
