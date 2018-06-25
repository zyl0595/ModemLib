#include "MDAtCmd.h"
#include "MDAtProc.h"
#include <string.h>
#include "MDTools.h"

/***************** 私有AT指令 ********************/
static uint8_t s_cmdATIPNETOPEN[] = "AT+IPNETOPEN\r\n"; /*驱动内嵌TCPIP协议栈打开Socket网络链接，获取IP地址*/
static uint8_t s_cmdATIPADDR[] = "AT+IPADDR\r\n";       /*获取当前IP地址*/

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
            
            ret = MD_Str2Ip(pIp, pFind);

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
    {cmdATE0,       3,  5,  NULL,                  NULL},
    {cmdATI,        3,  5,  CLM920_ATI_Hdl,        "CLM920"},
    {cmdATCPIN,     3,  5,  NULL,                  "READY"},
    {cmdATCGDCONT,  3,  5,  NULL,                  "OK"},
    {cmdATCGATT,    3,  5,  NULL,                  "+CGATT: 1"},
    {cmdATCGACT,    3,  5,  NULL,                  "OK"},
    {cmdATCGREG,    3,  5,  MD_ATCGREG_HDL,        NULL},
};


eMDErrCode CLM920_Init(void)
{
    return MD_ATCmdTableSnd(s_CLM920InitCmdTable, TABLE_SIZE(s_CLM920InitCmdTable));
}

eMDErrCode CLM920_SokctInit(void)
{
    sMDAtCmdRsp rsp;
    eMDErrCode ret;
    sMDIPv4Addr ipAddr;

    /*触发建立Socket链接*/
    ret = MD_ATCmdSnd(s_cmdATIPNETOPEN, 10, CLM920_ATIPNETOPEN_Hdl, NULL);
    if(MDE_ERROR == ret){
        MD_DEBUG("IP net open failed");
        return MDE_ERROR;
    }

    /*检查Socket建立是否成功*/
    if(MDE_ALREADYON != ret){

        ret = MD_GetURCMsg(urcIPNETOPEN, &rsp, 2);//等待URC 2秒钟

        if(MDE_OK == ret){
            if(!strstr(rsp.buf, "+IPNETOPEN: 0")){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }else{
            ret = MD_ATCmdSnd("AT+IPNETOPEN?\r\n", 10, NULL, "IPNETOPEN: 1");
            if(MDE_OK != ret){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }
    }

    /*获取本地IP地址*/
    ret = MD_ATCmdSnd(s_cmdATIPADDR, 5, CLM920_ATIPADDR_Hdl, &ipAddr);

    return ret;
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

eMDErrCode CLM920_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{
    eMDErrCode ret;
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+CDNSGIP=\"%s\"\r\n", pName);
    ret = MD_ATCmdSnd(cmdBuf, 30, CLM920_ATIPADDR_Hdl, (void*)pAddr);

    return ret;
}