/* 
* 文件名称：MD.c
* 摘    要：
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#include <string.h>
#include "MDType.h"
#include "MDConfig.h"
#include "MDPort.h"
#include "MDAtProc.h"
#include "MDTools.h"
#include "ucos_ii.h"

//支持的所有模块头文件添加到这
#include "MD_CLM920.h"

/*************************** 私有定义 ******************************/
typedef struct{
    uint8_t type;                       /*模块类型编号，唯一*/
    const uint8_t *pInfo;               /*模块类型名称信息，常量字符串*/
    const sMDFucTable *pFunTable;       /*对模块进行各类操作的函数表*/
    const sMDURCHdlItem *urcHdlTable;   /*非请求结果码处理表*/
}sMDModemTypeInfo;

typedef enum{
    MD_STATE_BEGIN,   /*开机初始状态*/
    MD_STATE_NET_REG, /*已注册网络，socket连接*/
    MD_STATE_SOCK_INIT,
    MD_STATE_RUN,
    MD_STATE_ERROR,
};

#define MODEL_UNKNOWN       0
#define MODEL_YUGA_CLM920   1
#define MODEL_QUECTEL_EC20  2

const sMDModemTypeInfo s_modelInfoTable[] = {
    {MODEL_YUGA_CLM920,     "CLM920",   &CLM920_FunTable,   s_CLM920UrcHdlTable},
    {MODEL_QUECTEL_EC20,    "EC20",     NULL,               NULL},
    {MODEL_UNKNOWN,         "unknown",  NULL,               NULL}
};

/*************************** 私有变量 ******************************/
/*TCP、UDP数据发送确认队列*/
static void *s_SndCfm[5];
OS_EVENT *s_SndCfmQ = NULL;
static int s_sndCfmVal = 0;

/*TCP、UDP数据发送确认队列*/
static void *s_ConnCfm[5];
OS_EVENT *s_ConnCfmQ = NULL;
static int s_sndConnVal = 0;


/*************************** 全局变量 ******************************/
sMDModem g_Md; /*存储记录模块状态和资源*/
static void *s_ProcMsg[MD_PROC_MSG_Q_SIZE];/*模块事务消息队列存储*/
OS_EVENT *g_MsgQ = NULL;    /*模块事务消息队列，用户对模块的操作首先封装成请求消息，然后发送到该消息队列，由
                              模块事务处理进程挨个处理*/


/*
空闲等待状态：主机循环检查(或在串口接收完成触发接收事件后)检查接收缓冲区，并根据接收缓冲区内容
处理接收消息
主动交互状态：主机向模块发送AT指令，并等待直至接收到模块回复数据的过程
*/
uint8_t g_AtIsIdle = TRUE; //AT交互状态切换标志：TRUE:空闲等待状态  FALSE:主动交互状态


/*************************** 函数实现 ******************************/

static void MD_SockClean(sMDSocket *pSock)
{
    if(NULL != pSock){
        pSock->serverIp.val = 0; //0.0.0.0
        pSock->serverPort   = 0;
        pSock->state  = SOCK_CLOSED;
        pSock->type   = MDSOCK_STREAM;
    }
}


static void MD_ModemClean(sMDModem *pMd)
{
    int i = 0;

    memset(pMd, 0, sizeof(sMDModem));

    /*设置初始状态*/
    pMd->state = MDS_BEGIN;

    /*清除Socket连接信息*/
    for(i=0;i<MD_MAX_SOCK_NUM;i++){
        MD_SockClean(&(pMd->sockets[i]));
    }

    /*清除模块型号信息和函数操作表*/
    pMd->pTypeName = NULL;
    pMd->pFunTable = NULL;
}


/*
* 函数功能：识别模块类型并获取模块信息、操作函数表
*
* 参数说明：
*     [out]pMd :   保存模块类型信息和操作函数表
*
* 返回值：是否成功获取到模块信息并配置了pMd模块相关数据
*/ 
static eMDErrCode MD_ModemDetect(sMDModem *pMd)
{
    eMDErrCode err;
    sMDAtCmdRsp rsp;
    int i;

    err = MD_ATCmdSnd("ATI\r\n", 2, &rsp);
    if((MDE_OK == err) && (rsp.isPositive)){
        /*查找匹配模块信息*/
        for(i = 0; i < TABLE_SIZE(s_modelInfoTable); i++){
            if(strstr(rsp.buf, s_modelInfoTable[i].pInfo)){
                pMd->type = s_modelInfoTable[i].type;//模块型号码
                pMd->pTypeName = s_modelInfoTable[i].pInfo;//型号名称
                pMd->pFunTable = s_modelInfoTable[i].pFunTable;//模块操作函数表
                pMd->pURCHdlTable = s_modelInfoTable[i].urcHdlTable;//URC处理
                MD_DEBUG("Detect model type:%s\r\n", pMd->pTypeName);

                /*模块操作函数表检查*/
                if(NULL == pMd->pFunTable){
                    MD_DEBUG("model fun table empty error!\r\n");
                    return MDE_ERROR;
                }

                /*URC处理操作函数表检查*/
                if(NULL == pMd->pURCHdlTable){
                    MD_DEBUG("model URC handle table empty error!\r\n");
                    return MDE_ERROR;
                }
                
                /*模块操作函数表项检查*/
                if((NULL == pMd->pFunTable->SocketInit) ||
                   (NULL == pMd->pFunTable->GetLocalAddr) ||
                   (NULL == pMd->pFunTable->SockConnect) ||
                   (NULL == pMd->pFunTable->SockClose) ||
                   (NULL == pMd->pFunTable->SockSend) ||
                   (NULL == pMd->pFunTable->GetHostByName)){

                   MD_DEBUG("model fun table item empty error!\r\n");
                   return MDE_ERROR;
                }

                return MDE_OK;
            }
        }
    }
    return MDE_ERROR;
}


//域名解析
eMDErrCode MD_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{
    if(NULL != g_Md.pFunTable){
        if(NULL != g_Md.pFunTable->GetHostByName){
            return g_Md.pFunTable->GetHostByName(pName, pAddr);
        }else{
            MD_DEBUG("Fuc err:%s missing GetHostByName()\r\n", g_Md.pTypeName);
        }
    }
    return MDE_ERROR;
}

//建立Socket连接
eMDErrCode MD_ConnectTest(sMDSocket *pSock)
{
    if(NULL != g_Md.pFunTable){
        if(NULL != g_Md.pFunTable->SockConnect){
            return g_Md.pFunTable->SockConnect(pSock);
        }else{
            MD_DEBUG("Fuc err:%s missing SockConnect()\r\n", g_Md.pTypeName);
        }
    }
    return MDE_ERROR;
}


/*
* 函数功能：建立一个Socket链接（阻塞式）
*
* 参数说明：
*     [in]pSock :   要用来创建链接的socket信息； 
*
* 返回值：成功返回1，失败返回0
*/ 
int MD_ConnectB(sMDSocket *pSock)
{
    sMDConnectReq connReq;
    int *connResult;
    INT8U err;

    connReq.type = MSG_CONNECT_REG;
    memcpy(&connReq.sock, pSock, sizeof(sMDSocket));
    OSQPost(g_MsgQ, &connReq);

    MD_DEBUG("Waiting for connect confirm...\r\n");

    /*等待连接操作完成*/
    connResult = OSQPend(s_ConnCfmQ, 0, &err);
    if(*connResult){
        pSock->state = SOCK_OPENED;
        return 1;
    }else{
        return 0;
    }
}


/*
* 函数功能：通过socket发送TCP/UDP数据（阻塞式）
*
* 参数说明：
*     [in]fd :   用于发送数据的socket编号； 
*     [in]pBuf : 指向要发送数据存储区；
*     [in]len :  要发送的数据长度；
*
* 返回值：成功返回发送的总长度，失败返回0
*/ 
int MD_SendDataB(unsigned char fd, unsigned char *pBuf, int len)
{
    sMDSockSndReq sndReq;
    int *sndResult;
    INT8U err;

    sndReq.type = MSG_SEND_REQ;
    sndReq.sock = fd;
    sndReq.len = len;
    sndReq.pBuf = pBuf;
    OSQPost(g_MsgQ, &sndReq);

    MD_DEBUG("Waiting for data send confirm...\r\n");

    /*等待数据发送完成*/
    sndResult = OSQPend(s_SndCfmQ, 0, &err);
    if(*sndResult){
        return len;
    }else{
        return 0;
    }
}

//测试函数，用于连接指定tcp服务器
int MD_connect(void)
{
    //eMDErrCode ret;
    sMDSocket *pSock = &g_Md.sockets[0];
 
    pSock->id = 1;
    pSock->type = MDSOCK_STREAM;
    pSock->serverIp.sVal.v4 = 120;
    pSock->serverIp.sVal.v3 = 42;
    pSock->serverIp.sVal.v2 = 46;
    pSock->serverIp.sVal.v1 = 98;
    pSock->serverPort = 5700;
    pSock->localPort = 1001;

    //ret = MD_ConnectTest(pSock);
    return MD_ConnectB(pSock);
}


//int MD_SendData(unsigned char fd, unsigned char *pBuf, int len)
//{
//
//}


static eMDErrCode MD_MsgProc(sMDMsg *pMsg)
{
    eMDErrCode ret;
    sMDSockSndReq *pSockSndReq;
    uint8_t sndCnt = 0;

    if(NULL == pMsg){
        //MD_DEBUG("Msg Proc para err!\r\n");
        return MDE_PARAM_ERR;
    }

    /*根据消息类型执行相应的处理*/
    switch(pMsg->type){
        case MSG_UART_RCV:                                                  //接收到模块数据
            {
                //MD_DEBUG("Proc uart rcv msg!\r\n");
                do{
                    ret = MD_ATURCProc(g_Md.pURCHdlTable, 5000);
                }while(MDE_BUFEMPTY != ret);
            }
            break;
        case MSG_CONNECT_REG:                                               //建立Socket链接
            {
                MD_DEBUG("Proc socket connect msg!\r\n");
                if((NULL != g_Md.pFunTable) && (NULL != g_Md.pFunTable->SockConnect)){
                    ret = g_Md.pFunTable->SockConnect(&((sMDConnectReq *)pMsg)->sock);
                    if(MDE_OK == ret){
                        s_sndConnVal = 1;//连接成功
                        ((sMDConnectReq *)pMsg)->sock.state = SOCK_OPENED;
                    }else{
                        s_sndConnVal = 0;//连接失败
                    }
                }else{
                    MD_DEBUG("Fuc err:%s missing SockConnect()\r\n", g_Md.pTypeName);
                    s_sndConnVal = 0;//连接失败
                }
                OSQPost(s_ConnCfmQ, &s_sndConnVal);
            }
            break;
        case MSG_SEND_REQ:                                                  //发送网络数据
            {
                pSockSndReq = (sMDSockSndReq *)pMsg;

                if(SOCK_OPENED == g_Md.sockets[pSockSndReq->sock].state){

                    /*执行数据发送*/
                    do{
                        ret = g_Md.pFunTable->SockSend(&g_Md.sockets[pSockSndReq->sock],
                            pSockSndReq->pBuf, pSockSndReq->len);
                        sndCnt++;
                        if(MDE_OK != ret){
                            if(sndCnt > MD_MAX_RESND_TIMES){
                                MD_DEBUG("Data snd faild!\r\n");
                                s_sndCfmVal = 0;
                                break;//do while
                            }
                            MD_Delay(5000);
                        }
                    }while(MDE_OK != ret);
                    
                    s_sndCfmVal = 1;//发送成功
                }else{
                    s_sndCfmVal = 0;
                }

                /*告知消息发送者发送操作完成*/
                OSQPost(s_SndCfmQ, &s_sndCfmVal);
            }
            break;
        default:                                                            //未知请求
            MD_DEBUG("MD got unknow msg type:%d\r\n", (int)pMsg->type);
            ret = MDE_OK;
            break;
    }
    return ret;
}


static eMDErrCode MD_Run(void)
{
    sMDMsg *pMsg;
    INT8U err;

    do{
        pMsg = OSQPend(g_MsgQ, 3000, &err);
        if(OS_NO_ERR == err){
            MD_MsgProc(pMsg);
        }else if(OS_TIMEOUT ==  err){
            /*执行一些状态检查如检查链路状态等*/
            MD_DEBUG("-\r\n");
        }else{
            MD_DEBUG("MD run msg queue pend err!\r\n");
        }
    }while(1);

    return MDE_OK;
}


static void MD_Task(void *pArg)
{
    do{
        switch(g_Md.state)
        {
        case MDS_BEGIN:
            //进行资源初始化、网络注册、Socket连接初始

            /*模块型号识别*/
            if(MDE_OK != MD_ModemDetect(&g_Md)){
                MD_DEBUG("Detect MD model failed!\r\n");
                break;
            }

            /*模块初始化*/
            if(NULL == g_Md.pFunTable){
                MD_DEBUG("Modem function table is empty!\r\n");
                break;
            }
            if(MDE_OK != g_Md.pFunTable->SocketInit()){
                MD_DEBUG("Modem init failed!\r\n");
            }else{
                g_Md.state = MDS_READY;
                //if(MD_connect()){
                //    g_Md.state = MDS_READY;
                //}else{
                //    MD_DEBUG("Modem sock connect failed!\r\n");
                //}
            }

            break;
        case MDS_READY:
            //等待事件
            //事件类型：发起链接、发送数据、接收数据
            if(MDE_ONEXIT == MD_Run()){//如没有出现错误不返回
                g_Md.state = MDS_FAILED;
            }
            break;
        case MDS_FAILED:
            //关闭链接、回收资源、重置状态、报告错误
            MD_DEBUG("Modem err occour!\r\n");

            g_Md.state = MDS_BEGIN;
            break;
        default:
            //不应执行到这。
            g_Md.state = MDS_BEGIN;
            break;
        }

        MD_Delay(10);
    }while(1);

    MD_DEBUG("*************MD_ProcTask end*************\r\n");//不应该执行到这
    while(1);
}


/*
* 识别模块类型，初始化模块连接网络 
*/ 
int MD_Init(unsigned char *pBuf, unsigned int bufSize)
{
    /*创建消息邮箱*/
    if(NULL == g_MsgQ){//模块操作请求邮箱队列
        g_MsgQ = OSQCreate(&s_ProcMsg[0], MD_PROC_MSG_Q_SIZE);
        if(NULL == g_MsgQ){
            MD_DEBUG("Message quere create failed!\r\n");
            return 0;
        }
    }

    if(NULL == s_SndCfmQ){//TCP/IP数据发送确认邮箱
        s_SndCfmQ = OSQCreate(&s_SndCfm[0], (sizeof(s_SndCfm)/sizeof(s_SndCfm[0])));
        if(NULL == s_SndCfmQ){
            MD_DEBUG("Message snd confim quere create failed!\r\n");
            return 0;
        }
    }

    if(NULL == s_ConnCfmQ){//Socket连接确认发送确认邮箱
        s_ConnCfmQ = OSQCreate(&s_ConnCfm[0], (sizeof(s_ConnCfm)/sizeof(s_ConnCfm[0])));
        if(NULL == s_ConnCfmQ){
            MD_DEBUG("Message snd confim quere create failed!\r\n");
            return 0;
        }
    }

    /*打开与模块的通信串口，初始化数据接收*/
    if(!MD_LowLayInit()){
        MD_DEBUG("MD_LowLayInit failed!\r\n");
        return 0;
    }

    /*创建控制进程*/
    OSTaskCreateExt(MD_Task,
        NULL,
        NULL,
        0,
        2,
        NULL,
        1024,
        NULL,
        OS_TASK_OPT_STK_CHK);

    return 1;
}



