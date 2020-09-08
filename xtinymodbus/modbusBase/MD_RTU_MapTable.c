/********************************************************************************
 * @File name: MD_RTU_RegCoil.c
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
/*********************************************************************/
#include "MD_RTU_MapTable.h"
/*************************************************************************/

/*******************************************************
 *
 * Function name :RegCoilListAdd
 * Description        :
 * Parameter         :
 *        @obj
 *        @pRegCoilItem    [PRegCoilItem]
 * Return          : true success , false fail
 **********************************************************/
uint8_t MapTableAdd(void *obj, PMapTableItem pMapTableItem, uint16_t tabSize) {
    uint16_t i = 0;
    PMapTableItem *pMapTableList = obj;
    if (!obj) {
        return false;
    }
    for (i = 0; i < tabSize; i++) {
        if (pMapTableList[i] == NULL) {
            pMapTableList[i] = pMapTableItem;
            return true;
        }
    }
    return false;
}
/*******************************************************
 *
 * Function name :RegCoilListDel
 * Description        :
 * Parameter         :
 *        @obj
 *        @pRegCoilItem   [PRegCoilItem]
 * Return          : true success , false fail
 **********************************************************/
uint8_t MapTableDel(void *obj, PMapTableItem pMapTableItem, uint16_t tabSize) {
    uint16_t i = 0;
    PMapTableItem *pMapTableList = obj;
    if (!obj) {
        return false;
    }
    for (i = 0; i < tabSize; i++) {
        if (pMapTableList[i] == pMapTableItem) {
            pMapTableList[i] = NULL;
            return true;
        }
    }
    return false;
}
