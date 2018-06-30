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
#include "MDTools.h"


/********************************* 私有变量 ************************************/
static uint8_t s_cmdATIPNETOPEN[] = "AT+IPNETOPEN\r\n"; /*驱动内嵌TCPIP协议栈打开Socket网络链接，获取IP地址*/
static uint8_t s_cmdATIPADDR[] = "AT+IPADDR\r\n";       /*获取当前IP地址*/

/**************************** 引入的外部全局变量 *******************************/
extern sMDModem g_MdInfo;


/********************************* 函数声明 ************************************/
eMDErrCode CLM920_ATI_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode CLM920_ATIPNETOPEN_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg);





//const sMDFucTable CLM920_FucTable = {
//    /*模块及SIM卡状态获取*/
//    .GetModelInfo = NULL,
//    .GetIMEI = NULL,
//    .CheckSIM = NULL,
//    .GetIMSI = NULL,
//    .GetCSQ = NULL,
//
//    /*选择网络并注册*/
//    .SelectNet = NULL,
//    .RegToNet = NULL,
//    .CheckNetState = NULL,
//
//    /*GPRS网络操作*/
//    .DefPDPContext = NULL,
//    .ActPDPContext = NULL,
//    .SetUsrPwd = NULL,
//    .ChekNetAttch = NULL,
//
//    /*内嵌TCP/IP协议栈操作*/
//    .SocketInit = NULL,
//    .GetLocalAddr = NULL,
//    .SockConnect = NULL,
//    .SockClose = NULL,
//    .SockSend = NULL,
//    .GetHostByName = NULL,
//};

const sMDFucTable CLM920_FucTable = {
    /*模块及SIM卡状态获取*/
        //GetModelInfo;
        //GetIMEI;
        //CheckSIM;
        //GetIMSI;
        //GetCSQ;

    /*选择网络并注册*/
        //SelectNet;
        //RegToNet;
        //CheckNetState;

    /*GPRS网络操作*/
        //DefPDPContext;
        //ActPDPContext;
        //SetUsrPwd;
        //ChekNetAttch;

    /*内嵌TCP/IP协议栈操作*/
        //SocketInit;
        //GetLocalAddr;
        //SockConnect;
        //SockClose;
        //SockSend;
        //GetHostByName;
};


/********************************* 函数实现 ************************************/
eMDErrCode CLM920_ATI_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    if(pRsp->isPositive){

        if(strstr(pRsp->buf, pArg)){
            return MDE_OK;
        }else{
            MD_DEBUG("Unknow modem type!\r\n");
        }
    }
    return MDE_ERROR;
}

eMDErrCode CLM920_ATIPNETOPEN_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    if(pRsp->isPositive){
        return MDE_OK;
    }else if(strstr(pRsp->buf, "Network is already opened")){
        //MD_DEBUG("Already Open\r\n");
        return MDE_ALREADYON;
    }else{
        return MDE_ERROR;
    }
}


//获取模块当前IP地址回调函数
eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
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
                MD_DEBUG("IP addr switch error!\r\n");
            }

        }else{
            MD_DEBUG("Got IP error\r\n");
        }
    }

    return ret;
}

/*将模块初始化到网络注册成功状态需要发送的AT指令表*/ 
const sMDAtCmdItem s_CLM920InitCmdTable[] = {
    {cmdATZ,        3,  5,  NULL,                  NULL},
    {cmdATE0,       3,  5,  NULL,                  NULL},
    {cmdATI,        3,  5,  CLM920_ATI_Hdl,        "CLM920"},
    {cmdATCPIN,     3,  5,  NULL,                  "READY"},
    {cmdATCSQ,      3,  5,  MD_ATCSQ_HDL,          (void*)&g_MdInfo},
    {cmdATCGDCONT,  3,  5,  NULL,                  "OK"},
    {cmdATCGATT,    3,  5,  MD_ATCGATT_HDL,        NULL},
    {cmdATCGACT,    3,  5,  NULL,                  "OK"},
    {cmdATCGREG,    3,  5,  MD_ATCGREG_HDL,        NULL},
};


eMDErrCode CLM920_Init(void)
{
    return MD_ATCmdTableSnd(s_CLM920InitCmdTable, TABLE_SIZE(s_CLM920InitCmdTable));
}

eMDErrCode CLM920_SokctInit(sMDModem *pMdInfo)
{
    sMDAtCmdRsp rsp;
    eMDErrCode ret;

    /*触发建立Socket链接*/
    ret = MD_ATCmdSndWithCb(s_cmdATIPNETOPEN, 10, CLM920_ATIPNETOPEN_Hdl, NULL);
    if(MDE_ERROR == ret){
        MD_DEBUG("IP net open failed");
        return MDE_ERROR;
    }

    /*检查Socket建立是否成功*/
    if(MDE_ALREADYON != ret){

        ret = MD_AtGetURCMsg(urcIPNETOPEN, &rsp, 1);//等待URC 1秒钟，等不到就主动查

        if(MDE_OK == ret){
            if(!strstr(rsp.buf, "+IPNETOPEN: 0")){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }else{//没有接收到反应Socket连接状态的URC，主动进行查询
            ret = MD_ATCmdSndWithCb("AT+IPNETOPEN?\r\n", 10, NULL, "IPNETOPEN: 1");
            if(MDE_OK != ret){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }
    }

    /*获取本地IP地址*/
    ret = MD_ATCmdSndWithCb(s_cmdATIPADDR, 5, CLM920_ATIPADDR_Hdl, &(pMdInfo->localAddr));

    return ret;
}


eMDErrCode CLM920_Connect(int s)
{
    
}


eMDErrCode CLM920_SendIpData(uint8_t fd, const sMDSockData *pData)
{
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+IPSEND=%d,%d,\"%s\",%d\r\n",
            fd,
            pData->len,
            pData->pIp,
            pData->port);
            
    //MD_DEBUG("Cmd:%s\r\n", cmdBuf);
    
    return MD_ATDataSend(cmdBuf, pData->pData, pData->len, 10);
}


//域名解析服务
eMDErrCode CLM920_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{
    eMDErrCode ret;
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+CDNSGIP=\"%s\"\r\n", pName);
    ret = MD_ATCmdSndWithCb(cmdBuf, 30, CLM920_ATIPADDR_Hdl, (void*)pAddr);

    return ret;
}