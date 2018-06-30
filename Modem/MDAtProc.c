/* 
* �ļ����ƣ�MDATProc.c
* ժ    Ҫ���ṩһЩͨ�õ�ATָ���������ʵ�֣���Щ�������Ա�������ģ�鷢��
*           ATָ�����ģ����Ӧ����
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#include <string.h>
#include <stdlib.h>
#include "MDConfig.h"
#include "MDAtCmd.h"
#include "MDPort.h"
#include "MDType.h"

/************************** ˽�ж��� *********************************/
#define AT_OK			"OK"
#define AT_ERROR		"ERROR"
#define AT_RTR          ">"     //Ready To Recive
#define CRLF			"\r\n"
#define CR				0x0Du
#define LF				0x0Au

#define IS_AT_RSP_OK(str) (strstr(str, AT_OK))
#define IS_AT_RSP_ERR(str) (strstr(str, AT_ERROR))


/************************** ˽�б���  *********************************/
//static uint8_t s_rcvBuf[];

/************************** ����ʵ��  *********************************/
/*
* �������ܣ����õײ����ݷ��ͽӿڷ���AT���ģ��
* ����˵����
*     [in]pCmd :   Ҫ���͵�AT��������� NULL terminated string���� 
*
* ����ֵ�����ͽ�����ɹ���ʧ��(����д��ʧ��)
*/ 
static eMDErrCode MD_SndATCmd(const uint8_t *pCmd)
{
    uint32_t cmdLen = strlen(pCmd);

    if(cmdLen == MD_WriteBuf(pCmd, cmdLen)){
        return MDE_OK;
    }else{
        return MDE_TTYSERR;
    }
}


/*
* �������ܣ��ӽ��ջ����н���ATָ����Ӧ����
* ����˵����
*     [out]pRsp  :   ָ��洢AT��Ӧ���ݵĻ���
*     [in] delay :   �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��10ms)�� 
*     [in] pTail :   ������Ӧ��β�ַ�������">"��NULLʱĬ��ƥ��"OK"��"ERROR"
*
* ����ֵ����ȡ������ɹ���ʧ��(��ʱ �����ջ������)
* ע�⣺��pStr��ΪNULLʱ��һƥ�䵽pStr�����������������շ���
*/ 
static eMDErrCode MD_GetATRsp(sMDAtCmdRsp *pRsp, uint32_t delay, const uint8_t *pTail)
{
    uint32_t waitCnt = 0;   //�ȴ�ģ����ӦATָ���ʱ
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;        //һ�ζ�ȡ���ջ���õ������ݳ���
    uint16_t remainedSpac;  //ʣ����ջ���ռ�
    uint8_t *pFind;
    uint8_t *pCurFind = pRsp->buf;

    do{
        remainedSpac = MD_RCV_BUF_SIZE-1 - rcvIndex;//����ʣ��ռ�
        if(remainedSpac > 0){
            rcvLen = MD_ReadBuf(pRsp->buf+rcvIndex, remainedSpac);
        }else{
            return MDE_BUFOVFL;
        }

        if(rcvLen > 0){
            rcvIndex += rcvLen;
            pRsp->buf[rcvIndex] = '\0';

            /*���������ַ���*/
            if((NULL != pTail) && strstr(pCurFind, pTail)){
                break;
            }

            /*���в��ң�"OK"��"ERROR"*/
            pFind = strstr(pCurFind, CRLF);//��������
            if(pFind){
                if(IS_AT_RSP_OK(pCurFind)){
                    pRsp->isPositive = TRUE;
                    break;
                }else if(IS_AT_RSP_ERR(pCurFind)){
                    pRsp->isPositive = FALSE;
                    break;
                }
                pCurFind = pFind + strlen(CRLF);//�´δ���һ�п�ʼ��������ٶ�
            }

        }else{
            MD_Delay(10);
            waitCnt++;
        }
    }while(waitCnt <= delay);

    pRsp->len = rcvIndex;

    if(waitCnt <= delay){
        return MDE_OK;
    }else{
        return MDE_TIMEOUT;
    }
}


/*
* �������ܣ���ģ�鷢��ATָ�������ģ����Ӧ
*
* ����˵����
*     [in]pCmd  :   ָ��Ҫ���͵�ATָ��(NULL terminated!)�� 
*     [in]delay :   �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��s) �� 
*     [out]pRsp :   ����ģ��ָ��ؽ��
*
* ע�⣺����������Ҫ���͵�ģ���ATָ��ĺϷ��Խ��м��
* ����ֵ���ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_ATCmdSnd(const uint8_t *pCmd, uint8_t delay, sMDAtCmdRsp *pRsp)
{
    eMDErrCode ret;

    /*����AT���������*/
    MD_DEBUG("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*����ָ��ظ�*/
    ret = MD_GetATRsp(pRsp, 100*delay, NULL);
    if(MDE_OK == ret){
        MD_DEBUG("Rsp:%s", pRsp->buf);
    }else{
        MD_DEBUG("AT cmd rsp failed!\r\n");
    }

    return ret;
}


/*
* �������ܣ���ģ�鷢��ATָ�������ģ����Ӧ��������Ӧ���ݽ��д����ģ����Ӧ��
*           �ݵĴ��������ַ�ʽ��
*           1.���д���ص�����������£����ûص�������ģ���Ӧ���ݽ��н������� 
*           2.��û�����ûص����������д���ƥ���ַ���ʱ����ģ����Ӧ������ƥ��ָ 
*             ���ַ�����������ƥ�������ض�Ӧֵ�� 
*           3.��û�����ûص�������Ŀ��ƥ���ַ���ʱ�����ж�ģ���в���������Ӧʱ��
*             �㷵�سɹ������򷵻�ʧ�ܡ�
* ����˵����
*     [in]pCmd  :   ָ��Ҫ���͵�ATָ��(NULL terminated!)�� 
*     [in]delay :   �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��s) �� 
*     [in]pRspHdl:  ģ�鷵�ؽ������ص�����
*     [in]pArg  :   ��(pRspHdl != NULL)ʱ��ΪpRspHdl��������(pRspHdl == NULL)ʱ��
*                   pArg�������ؽ��ƥ��Ŀ���ַ�����ΪNULLʱĬ�ϱȽ�"OK"����"ERROR"
*
* ע�⣺����������Ҫ���͵�ģ���ATָ��ĺϷ��Խ��м��
* ����ֵ���ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_ATCmdSndWithCb(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;

    /*ִ��AT�����*/
    ret = MD_ATCmdSnd(pCmd, delay, &rsp);
    
    /*��Ӧ���ݽ�������*/
    if(MDE_OK == ret){
        /*���������ַ�ʽ�Ի�Ӧ���ݽ��д���*/ 
        if(NULL != pRspHdl){    //1.�лص��������ûص��������� 
            return  pRspHdl(&rsp, pArg);
        }else if(NULL != pArg){ //2.û�лص�����������ƥ���ַ�������������ƥ����
            if(strstr(rsp.buf, pArg)){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }else{                  //3.��������Ӧ��ֱ�ӷ��ؽ��
            if(TRUE == rsp.isPositive){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }
    }else{
        return ret;
    }
}


/*
* �������ܣ�ִ�����ڷ������ݵ�ATָ��������ݣ��緢�Ͷ��š�����TCP���ݱ�
* 
* ����˵����
*   [in]pTable: ָ��Ҫ���͵�ATָ��� 
*   [in]pData : ָ��Ҫͨ������ָ��͵����ݣ�
*   [in]len :   Ҫ�������ݵĳ���
*   [in]delay:  �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��s) 
* ����ֵ��  �ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_ATDataSend(const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;
    
    /*����AT���������*/
    MD_DEBUG("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }
    
    /*�ȴ�ģ��ȷ��*/
    MD_DEBUG("Rcving '>'...\r\n");
    ret = MD_GetATRsp(&rsp, 100*delay, AT_RTR);
    if(MDE_OK != ret){
        MD_DEBUG("Get '>' faild!\r\n");
        return ret;
    }
    
    /*��������*/
    MD_DEBUG("Snd data len:%d\r\n", len);
    if(len != MD_WriteBuf(pData, len)){
        MD_DEBUG("Snd failed!\r\n");
        return MDE_TTYSERR;
    }
    
    /*�ȴ�ȷ��*/
    ret = MD_GetATRsp(&rsp, 100*delay, NULL);
    if(MDE_OK == ret){
        if(rsp.isPositive){//�յ�"OK"
            MD_DEBUG("Data snd succ!\r\n");
            return MDE_OK;
        }else{//�յ�"ERROR"
            MD_DEBUG("Data snd failed!\r\n");
            return MDE_ERROR;
        }
    }else{
        MD_DEBUG("Data snd failed! %d\r\n", ret);
        return ret;
    }
}


/*
* �������ܣ�����˳����ģ�鷢��ATָ����ڵ�����ָ� ����һ��ָ��ͷ���ʧ�ܳ�
*           ����ط��������򷵻ش��󣬷��򷵻سɹ� 
* ����˵����
*     [in]pTable:   ָ��Ҫ���͵�ATָ��� 
*     [in]size  :   ָ�����ָ��������
* ����ֵ�� �ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_ATCmdTableSnd(const sMDAtCmdItem *pTable, uint8_t size)
{
    //uint8_t buf[100];//��ָ̬����� 
    eMDErrCode ret;
    uint8_t i;
    uint8_t tryCnt = 0;
    const uint8_t *pCmd; 
    
    for(i=0; i<size; ){
        
        pCmd = pTable[i].pCmd;
        
        /*��Ϊ��ָ̬������ɶ�ָ̬�����Ҫ��������ʱ���������ָ�*/
        //if(cmdATCGDCONT == pCmd){//���ݵ�ǰ��Ӫ������PDP�����Ķ���ָ�� 
        //    sprintf(buf, "AT+CGDCONT %s %d\r\n", "123", 15);
        //    pCmd = buf;
        //}
        
        ret = MD_ATCmdSndWithCb(pCmd,
                          pTable[i].delay,
                          pTable[i].rspHdl,
                          pTable[i].pArg);
        tryCnt ++;
                          
        if(MDE_OK == ret){
            i++;
            tryCnt = 0;
            MD_Delay(MD_ATCMD_ITV);
        }else{
            if(tryCnt >= pTable[i].tryTms){
                MD_DEBUG("AT cmd falied:\r\n----\r\n%s----\r\n", pTable[i].pCmd);
                return ret;
                //i++; tryCnt=0; //����ʱ�򿪣���ʹһ��ָ�����Ҳ���Լ�����һ��
            }else{
                MD_Delay(MD_ATCMD_ITV_WHENERR);//�����ȴ��ٽ����ط�
            }
        }
    }
    
    return MDE_OK;
}


/*
* �������ܣ�����ģ��ķ���������Ӧ
* ����˵����
*     [out]pRsp:    ��ŷ���������Ӧ�� 
*     [in]delay:    �������ȴ�ʱ��(s)��
* ����ֵ�� �ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint8_t delay)
{
    return MD_GetATRsp(pRsp, 100*delay, pUrc);
}


/*
* �������ܣ�����ָ��ؽ���ж�ģ�鵱ǰ����״̬�Ƿ���ע��
* ����˵����
*     pRsp: ������������յ���ATָ����Ӧ
*/ 
eMDErrCode MD_ATCGREG_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    
    pFind =  strstr(pRsp->buf, "+CGREG");
    if(NULL != pFind){
        pFind += 7;
        pFind = strstr(pRsp->buf, ",");
        
        if(NULL != pFind){
            pFind++;
            if('1' == *pFind){//�ڶ�������Ϊ1˵��ע��ɹ� 
                return MDE_OK;
            }
        }
    }
    
    return MDE_ERROR;
}


/*
* �������ܣ�����ָ��ؽ���ж�ģ�鵱ǰ���總��״̬
* ����˵����
*     pRsp: ������������յ���ATָ����Ӧ
*/ 
eMDErrCode MD_ATCGATT_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t val;
    sMDModem *pMD = (sMDModem *)pArg;

    pFind =  strstr(pRsp->buf, "+CGATT:");
    if(NULL != pFind){
        pFind += strlen("+CGATT:");
        val = atoi(pFind);

        /*����ϵͳ��Ϣ*/
        if(NULL != pMD){
            //��δʵ��
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
*     pRsp: ������������յ���ATָ����Ӧ
*/ 
eMDErrCode MD_ATCSQ_HDL(sMDAtCmdRsp *pRsp, void *pArg)
{
    uint8_t *pFind;
    uint8_t rssi, ber;
    sMDModem *pMD = (sMDModem *)pArg;
    
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
    if(NULL != pMD){
        pMD->mdInfo.csq.rssi = rssi;
        pMD->mdInfo.csq.ber = ber;
    }
    
    MD_DEBUG("CSQ update:%d,%d\r\n", rssi, ber);

    return MDE_OK;
}