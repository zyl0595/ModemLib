/* 
* 文件名称：MD_CLM920.c
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

#include "MDAtCmd.h"
#include "MDAtProc.h"
#include <string.h>
#include <stdlib.h>
#include "MDTools.h"


/********************************* 私有变量 ************************************/
static uint8_t s_cmdATIPNETOPEN[] = "AT+IPNETOPEN\r\n"; /*驱动内嵌TCPIP协议栈打开Socket网络链接，获取IP地址*/
static uint8_t s_cmdATIPADDR[] = "AT+IPADDR\r\n";       /*获取当前IP地址*/

extern sMDIPData g_ipRcvData;   //TCP/IP数据接收存储（必须进行初始化给pBuf赋值使其拥有真正的存储空间）

/**************************** 引入的外部全局变量 *******************************/
extern sMDModem g_Md;

/********************************* 函数声明 ************************************/
static eMDErrCode CLM920_ATIPNETOPEN_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
static eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
static eMDErrCode CLM920_ATIPOPNE_Hdl(sMDAtCmdRsp *pRsp, void *pArg);

static eMDErrCode CLM920_Init(void);
//static eMDErrCode CLM920_SockInit(void);
static eMDErrCode CLM920_GetLocalAddr(sMDIPv4Addr *pAddr);
eMDErrCode CLM920_SockConnect(sMDSocket *pS);
static eMDErrCode CLM920_SockClose(sMDSocket *pS);
static eMDErrCode CLM920_SockSend(sMDSocket *pS, const uint8_t *pData, uint16_t len);
static eMDErrCode CLM920_GetHostByName(const uint8_t *pName, sMDIPv4Addr *pAddr);



/********************************** 模块操作函数列表函数 ********************************/
const sMDFucTable CLM920_FunTable = {
    CLM920_Init,            //DMF_SocketInit   
    CLM920_GetLocalAddr,    //DMF_GetLocalAddr 
    CLM920_SockConnect,     //DMF_SockConnect  
    CLM920_SockClose,       //DMF_SockClose    
    CLM920_SockSend,        //DMF_SockSend     
    CLM920_GetHostByName,   //DMF_GetHostByName
};

/********************************** AT命令回应处理回调函数 ******************************/
/*CLM920_Init()将模块初始化到网络注册成功状态需要发送的AT指令表*/ 
static const sMDAtCmdItem s_CLM920InitCmdTable[] = {
    {cmdATE0,       3,  5,  NULL,                  NULL},
    {cmdATGSN,      3,  5,  MD_ATGSN_HDL,          g_Md.IMEI},
    {cmdATCPIN,     3,  5,  NULL,                  "READY"},
    {cmdATCIMI,     3,  5,  MD_ATCIMI_HDL,         g_Md.IMSI},
    {cmdATCSQ,      3,  5,  MD_ATCSQ_HDL,          &(g_Md.csq)},
    {cmdATCGDCONT,  3,  5,  NULL,                  "OK"},
    {cmdATCGATT,    3,  5,  MD_ATCGATT_HDL,        NULL},
    {cmdATCGACT,    3,  5,  NULL,                  "OK"},
    {cmdATCGREG,    3,  5,  MD_ATCGREG_HDL,        NULL},
    {s_cmdATIPNETOPEN, 3,5, CLM920_ATIPNETOPEN_Hdl,g_Md.sockets},
    {s_cmdATIPADDR, 3,  5,  CLM920_ATIPADDR_Hdl,   &(g_Md.localAddr)},
};

/********************************** URC处理回调函数 **************************************/
/*CLM920非请求结果码处理函数前置声明*/
eMDErrCode CLM920_RECV_FROM_UrcHdl(const uint8_t *pBuf, sMDSockPeerAddr *pArg);
eMDErrCode CLM920_RECV_UrcHdl(const uint8_t *pBuf, sMDIPData *pIPData);
eMDErrCode CLM920_IPOPEN_UrcHdl(const uint8_t *pBuf, sMDSocket *sockList);

/*CLM920非请求结果码处理表*/
const sMDURCHdlItem s_CLM920UrcHdlTable[] = {
    {"RECV FROM",   CLM920_RECV_FROM_UrcHdl,    &g_ipRcvData.srcAddr},
    {"+RECV:",      CLM920_RECV_UrcHdl,         &g_ipRcvData},
    {"+IPOPEN",     CLM920_IPOPEN_UrcHdl,       g_Md.sockets},
    {NULL,          MD_DefaultUrcHdl,           NULL},//最后一个必须得是这个！
};









/********************************* 函数实现 ************************************/

static eMDErrCode CLM920_ATIPNETOPEN_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    if(pRsp->isPositive){
        return MDE_OK;
    }else if(strstr(pRsp->buf, "Network is already opened")){
        sMDSocket *sockList = (sMDSocket *)pArg;

        if(NULL != sockList){
            /*更新Socket连接状态*/
            
        }

        MD_DEBUG("Already Open\r\n");
        return MDE_OK;
    }else{
        return MDE_ERROR;
    }
}


//获取模块当前IP地址回调函数
static eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    eMDErrCode ret = MDE_ERROR;
    sMDIPv4Addr *pIp = (sMDIPv4Addr*)pArg;
    uint8_t *pFind;

    if((pRsp->isPositive) && (NULL != pArg)){
        pFind = strstr(pRsp->buf, "+IPADDR:");

        if(NULL != pFind){
            pFind += 9;//strlen("+IPADDR: ")
            
            ret = MD_Str2Ip(pIp, pFind);//提取返回数据中的IP信息并存放到*pIp中

            if(MDE_OK == ret){
                MD_DEBUG("Got IP addr:%d.%d.%d.%d\r\n", pIp->sVal.v4, pIp->sVal.v3, pIp->sVal.v2, pIp->sVal.v1);
            }else{
                MD_DEBUG("IP addr formate error!\r\n");
            }

        }else{
            MD_DEBUG("Got IP rsp error\r\n");
        }
    }

    return ret;
}


static eMDErrCode CLM920_ATIPOPNE_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    sMDSocket *pS = (sMDSocket*)pArg;

    if(pRsp->isPositive){
        pS->state = SOCK_OPENED;
        return MDE_OK;
    }else{
        if(strstr(pRsp->buf, "has been opened")){
            MD_DEBUG("Socket %d has been opened!\r\n", pS->id);
            return MDE_OK;
        }else{
            MD_DEBUG("Socket %d open failed : has been opened!\r\n", pS->id);
            return MDE_ERROR;
        }
    }
}


static eMDErrCode CLM920_Init(void)
{
    /*变量初始化*/

    /*模块初始化*/
    return MD_ATCmdTableSnd(s_CLM920InitCmdTable, TABLE_SIZE(s_CLM920InitCmdTable));
}


static eMDErrCode CLM920_GetLocalAddr(sMDIPv4Addr *pAddr)
{
    /*获取本地IP地址*/
    return MD_ATCmdSndRich(s_cmdATIPADDR, 5, CLM920_ATIPADDR_Hdl, pAddr);
}


eMDErrCode CLM920_SockConnect(sMDSocket *pS)
{
    const uint8_t tcp[] = "TCP";
    const uint8_t udp[] = "UDP";
    uint8_t cmdBuf[60];
    int index = 0;
    const uint8_t *pType; //链接类型

    if(MDSOCK_STREAM == pS->type){
        pType = tcp;
    }else if(MDSOCK_DGRAM == pS->type){
        pType = udp;
    }else{
        MD_DEBUG("Connect failed:err sock type!\r\n");
        return MDE_PARAM_ERR;//传入参数错误
    }

    /*组装AT命令*/
    index = sprintf(cmdBuf, "AT+IPOPEN=%d,\"%s\",\"", pS->id, pType);   //socket id, type
    index += MD_Ip2Str(cmdBuf+index, &pS->serverIp);                    //server id
    index += sprintf(cmdBuf+index, "\",%d\r\n", pS->serverPort);        //server port
    if(0 != pS->localPort){
        index += sprintf(cmdBuf+index-2, ",%d\r\n", pS->localPort);     //local port
    }

    MD_DEBUG("Connecting to %s server %d.%d.%d.%d:%d fd:%d...\r\n", pType,
        pS->serverIp.sVal.v4, pS->serverIp.sVal.v3, pS->serverIp.sVal.v2, pS->serverIp.sVal.v1,
        pS->serverPort, pS->id);

    return MD_ATCmdSndRich(cmdBuf, 30, CLM920_ATIPOPNE_Hdl, (sMDSocket *)pS);
}


eMDErrCode CLM920_SockSendTest(uint8_t fd, const sMDSockData *pData)
{
    uint8_t cmdBuf[100];
    
    /*发送数据发送指令*/
    sprintf(cmdBuf, "AT+IPSEND=%d,%d\r\n",
            fd,
            pData->len);
    return MD_ATDataSend(cmdBuf, pData->pData, pData->len, 10);
}


static eMDErrCode CLM920_SockClose(sMDSocket *pS)
{
    uint8_t cmdBuf[15];

    sprintf(cmdBuf, "AT+IPCLOSE=%d\r\n", pS->id);

    return MD_ATCmdSndRich(cmdBuf, 2, NULL, NULL);
}


static eMDErrCode CLM920_SockSend(sMDSocket *pS, const uint8_t *pData, uint16_t len)
{
    uint8_t cmdBuf[100];
    int index = 0;
    eMDErrCode ret;
    sMDAtCmdRsp rsp;

    /*合成数据发送AT指令*/
    if(MDSOCK_STREAM == pS->type){//TCP
        index = sprintf(cmdBuf, "AT+IPSEND=%d,%d\r\n", pS->id, len);
    }else if(MDSOCK_DGRAM == pS->type){//UDP
        index = sprintf(cmdBuf, "AT+IPSEND=%d,%d,\"%s\",%d\r\n", pS->id, len);
        index += MD_Ip2Str(cmdBuf+index, &pS->serverIp);
        index += sprintf(cmdBuf+index, "\",%d\r\n", pS->serverPort);
    }else{
        MD_DEBUG("Unsupported sock type!\r\n");
        return MDE_ERROR;
    }

    /*发送数据*/
    ret = MD_ATDataSend(cmdBuf, pData, len, 10);
    if(MDE_OK != ret)return ret;

    /*接收非请求结果码判断是否发送成功*/
    ret = MD_AtGetURCMsg("+IPSEND:", &rsp, 1000);
    if(MDE_OK != ret){
        MD_DEBUG("CLM920 Sock date send failed:%d\r\n", ret);
        return ret;
    }else{
        uint8_t *pFind;
        
        pFind = strstr(rsp.buf, "+IPSEND:");
        if(NULL == pFind){
            MD_DEBUG("CLM920 Sock snd URC err!\r\n");
            return MDE_ERROR;
        }else{
            int sockNo, reqSndLen, cnfSndLen;
            
            pFind += strlen("+IPSEND:");
            sockNo = atoi(pFind);
            if(sockNo != pS->id){
                MD_DEBUG("CLM920 Sock snd URC err id Y:%d N:%d\r\n", pS->id, sockNo);
                return MDE_ERROR;
            }

            pFind = MD_SkipStr(pFind, ",", 1);
            if(NULL == pFind){
                MD_DEBUG("CLM920 Sock snd URC reqSndLen err!\r\n");
                return MDE_ERROR;
            }
            reqSndLen = atoi(pFind);

            pFind = MD_SkipStr(pFind, ",", 1);
            if(NULL == pFind){
                MD_DEBUG("CLM920 Sock snd URC cnfSndLen err!\r\n");
                return MDE_ERROR;
            }
            cnfSndLen = atoi(pFind);

            if(reqSndLen != cnfSndLen){//数据发送失败，该Socket不再可用
                MD_DEBUG("CLM920 Sock snd URC  err reqSndLen:%d cnfSndLen:%d\r\n", reqSndLen, cnfSndLen);
                return MDE_ERROR;
            }else{
                return MDE_OK;
            }
        }
    }
}

//域名解析服务
static eMDErrCode CLM920_GetHostByName(const uint8_t *pName, sMDIPv4Addr *pAddr)
{
    eMDErrCode ret;
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+CDNSGIP=\"%s\"\r\n", pName);
    ret = MD_ATCmdSndRich(cmdBuf, 30, CLM920_ATIPADDR_Hdl, (void*)pAddr);

    return ret;
}

#if 0
/*
* 函数功能：接收数据TCP/IP数据
* 参数说明：
*     [out]pSN  :收到数据的Socket编号
*     [out]pDes :数据接收缓存
*     [out]pLen :接收到的数据长度
*     [out]pSrcIp:源IP和端口号
*     [in]delay:接收等待延时时间（最长等待时间3*delay !!!）
* 
* 返回值： 成功、超时 、接收缓存溢出
*/ 
static eMDErrCode CLM920_SockRcv(uint8_t *pSN, uint8_t *pDes, int *pLen, sMDSockPeerAddr *pSrcIp, uint32_t delay)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;
    uint8_t *pFind;
    uint8_t rcvLen; //接收数据总长

    /*解析数据源IP地址和端口号*/
    ret = MD_AtGetURCMsg("RECV FROM:", &rsp, delay);
    do{
        if(MDE_OK == ret){
            MD_DEBUG("CLM920 Rcv ");
            pFind = strstr(rsp.buf, "RECV FROM:");
            pFind += strlen("RECV FROM:");
            ret = MD_Str2Ip(&pSrcIp->IP, pFind);    //获取源IP地址
            if(MDE_OK != ret){
                MD_DEBUG("Src IP parse failed!\r\n");
                break;
            }
            pFind = MD_SkipStr(pFind, ":", 1);
            if(NULL == pFind){
                MD_DEBUG("Src port parse failed!\r\n");
                break;
            }
            pSrcIp->port = atoi(pFind);             //获取源端口号

            MD_DEBUG(" %d.%d.%d.%d:%d ",
                pSrcIp->IP.sVal.v4,
                pSrcIp->IP.sVal.v3,
                pSrcIp->IP.sVal.v2,
                pSrcIp->IP.sVal.v1,
                pSrcIp->port);
        }
    }while(0);

    /*获取接收数据信息（Socket编号、数据长度）*/
    ret = MD_AtGetURCMsg("+RECV:", &rsp, delay);
    if(MDE_OK != ret)return ret;
    pFind = rsp.buf + strlen("+RECV:");
    *pSN = atoi(pFind);
    pFind = MD_SkipStr(pFind, ",", 1);
    if(NULL == pFind){
        MD_DEBUG("CLM920 Rcv len parse err\r\n");
        return MDE_ERROR;
    }
    rcvLen = atoi(pFind);

    /*接收数据*/
    ret = MD_ATRcvData(pDes, pLen, rcvLen, delay);
    if(rcvLen != *pLen){
        MD_DEBUG("CLM920 Rcv len err Y:%d N:%d\r\n", rcvLen, *pLen);
        return MDE_ERROR;
    }else{
        return MDE_OK;
    }
}
#endif


eMDErrCode CLM920_RECV_FROM_UrcHdl(const uint8_t *pBuf, sMDSockPeerAddr *pSrcIp)
{
    uint8_t *pFind;
    eMDErrCode ret;

    /*参数检查*/
    if(NULL == pSrcIp){
        MD_DEBUG("CLM920_RECV_FROM_UrcHdl para err!\r\n");
        return MDE_ERROR;
    }

    pFind = strstr(pBuf, "RECV FROM:");
    if(NULL == pFind){
        MD_DEBUG("CLM920 RECV_FROM UrcHdl err!\r\n");
        return MDE_ERROR;
    }
    pFind += strlen("RECV FROM:");
    ret = MD_Str2Ip(&pSrcIp->IP, pFind);    //获取源IP地址
    if(MDE_OK != ret){
        MD_DEBUG("Src IP parse failed!\r\n");
        return MDE_ERROR;
    }
    pFind = MD_SkipStr(pFind, ":", 1);
    if(NULL == pFind){
        MD_DEBUG("Src port parse failed!\r\n");
        return MDE_ERROR;
    }
    pSrcIp->port = atoi(pFind);             //获取源端口号

    MD_DEBUG("Rcv form: %d.%d.%d.%d:%d\r\n",
        pSrcIp->IP.sVal.v4,
        pSrcIp->IP.sVal.v3,
        pSrcIp->IP.sVal.v2,
        pSrcIp->IP.sVal.v1,
        pSrcIp->port);

    return MDE_OK;
}

eMDErrCode CLM920_RECV_UrcHdl(const uint8_t *pBuf, sMDIPData *pIPData)
{
    uint8_t *pFind;
    eMDErrCode ret;
    uint32_t dataLen;
    sMDDataHdl *pRcvBuf;

    /*参数检查*/
    if(NULL == pIPData){
        MD_DEBUG("CLM920_RECV_UrcHdl para err!\r\n");
        return MDE_PARAM_ERR;
    }
    pRcvBuf = &pIPData->rcvData;

    pFind = strstr(pBuf, "+RECV:");
    if(NULL == pFind){
        MD_DEBUG("CLM920 RECV UrcHdl err!\r\n");
        return MDE_ERROR;
    }
    pFind += strlen("+RECV:");
    pIPData->sockNum = atoi(pFind);
    pFind = MD_SkipStr(pFind, ",", 1);
    if(NULL == pFind){
        MD_DEBUG("CLM920 Rcv len parse err\r\n");
        return MDE_ERROR;
    }
    dataLen = atoi(pFind);

    /*判断接收缓存够不够大*/
    if(dataLen >= pRcvBuf->bufSize-1){
        MD_DEBUG("CLM920 Rcv buf too small!\r\n");
        return MDE_ERROR;
    }

    /*接收数据*/
    ret = MD_ATRcvData(pRcvBuf->pBuf, &pRcvBuf->len, dataLen, 500);
    if(pRcvBuf->len != dataLen){
        MD_DEBUG("CLM920 Rcv len err Y:%d N:%d\r\n", dataLen, pRcvBuf->len);
        return MDE_ERROR;
    }else{
        pRcvBuf->pBuf[dataLen] = '\0';
        pIPData->isNewRcved = TRUE;
        MD_DEBUG("CLM920 Sock:%d Rcv data len:%d\r\n%s\r\n", pIPData->sockNum, pRcvBuf->len, pRcvBuf->pBuf);
        return MDE_OK;
    }
}

eMDErrCode CLM920_IPOPEN_UrcHdl(const uint8_t *pBuf, sMDSocket *sockList)
{
    if(NULL != sockList){

    }
    return MDE_OK;
}