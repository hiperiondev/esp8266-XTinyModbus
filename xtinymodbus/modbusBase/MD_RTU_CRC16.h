/********************************************************************************
 * @File name: MD_RTU_CRC16.h
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU CRC16
 ********************************************************************************/
#ifndef _MD_RTU_CRC16_H__
#define _MD_RTU_CRC16_H__

#include <stdint.h>
#include "MD_RTU_Config.h"

/********************************************************************/
uint16_t MD_CRC16Update(uint16_t CRC, uint8_t byte);
/************************************************************************/

#endif
