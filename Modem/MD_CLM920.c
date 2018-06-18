#include "MDAtCmd.h"
#include "MDAtProc.h"

eMDErrCode CLM920_ATI_Hdl(sMDAtCmdRsp *pRsp, void * arg)
{
    if(pRsp->isPositive){
        MD_DEBUG("Positive Rsp:%d\r\n%s", pRsp->len, pRsp->buf);
    }else{
        MD_DEBUG("Negative Rsp:%d\r\n%s", pRsp->len, pRsp->buf);
    }
    
    return MDE_OK;
}

/*将模块初始化到网络注册成功状态需要发送的AT指令表*/ 
const sMDAtCmdItem s_CLM920InitCmdTable[] = {
    {cmdATI,        3,  5,  NULL,   "CLM920"},
//    {cmdATCPIN,     3,  5,  NULL,   "READY"},
//    {cmdATCGDCONT,  3,  5,  NULL,   "OK"},
//    {cmdATCGATT,    3,  5,  NULL,   "+CGATT: 1"},
//    {cmdATCGACT,    3,  5,  NULL,   "OK"},
//    {cmdATCGREG,    3,  5,  MD_ATCGREG_HDL,   NULL},
};

int MD_Ip2Str(uint8_t *pDes, uint32_t ip)
{
    return sprintf(pDes, "%d,%d,%d,%d", ip>>24, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
}

eMDErrCode CLM920_Init(void)
{
    return MD_ATCmdTableSnd(s_CLM920InitCmdTable, TABLE_SIZE(s_CLM920InitCmdTable));
}

eMDErrCode CLM920_SendIpData(uint8_t fd, const sMDTcpData *pData)
{
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+IPSEND=%d,%d,\"%s\",%d\r\n",
            fd,
            pData->len,
            pData->pIp,
            pData->port);
            
    MD_DEBUG("Cmd:%s\r\n", cmdBuf);
    
    return MD_ATDataSend(cmdBuf, pData->pData, pData->len, 10);
}

