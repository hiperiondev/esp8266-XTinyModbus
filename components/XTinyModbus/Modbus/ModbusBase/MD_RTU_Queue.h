/********************************************************************************
 * @File name: MD_RTU_Queue.c
 * @Author: zspace
 * @Email: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU Queue function module
 ********************************************************************************/
#ifndef _MD_RTU_QUEUE_H__
#define _MD_RTU_QUEUE_H__

/*********************************HEAD FILE************************************/
#include "MD_RTU_Type.h"
#include "MD_RTU_Config.h"
/*********************************END******************************************/

/*********************************TYPE DEFINITION**************************************/
typedef uint8 MDQueueDateType;

typedef struct {
    MDQueueDateType data[MD_RTU_QUEUE_SIZE + 1];
     int16 front;
     int16 rear;
    uint16 maxVal;
     uint8 valid;
} *PMDSqQueue, MDSqQueue;
/*********************************END******************************************/

/*********************************FUNCTION DECLARATION**************************************/
 uint8 MDInitQueue(MDSqQueue *q);
uint16 MDQueueLength(MDSqQueue *q);
  void MDResetQueue(MDSqQueue *q);
 uint8 MDQueueEmpty(MDSqQueue *q);
 uint8 MDenQueueH(MDSqQueue *q, MDQueueDateType e);
 uint8 MDenQueue(MDSqQueue *q, MDQueueDateType e);
 uint8 MDdeQueue(MDSqQueue *q, MDQueueDateType *e);
 uint8 MDdeQueueF(MDSqQueue *q, MDQueueDateType *e);
 uint8 MDgetTailQueue(MDSqQueue *q, MDQueueDateType *e);
/*********************************END******************************************/

#endif
