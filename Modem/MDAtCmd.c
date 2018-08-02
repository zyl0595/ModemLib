/* 
* �ļ����ƣ�MDATCmd.c
* ժ    Ҫ��һЩͨ��ATָ��Ķ��弰��Ӧ����ص�������ʵ��
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#include <stdlib.h>
#include <string.h>
#include "MDTools.h"

/********************************** ͨ��ATָ��� **************************************/
const unsigned char cmdATZ[] = "ATZ\r\n";               /*�ָ���������*/
const unsigned char cmdATE0[] = "ATE0\r\n";             /*�رջ���*/
const unsigned char cmdATE1[] = "ATE1\r\n";             /*�򿪻���*/
const unsigned char cmdATI[] = "ATI\r\n";               /*��ȡģ����Ϣ*/
const unsigned char cmdATGSN[] = "AT+GSN\r\n";            /*��ȡģ��IMEI��*/
const unsigned char cmdATCPIN[] = "AT+CPIN?\r\n";       /*���SIM״̬*/
const unsigned char cmdATCIMI[] = "AT+CIMI\r\n";        /*��ȡSIM��IMSI��*/
const unsigned char cmdATCSQ[] = "AT+CSQ\r\n";          /*��ȡ�ź�����*/
const unsigned char cmdATCREG[] = "AT+CREG?\r\n";       /*�������ע��״̬*/
const unsigned char cmdATCGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";/*����PDP������*/
const unsigned char cmdATCGATT[] = "AT+CGATT?\r\n";     /*������總��״̬*/
const unsigned char cmdATCGACT[] = "AT+CGACT=1,1\r\n";  /*����PDP������*/
const unsigned char cmdATCGREG[] = "AT+CGREG?\r\n";     /*���GPRS����ע��״̬*/

/********************************** ͨ�÷��������� ************************************/
const unsigned char urcIPNETOPEN[] ="IPNETOPEN";


/*
* �������ܣ�����ָ��ؽ���ж�ģ�鵱ǰGPRS����״̬�Ƿ���ע��
* ����˵����
*     pRsp: ������������յ���ATָ����Ӧ
*
* ����ֵ��ע��ɹ�����MDE_OK�����򷵻�MDE_ERROR
*/ 
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t stat;

    pFind =  strstr(pRsp->buf, "+CGREG");
    if(NULL != pFind){
        pFind = strstr(pRsp->buf, ",");

        if(NULL != pFind){
            pFind++;
            stat = atoi(pFind);
            MD_DEBUG("Get CGREG:%d\r\n", stat);

            if(NULL != pArg){
                *(uint8_t *)pArg = stat;
            }

            if(('1' == *pFind) || ('5' == *pFind)){
                return MDE_OK;
            }
        }
    }

    return MDE_ERROR;
}


/*
* �������ܣ�����ָ��ؽ���ж�ģ�鵱ǰ����ע��״̬
* ����˵����
*     [in ]pRsp: ���յ���ATָ����Ӧ
*     [out]pArg: ����ע������ע��״̬����
*
* ����ֵ��ע��ɹ�����MDE_OK�����򷵻�MDE_ERROR
*/ 
eMDErrCode MD_ATCREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t stat;

    pFind =  strstr(pRsp->buf, "+CREG");
    if(NULL != pFind){
        pFind = strstr(pRsp->buf, ",");

        if(NULL != pFind){
            pFind++;
            stat = atoi(pFind);
            MD_DEBUG("Get CREG:%d\r\n", stat);

            if(NULL != pArg){
                *(uint8_t *)pArg = stat;
            }

            if((1 == stat) || (5 == stat)){
                return MDE_OK;
            }
        }
    }

    return MDE_ERROR;
}



/*
* �������ܣ�����ָ��ؽ���ж�ģ�鵱ǰGPRS���總��״̬
* ����˵����
*     [in ]pRsp: ���յ���ATָ����Ӧ
*     [out]pArg: ����ע�����總��״̬����
*
* ����ֵ�����ŷ���MDE_OK��δ���ŷ���MDE_ERROR
*/ 
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t val;
    uint8_t *pVal = (uint8_t *)pArg;

    pFind =  strstr(pRsp->buf, "+CGATT:");
    if(NULL != pFind){
        pFind += strlen("+CGATT:");
        val = atoi(pFind);

        /*����ϵͳ��Ϣ*/
        if(NULL != pVal){
            *pVal = val;
        }

        if(1 == val){
            return MDE_OK;
        }
    }

    return MDE_ERROR;
}

/*
* �������ܣ�����ָ��ؽ����ȡ��ǰ�ź�����
* ����˵����
*     [in ]pRsp: ���յ���ATָ����Ӧ
*     [out]pArg: �洢�ź�������Ϣ
*/ 
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t rssi, ber;
    sMDCSQ *pCsq = (sMDCSQ *)pArg;

    /*��ȡ�����ź�ǿ��ָʾֵ(received signal strength indication)*/
    pFind =  strstr(pRsp->buf, "+CSQ:");
    if(NULL != pFind){
        pFind += strlen("+CSQ:");
        rssi = atoi(pFind);
    }else{
        MD_DEBUG("CSQ rsp error!\r\n");
        return MDE_ERROR;
    }

    /*��ȡͨ��������(channel bit error rate)*/
    pFind = strstr(pFind, ",");
    if(NULL != pFind){
        pFind += 1;
        ber = atoi(pFind);
    }else{
        MD_DEBUG("CSQ rsp parse failed!\r\n");
        return MDE_ERROR;
    }

    /*����ϵͳ��Ϣ*/
    if(NULL != pCsq){
        pCsq->rssi = rssi;
        pCsq->ber = ber;
    }

    MD_DEBUG("CSQ update:%d,%d\r\n", rssi, ber);

    return MDE_OK;
}


eMDErrCode MD_ATGSN_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
#define MD_IMEI_STR_LEN 15 //IMEI����

    if(pRsp->isPositive){
        if(MD_IMEI_STR_LEN == MD_GetDecStr((uint8_t *)pArg, pRsp->buf, MD_RCV_BUF_SIZE)){
            MD_DEBUG("Get IMEI:%s\r\n", (uint8_t *)pArg);
            return MDE_OK;
        }
    }
    return MDE_ERROR;
}

eMDErrCode MD_ATCIMI_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
#define MD_IMSI_STR_LEN 15 //IMEI����

    if(pRsp->isPositive){
        if(MD_IMSI_STR_LEN == MD_GetDecStr((uint8_t *)pArg, pRsp->buf, MD_RCV_BUF_SIZE)){
            MD_DEBUG("Get IMSI:%s\r\n", (uint8_t *)pArg);
            return MDE_OK;
        }
    }
    return MDE_ERROR;
}


eMDNetRegState MD_GetNetRegStatByCode(uint8_t code)
{
    switch(code){
    case 0:
        return NET_REG_UNREG;
    case 1:
        return NET_REG_HOME;
    case 2:
        return NET_REG_SEARCHING;
    case 3:
        return NET_REG_DENIED;
    case 5:
        return NET_REG_ROAM;
    default:
        return NET_REG_UNKNOW;
    }
}


eMDErrCode MD_DefaultUrcHdl(const uint8_t *pBuf, void *pArg)
{
    MD_DEBUG("Default Urc Hdl\r\n");
    return MDE_OK;
}