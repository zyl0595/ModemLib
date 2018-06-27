/* 
* 文件名称：MDType.h
* 摘    要：库使用到的自定义类型定义
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

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

#ifndef bool
#define bool uint8_t 
#endif

/*错误类型定义*/ 
typedef enum{
    MDE_OK = 0,     /*正确*/
    MDE_TTYSERR,    /*串口读写错误*/
    MDE_BUFOVFL,    /*接收缓存溢出*/
    MDE_TIMEOUT,    /*超时*/
    MDE_ALREADYON,  /*已打开*/
    MDE_ERROR       /*未定义错误*/
}eMDErrCode;

/*Socket连接状态*/
typedef enum{
    SOCK_CLOSED = 0,   /*断开*/
    SOCK_OPENED,       /*打开，已连接*/
    SOCK_ERROR,        /*出错*/
}eMDSockState;

/*Socket类型*/
typedef enum{
    SOCK_STREAM = 1,    /*TCP*/
    SOCK_DGRAM  = 2,    /*UDP*/
    //SOCK_RAW  = 3     /*RAW*/
}eMDSockType;

/*模块返回AT指令响应定义*/ 
typedef struct{
    uint8_t isPositive;           /*是否是积极响应，指令返回"OK"时为TRUE，返回"ERROR"或其他时为FALSE*/
    uint8_t buf[MD_RCV_BUF_SIZE]; /*指令返回数据接收缓存*/
    uint16_t len;                 /*回应数据长度*/ 
}sMDAtCmdRsp;

/*
* AT指令响应处理回调函数
* 参数定义：
*   pRsp：指令回应数据
*/
typedef eMDErrCode (*ATCmdRspHdl)(sMDAtCmdRsp *pRsp, void *pArg); 

/*AT指令定义*/
typedef struct{
    const uint8_t *pCmd;  /*指令*/
    uint8_t tryTms;       /*最大重试次数*/
    uint8_t delay;        /*指令响应等待时间(单位：s)*/
    ATCmdRspHdl rspHdl;   /*指令响应处理回调函数，没有时置NULL*/
    void *pArg;           /*回调函数参数或匹配返回数据的目标字符串*/
}sMDAtCmdItem;

/*通过AT指令发送Tcp数据的数据结构定义*/
typedef struct{
    const uint8_t *pData;   /*要发送的数据*/
    uint16_t len;           /*数据长度*/
    const uint8_t *pIp;     /*目的Ip地址*/
    uint16_t port;          /*目的端口号*/
}sMDSockData;

typedef union{
    uint32_t val;
    struct{
        uint8_t v1; /*低位*/
        uint8_t v2; /*次低位*/
        uint8_t v3; /*次高位*/
        uint8_t v4; /*高位*/
    }sVal;
}sMDIPv4Addr;

/*Socket Control Block 控制块*/
typedef struct{
    eMDSockType type;   /*Socket类型*/
    eMDSockState state; /*Socket连接状态(用作建立连接时这个字段不用理会)*/
    sMDIPv4Addr ip;     /*对端IP地址*/
    uint16_t    port;   /*对端端口号*/
}sMDSocket;

/*模块状态*/
typedef enum{
    MDS_BEGIN = 0,  /*初始状态*/
    MDS_CHECK,      /*状态检测：插卡状态、信号质量、当前网络运营商、网络注册状态、网络注册状态*/
    MDS_READY,      /*准备就绪：可建立TCP、UDP链路进行数据收发状态*/
    MDS_FAILED      /*未定义错误状态*/
}eMDStates;


/*网络信号质量*/
typedef struct{
    uint8_t rssi;   /*接收信号强度指示*/
    uint8_t ber;    /*通道误码率*/
}sMDCSQ;


/*存储模块各类状态信息*/
typedef struct{
    uint16_t type;      /*模块型号*/
    sMDCSQ   csq;       /*信号质量*/
    uint8_t IMEI[20];
    uint8_t IMSI[20];

}sMDModemInfo;


typedef struct{
    eMDStates   state;
    sMDModemInfo mdInfo;                    /*模块信息：类型、厂家信息、信号质量、网络状态、IMEI、IMSI*/
    sMDIPv4Addr localAddr;                  /*建立网络连接之后获得的本机IP地址*/
    sMDSocket   sockets[MD_MAX_SOCK_NUM];   /*Socket列表*/
    uint8_t     maxSockNum;                 /*根据模块不同支持的最大Socket链接数不同*/
}sMDModem;

#endif //__MD_TYPE_H

