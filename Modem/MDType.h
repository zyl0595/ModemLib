#ifndef __MD_TYPE_H
#define __MD_TYPE_H

#include <stdint.h> 
#include "MDConfig.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef BOOLEAN
#define BOOLEAN uint8_t 
#endif

/*错误类型定义*/ 
typedef enum{
    MDE_OK = 0,     /*正确*/
    MDE_TTYSERR,    /*串口读写错误*/
    MDE_BUFOVFL,    /*接收缓存溢出*/
    MDE_TIMEOUT,    /*超时*/
    MDE_ERROR       /*未定义错误*/
}eMDErrCode;

/*模块返回AT指令响应定义*/ 
typedef struct{
    uint8_t isPositive;           /*是否是积极响应*/
    uint8_t buf[MD_RCV_BUF_SIZE]; /*接收回应内容缓存*/
    uint16_t len;                 /*回应数据长度*/ 
}sMDAtCmdRsp;

typedef eMDErrCode (*ATCmdRspHdl)(sMDAtCmdRsp *pRsp, void * arg); //AT指令回应处理回调函数接口定义 

/*AT指令定义*/
typedef struct{
    const uint8_t *pCmd;  /*指令*/
    uint8_t tryTms;       /*最大重试次数*/
    uint8_t delay;        /*指令响应等待时间(单位：s)*/
    ATCmdRspHdl rspHdl;   /*指令响应处理回调函数，没有时置NULL*/
    void *pArg;           /*回调函数参数，没有回调函数时作为匹配返回数据的目标字符串*/
}sMDAtCmdItem;

/*通过AT指令发送Tcp数据的数据结构定义*/
typedef struct{
    const uint8_t *pData;   /*要发送的数据*/ 
    uint16_t len;           /*数据长度*/ 
    const uint8_t *pIp;     /*目的Ip地址*/ 
    uint16_t port;          /*目的端口号*/
}sMDTcpData;

#endif //__MD_TYPE_H

