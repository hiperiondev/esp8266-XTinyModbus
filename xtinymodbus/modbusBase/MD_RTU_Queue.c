/********************************************************************************
 * @File name: MD_RTU_Queue.c
 * @Author: zspace
 * @Emial: 1358745329@qq.com
 * @Version: 1.0
 * @Date: 2020-4-10
 * @Description: Modbus RTU
 * https://github.com/lotoohe-space/XTinyModbus
 ********************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "MD_RTU_Queue.h"

/*******************************************************
 *
 * Function name: MDInitQueue
 * Description:
 * Parameter:
 *        @q:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDInitQueue(MDSqQueue *q) {
//MDQueueDateType* data;
    if (q == NULL) {
        return false;
    }
    q->maxVal = MD_RTU_QUEUE_SIZE + 1;
    q->front = q->rear = 0;
    q->valid = true;
    return true;
}

/*******************************************************
 *
 * Function name: MDQueueLength
 * Description:
 * Parameter:
 *        @q:
 * Return:
 **********************************************************/
uint16_t MDQueueLength(MDSqQueue *q) {
    if (q == NULL) {
        return 0;
    }
    return (q->rear - q->front + q->maxVal) % q->maxVal;
}

/*******************************************************
 *
 * Function name: MDResetQueue
 * Description:
 * Parameter:
 *        @q:
 * Return:
 **********************************************************/
void MDResetQueue(MDSqQueue *q) {
    if (q == NULL) {
        return;
    }
    q->front = q->rear = 0;
}

/*******************************************************
 *
 * Function name: MDQueueEmpty
 * Description:
 * Parameter:
 *        @q:
 * Return: true, false
 **********************************************************/
uint8_t MDQueueEmpty(MDSqQueue *q) {
    if (q == NULL) {
        return 1;
    }
    return (q->front == q->rear);
}

/*******************************************************
 *
 * Function name: MDenQueue
 * Description:
 * Parameter:
 *        @q:
 *        @e:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDenQueue(MDSqQueue *q, MDQueueDateType e) {
    if ((q->rear + 1) % q->maxVal == q->front) {
        return false;
    }
    q->rear = (q->rear + 1) % q->maxVal;
    q->data[q->rear] = e;
    return true;
}

/*******************************************************
 *
 * Function name: MDenQueueH
 * Description:
 * Parameter:
 *        @q:
 *        @e:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDenQueueH(MDSqQueue *q, MDQueueDateType e) {
    if ((q->front - 1 + q->maxVal) % q->maxVal == q->rear)
        return false;

    q->data[q->front] = e;
    q->front = (q->front - 1 + q->maxVal) % q->maxVal;
    return true;
}

/*******************************************************
 *
 * Function name: MDdeQueue
 * Description:
 * Parameter:
 *        @q:
 *        @e:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDdeQueue(MDSqQueue *q, MDQueueDateType *e) {
    if (q->front == q->rear) { /**/
        return false;
    }
    q->front = (q->front + 1) % q->maxVal;
    *e = q->data[q->front];
    return true;
}

/*******************************************************
 *
 * Function name: MDdeQueueF
 * Description:
 * Parameter:
 *        @q:
 *        @e:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDdeQueueF(MDSqQueue *q, MDQueueDateType *e) {
    if (q->front == q->rear) {
        return false;
    }
    *e = q->data[q->rear];
    q->rear = (q->rear - 1 + q->maxVal) % q->maxVal;
    return true;
}

/*******************************************************
 *
 * Function name: MDgetTailQueue
 * Description:
 * Parameter:
 *        @q:
 *        @e:
 * Return: true success, false fail
 **********************************************************/
uint8_t MDgetTailQueue(MDSqQueue *q, MDQueueDateType *e) {
    if (q->front == q->rear) { /**/
        return false;
    }
    *e = q->data[(q->front + 1) % q->maxVal];
    return true;
}
