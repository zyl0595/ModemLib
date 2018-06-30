/* 
* �ļ����ƣ�MD_CLM920.c
* ժ    Ҫ��
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#include "MDAtCmd.h"
#include "MDAtProc.h"
#include <string.h>
#include "MDTools.h"


/********************************* ˽�б��� ************************************/
static uint8_t s_cmdATIPNETOPEN[] = "AT+IPNETOPEN\r\n"; /*������ǶTCPIPЭ��ջ��Socket�������ӣ���ȡIP��ַ*/
static uint8_t s_cmdATIPADDR[] = "AT+IPADDR\r\n";       /*��ȡ��ǰIP��ַ*/

/**************************** ������ⲿȫ�ֱ��� *******************************/
extern sMDModem g_MdInfo;


/********************************* �������� ************************************/
eMDErrCode CLM920_ATI_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode CLM920_ATIPNETOPEN_Hdl(sMDAtCmdRsp *pRsp, void *pArg);
eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg);





//const sMDFucTable CLM920_FucTable = {
//    /*ģ�鼰SIM��״̬��ȡ*/
//    .GetModelInfo = NULL,
//    .GetIMEI = NULL,
//    .CheckSIM = NULL,
//    .GetIMSI = NULL,
//    .GetCSQ = NULL,
//
//    /*ѡ�����粢ע��*/
//    .SelectNet = NULL,
//    .RegToNet = NULL,
//    .CheckNetState = NULL,
//
//    /*GPRS�������*/
//    .DefPDPContext = NULL,
//    .ActPDPContext = NULL,
//    .SetUsrPwd = NULL,
//    .ChekNetAttch = NULL,
//
//    /*��ǶTCP/IPЭ��ջ����*/
//    .SocketInit = NULL,
//    .GetLocalAddr = NULL,
//    .SockConnect = NULL,
//    .SockClose = NULL,
//    .SockSend = NULL,
//    .GetHostByName = NULL,
//};

const sMDFucTable CLM920_FucTable = {
    /*ģ�鼰SIM��״̬��ȡ*/
        //GetModelInfo;
        //GetIMEI;
        //CheckSIM;
        //GetIMSI;
        //GetCSQ;

    /*ѡ�����粢ע��*/
        //SelectNet;
        //RegToNet;
        //CheckNetState;

    /*GPRS�������*/
        //DefPDPContext;
        //ActPDPContext;
        //SetUsrPwd;
        //ChekNetAttch;

    /*��ǶTCP/IPЭ��ջ����*/
        //SocketInit;
        //GetLocalAddr;
        //SockConnect;
        //SockClose;
        //SockSend;
        //GetHostByName;
};


/********************************* ����ʵ�� ************************************/
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


//��ȡģ�鵱ǰIP��ַ�ص�����
eMDErrCode CLM920_ATIPADDR_Hdl(sMDAtCmdRsp *pRsp, void *pArg)
{
    eMDErrCode ret = MDE_ERROR;
    sMDIPv4Addr *pIp = (sMDIPv4Addr*)pArg;
    uint8_t *pFind;

    if((pRsp->isPositive) && (NULL != pArg)){
        pFind = strstr(pRsp->buf, "+IPADDR:");

        if(NULL != pFind){
            pFind += 9;//strlen("+IPADDR: ")
            
            ret = MD_Str2Ip(pIp, pFind);//��ȡ���������е�IP��Ϣ����ŵ�*pIp��

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

/*��ģ���ʼ��������ע��ɹ�״̬��Ҫ���͵�ATָ���*/ 
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

    /*��������Socket����*/
    ret = MD_ATCmdSndWithCb(s_cmdATIPNETOPEN, 10, CLM920_ATIPNETOPEN_Hdl, NULL);
    if(MDE_ERROR == ret){
        MD_DEBUG("IP net open failed");
        return MDE_ERROR;
    }

    /*���Socket�����Ƿ�ɹ�*/
    if(MDE_ALREADYON != ret){

        ret = MD_AtGetURCMsg(urcIPNETOPEN, &rsp, 1);//�ȴ�URC 1���ӣ��Ȳ�����������

        if(MDE_OK == ret){
            if(!strstr(rsp.buf, "+IPNETOPEN: 0")){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }else{//û�н��յ���ӦSocket����״̬��URC���������в�ѯ
            ret = MD_ATCmdSndWithCb("AT+IPNETOPEN?\r\n", 10, NULL, "IPNETOPEN: 1");
            if(MDE_OK != ret){
                MD_DEBUG("IP net open failed");
                return MDE_ERROR;
            }
        }
    }

    /*��ȡ����IP��ַ*/
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


//������������
eMDErrCode CLM920_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{
    eMDErrCode ret;
    uint8_t cmdBuf[100];

    sprintf(cmdBuf, "AT+CDNSGIP=\"%s\"\r\n", pName);
    ret = MD_ATCmdSndWithCb(cmdBuf, 30, CLM920_ATIPADDR_Hdl, (void*)pAddr);

    return ret;
}