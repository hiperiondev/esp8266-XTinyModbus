#ifndef _MD_RTU_TOOL_H__
#define _MD_RTU_TOOL_H__

#include <stdint.h>
#include <stddef.h>

typedef void (*MDS_RTU_TimeHandlerFunction)(void *obj);
typedef void (*MDS_RTU_SendBytesFunction)(uint8_t *byte, uint16_t num);
typedef void (*MDS_RTU_RecByteFunction)(void *obj, uint8_t byte);
typedef void (*MDS_RTU_RecSendConv)(uint8_t mode);
typedef void (*MD_RTU_SerialInit)(void *obj, uint32_t baud, uint8_t dataBits, uint8_t stopBit, uint8_t parity);

typedef struct {
      MDS_RTU_SendBytesFunction mdRTUSendBytesFunction;
        MDS_RTU_RecByteFunction mdRTURecByteFunction;
            MDS_RTU_RecSendConv mdRTURecSendConv;
    MDS_RTU_TimeHandlerFunction mdRTUTimeHandlerFunction;
} *PModbusBase, ModbusBase;

/*ModbusBase*/
#define TO_MDBase(a)        ((PModbusBase)(a))

/*RegCoilItem*/
#define MDS_RTU_REG_COIL_LIST(a)              ((a)->pRegCoilList)
#define MDS_RTU_REG_COIL_ITEM_ADDR(a)         ((a)->modbusAddr)
#define MDS_RTU_REG_COIL_ITEM_DATA(a)         ((a)->modbusData)
#define MDS_RTU_REG_COIL_ITEM_Data_Size(a)    ((a)->modbusDataSize)
#define MDS_RTU_REG_COIL_ITEM_ADDR_TYPE(a)    ((a)->addrType)

#define MD_GET_BIT(a,b)    (((a)>>(b))&0x1)
#define MD_SET_BIT(a,b)    (a)|=(1<<(b))
#define MD_CLR_BIT(a,b)    (a)&=(~(1<<(b)))

#define MD_RTU_SEND_MODE(a)    while((a)->lastTimesTick!=0xFFFFFFFF);\
    if((a)->modbusBase.mdRTURecSendConv)(a)->modbusBase.mdRTURecSendConv(true)
#define MD_RTU_RECV_MODE(a)    while((a)->lastTimesTick!=0xFFFFFFFF);\
    if((a)->modbusBase.mdRTURecSendConv)(a)->modbusBase.mdRTURecSendConv(false)

#define MD_SWAP_HL(a)    (a)=((((a)&0xff)<<8)|(((a)>>8)&0xff))

#define MD_H_BYTE(a)     (((a)>>8)&0xff)
#define MD_L_BYTE(a)     ((a)&0xff)

#endif
