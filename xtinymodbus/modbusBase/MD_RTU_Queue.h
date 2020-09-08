/********************************************************************************
 * @File name: MD_RTU_Queue.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 ********************************************************************************/
#ifndef _SQQUEUE_H__
#define _SQQUEUE_H__

#include <stdint.h>

/*********************************************************************/
#include "MD_RTU_Config.h"
/***************************************************************************/

/***********************************************************************/
typedef uint8_t MDQueueDateType;

typedef struct {
    MDQueueDateType data[MD_RTU_QUEUE_SIZE + 1];
    int16_t front, rear;
    uint16_t maxVal;
    uint8_t valid;
} *PMDSqQueue, MDSqQueue;
/**************************************************************************/

/***********************************************************************/
uint8_t MDInitQueue(MDSqQueue *q);
uint16_t MDQueueLength(MDSqQueue *q);
void MDResetQueue(MDSqQueue *q);
uint8_t MDQueueEmpty(MDSqQueue *q);
uint8_t MDenQueueH(MDSqQueue *q, MDQueueDateType e);
uint8_t MDenQueue(MDSqQueue *q, MDQueueDateType e);
uint8_t MDdeQueue(MDSqQueue *q, MDQueueDateType *e);
uint8_t MDdeQueueF(MDSqQueue *q, MDQueueDateType *e);
uint8_t MDgetTailQueue(MDSqQueue *q, MDQueueDateType *e);
/**************************************************************************/

#endif

