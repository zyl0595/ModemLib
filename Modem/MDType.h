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
    MDE_BUFEMPTY,   /*缓存区空*/
    MDE_TIMEOUT,    /*超时*/
    MDE_ALREADYON,  /*已打开*/
    MDE_PARAM_ERR,  /*参数错误*/
    MDE_ONEXIT,     /*请求执行退出*/
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
    MDSOCK_STREAM = 1,    /*TCP*/
    MDSOCK_DGRAM  = 2,    /*UDP*/
    //MDSOCK_RAW  = 3     /*RAW(不支持)*/
}eMDSockType;

/*模块返回AT指令响应定义*/ 
typedef struct{
    uint8_t isPositive;           /*是否是积极响应，指令返回"OK"时为TRUE，返回"ERROR"或其他时为FALSE*/
    uint8_t *buf;                 /*指令返回数据接收缓存 固定指向全局数组 s_ATRcvBuf[MD_RCV_BUF_SIZE] */
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
}sMDSockData; //------------------------------------------------------------------------这个类型要去掉！！！

typedef union{
    uint32_t val;
    struct{
        uint8_t v1; /*低位*/
        uint8_t v2; /*次低位*/
        uint8_t v3; /*次高位*/
        uint8_t v4; /*高位*/
    }sVal;
}sMDIPv4Addr;


/*Socket连接对端地址*/
typedef struct{
    sMDIPv4Addr IP;     //IP地址
    uint16_t    port;   //端口号
}sMDSockPeerAddr;

/*保存数据索引使用的结构体*/
typedef struct{
    uint8_t  *pBuf;    //指向存储区
    uint32_t bufSize;  //存储区大小
    uint32_t len;      //存储区内数据长度
}sMDDataHdl;

/*管理TCP/IP接收或发送数据结构体*/
typedef struct{
    bool            isNewRcved; //是否接收到新的数据
    uint8_t         sockNum;    //Socket编号
    sMDSockPeerAddr srcAddr;    //Socket对端IP地址和端口号
    sMDDataHdl      rcvData;    //发送或者接收到的数据信息（发送和接收前必须对buf指针进行赋值！）
}sMDIPData;


/*Socket Control Block 控制块*/
typedef struct{
//#define MD_MAX_IPV4_STR_LEN (4*4+1) /*字符串型IPv4最大存储长度，如 "255.255.255.255"*/
    uint8_t         id;         /*Socket编号*/
    eMDSockType     type;       /*Socket类型*/
    eMDSockState    state;      /*Socket连接状态*/
    sMDIPv4Addr     serverIp;   /*对端IP地址*/
    uint16_t        serverPort; /*对端端口号*/
    uint16_t        localPort;  /*本地端口号*/
}sMDSocket;


/*模块状态*/
typedef enum{
    MDS_BEGIN = 0,  /*初始状态*/
    //MDS_CHECK,      /*状态检测：插卡状态、信号质量、当前网络运营商、网络注册状态、网络注册状态*/
    MDS_READY,      /*准备就绪：可建立TCP、UDP链路进行数据收发状态*/
    MDS_FAILED      /*未定义错误状态*/
}eMDStates;


/*网络信号质量*/
typedef struct{
    uint8_t rssi;   /*接收信号强度指示*/
    uint8_t ber;    /*通道误码率*/
}sMDCSQ;

/*网络制式*/
typedef enum{
    NET_MODE_AUTO = 0,
    NET_MODE_CDMA,
    NET_MODE_HDR,
    NET_MODE_GSM,
    NET_MODE_WCDMA,
    NET_MODE_TD_SCDMA,
    NET_MODE_EVDO,
}eMDNetMode;

typedef enum{
    NET_REG_UNREG = 0,  /*未注册*/
    NET_REG_HOME,       /*已注册，本地网络*/
    NET_REG_SEARCHING,  /*未注册，正在搜索运营商*/
    NET_REG_DENIED,     /*未注册，网络注册被拒绝*/
    NET_REG_UNKNOW,     /*未知状态*/
    NET_REG_ROAM        /*已注册，漫游状态*/
}eMDNetRegState;



/*模块操作函数接口定义*/
/*实现原则：
    1.这些函数完成的操作尽量要单一；
    2.无需在函数内做指令重发，指令的重发由调用者根据函数返回结果进行*/
////typedef eMDErrCode (*DMF_GetModelInfo)(uint8_t *pBuf);                                              /*获取模块型号信息*/
//typedef eMDErrCode (*DMF_GetIMEI)(uint8_t *pBuf);                                                   /*获取模块IMEI号*/
//typedef eMDErrCode (*DMF_CheckSIM)(void);                                                           /*检查SIM卡状态*/
//typedef eMDErrCode (*DMF_GetIMSI)(uint8_t *pBuf);                                                   /*获取SIM卡IMSI号、运营商识别*/
//typedef eMDErrCode (*DMF_GetCSQ)(sMDCSQ *pCsq);                                                     /*获取当前网络信号质量信息*/
//
//typedef eMDErrCode (*DMF_SelectNet)(eMDNetMode m);                                                  /*设置模块网络制式，强制为某一网络制式或自动*/
//typedef eMDErrCode (*DMF_RegToNet)(void);                                                           /*触发模块注册到网络*/
//typedef eMDErrCode (*DMF_ChekNetReg)(uint8_t *pStat);                                               /*检查网络注册状态*/
////typedef eMDErrCode (*DMF_GetNetOperAndMode)(uint8_t *pBuf);                                         /*获取当前网络运营商信息及当前网络模式*/
//
//typedef eMDErrCode (*DMF_DefPDPCtx)(eMDNetMode m, uint8_t *pCtx);                                   /*定义PDP上下文*/
//typedef eMDErrCode (*DMF_ActPDPCtx)(uint8_t cid);                                                   /*激活PDP上下文*/
//typedef eMDErrCode (*DMF_SetUsrPwd)(uint8_t *pUsr, uint8_t *pPwd);                                  /*设置用户名和密码*/
//typedef eMDErrCode (*DMF_ChekNetAttch)(uint8_t *pStat);                                             /*检查GPRS网络附着状态*/

typedef eMDErrCode (*DMF_SocketInit)(void);                                                         /*设置参数、初始化模块到可以开始建立Socket链接的状态*/
typedef eMDErrCode (*DMF_GetLocalAddr)(sMDIPv4Addr *pAddr);                                         /*获取本机IP地址*/
typedef eMDErrCode (*DMF_SockConnect)(sMDSocket *pS);                                               /*建立一个Socket链接*/
typedef eMDErrCode (*DMF_SockClose)(sMDSocket *pS);                                                 /*关闭一个Socket链接*/
//typedef eMDErrCode (*DMF_SockGetState)(sMDSocket *pSocks, uint8_t num);                             /*检查更新num个Socket链接的状态*/
typedef eMDErrCode (*DMF_SockSend)(sMDSocket *pS, const uint8_t *pData, uint16_t len);              /*通过一个Socket链接发送数据*/
//typedef eMDErrCode (*DMF_SockRcv)(uint8_t *pSN, uint8_t *pDes, int *pLen, sMDSockPeerAddr *pSrcIp, uint32_t delay);/*接收数据，部分模块需要主动去接收数据*/
typedef eMDErrCode (*DMF_GetHostByName)(const uint8_t *pName, sMDIPv4Addr *pIp);                    /*通过模块DNS服务获取解析域名获取IP地址*/

/*模块操作函数列表（每个模块都有一个自己列表，模块维护函数通过调用该列表中的函数完成所有对模块的操作）
当这个列表中的函数指针为NULL时，默认使用对应的通用函数对模块进行操作*/
/*内嵌TCP/IP协议栈操作*/
typedef struct{
    DMF_SocketInit      SocketInit;
    DMF_GetLocalAddr    GetLocalAddr;
    DMF_SockConnect     SockConnect;
    DMF_SockClose       SockClose;
    DMF_SockSend        SockSend;
    DMF_GetHostByName   GetHostByName;
}sMDFucTable;


/*非请求结果码处理相关定义*/
typedef eMDErrCode (*MDF_URCHandle)(const uint8_t *pBuf, void *pArg);

typedef struct{
    const uint8_t *pURC;    //非请求结果码标识字符串
    MDF_URCHandle handle;   //非请求结果码处理函数
    void          *pArg;    //非请求结果码处理函数传入/传出参数
}sMDURCHdlItem;


/*存储模块各类状态信息*/
typedef struct{
    eMDStates           state;
    uint16_t            type;           /*模块型号*/
    const uint8_t       *pTypeName;     /*模块型号描述*/
    const sMDFucTable   *pFunTable;     /*操作模块的各类函数列表*/
    const sMDURCHdlItem *pURCHdlTable;  /*URC数据接收处理函数表*/
    bool                isSIMReady;     /*SIM卡状态*/
    sMDCSQ              csq;            /*信号质量*/
    uint8_t             IMEI[20];
    uint8_t             IMSI[20];
    uint8_t             maxSockNum;     /*根据模块不同支持的最大Socket链接数不同*/
    eMDNetRegState      netRegState;    /*网络注册状态*/
    sMDIPv4Addr         localAddr;      /*建立网络连接之后获得的本机IP地址*/
    sMDSocket           sockets[MD_MAX_SOCK_NUM];   /*Socket列表，在该数组中的位置则代表该Socket的编号*/
}sMDModem;

/*模块事务处理进程将会收到的各类消息类型，这些消息需要模块事务处理进程逐个处理*/
typedef enum{
    MSG_UART_RCV,       /*从模块接收到新的数据，这是一个信号，告诉模块事务处理进程要去检查接收了*/
    MSG_CONNECT_REG,    /*建立Socket连接请求*/
    MSG_DISCONN_REQ,    /*断开Socket请求*/
    MSG_SEND_REQ,       /*发送TCP、UDP数据请求*/
    MSG_SQC_REQ,        /*信号质量查询请求*/
    MSG_DNS_REG,        /*域名解析服务请求*/
}eMDMsgType;

/*一般性消息*/
typedef struct{
    /*消息类型（强制）*/
    eMDMsgType type;    /*消息类型*/

    void *pContent;     /*消息内容，一般指向一个请求内容体*/
}sMDMsg;

/*连接请求消息内容体*/
typedef struct{
    /*消息类型（强制）*/
    eMDMsgType type;

    sMDSocket sock; /*要执行连接的socket信息*/
}sMDConnectReq;

/*数据发送请求消息内容体*/
typedef struct{
    /*消息类型（强制）*/
    eMDMsgType type;

    uint8_t sock;   /*Socket编号*/
    uint8_t *pBuf;  /*指向要发送数据起始地址*/
    uint32_t len;   /*发送数据长度*/
}sMDSockSndReq;

#endif //__MD_TYPE_H

