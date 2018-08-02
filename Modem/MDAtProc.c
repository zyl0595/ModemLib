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
#include "MDConfig.h"
#include "MDPort.h"
#include "MDType.h"

#if MD_OS_ENABLE
#include "ucos_ii.h"
#endif

/************************** ˽�ж��� *********************************/
#define AT_OK			"OK\r\n"
#define AT_ERROR		"ERROR\r\n"
#define AT_RTR          ">"     //Ready To Recive
#define CRLF			"\r\n"
#define CR				'\r'
#define LF				'\n'
#define RTR             '>'

#define IS_AT_RSP_OK(str) (str == strstr(str, AT_OK))
#define IS_AT_RSP_ERR(str) (str == strstr(str, AT_ERROR))

/************************** ˽�б���  *********************************/

/*���ջ���*/
uint8_t s_ATRcvBuf[MD_RCV_BUF_SIZE];    /*ATָ������ջ���*/
#if MD_OS_ENABLE
OS_EVENT s_ATRcvBufLock;
#endif

/************************** ȫ�ֱ���  *********************************/
sMDIPData g_ipRcvData = {
    FALSE,
    0,
    {0},
    {s_ATRcvBuf, MD_RCV_BUF_SIZE, 0}//Ĭ��ʹ��ATָ������ջ���
};   //TCP/IP���ݽ��մ洢��������г�ʼ����pBuf��ֵʹ��ӵ�������Ĵ洢�ռ䣩




/************************** ����ʵ��  *********************************/

/*
* �������ܣ��������ڽ���ģ�����ݣ����ģ�����TCP���ݣ��Ļ�����������û�
*           û�е��ý������ã���Ĭ��ʹ��ATָ���ʱ���ڽ��ս������ݵĻ���
*           ���û����Сһ��Ƚ�С����ֻҪ����ATָ������û���ͻᱻ���ǣ�
*           ��ǿ�ҽ�������һ�鵥���Ļ������ڽ���TCP/IP���ݣ�
*
*     ע�⣺��Ҫ��ģ���ʼ������֮ǰ�͵��øú������ò���Ч
*
* ����˵����
*     [in]addr :   ָ����ջ���ĵ�ַ�� 
*     [in]size :   ���ջ���Ĵ�С��
*
* ����ֵ�����ͽ�����ɹ�
*/ 
eMDErrCode MD_ATSetATRcvBuf(uint8_t *addr, uint32_t size)
{
    if((NULL != addr) || (0 != size)){
        g_ipRcvData.rcvData.pBuf = addr;
        g_ipRcvData.rcvData.bufSize = size;
        MD_DEBUG("Using custom rcv buf, size:%d\r\n", size);
    }else{
        g_ipRcvData.rcvData.pBuf = s_ATRcvBuf;
        g_ipRcvData.rcvData.bufSize = MD_RCV_BUF_SIZE;
        MD_DEBUG("Using default rcv buf, size:%d\r\n", MD_RCV_BUF_SIZE);
    }
    return MDE_OK;
}

#if 0 //�������������ڶ�ATָ����ջ�����ж�д��������ʱû���� 20180720
eMDErrCode MD_WriteATRcvBuf(uint32_t pos, uint8_t *pSrc, uint32_t len)
{
    uint32_t i;

    for(i=0; i< len; i++,pos++){
        if(pos < sizeof(s_ATRcvBuf)){
            s_ATRcvBuf[pos] = pSrc[i];
        }else{
            return MDE_BUFOVFL;
        }
    }

    return MDE_OK;
}

eMDErrCode MD_ReadATRcvBuf(uint32_t pos, uint8_t *pDes, uint32_t len)
{
    uint32_t i;

    for(i=0; i< len; i++,pos++){
        if(pos < sizeof(s_ATRcvBuf)){
            pDes[i] = s_ATRcvBuf[pos];
        }else{
            return MDE_BUFOVFL;
        }
    }

    return MDE_OK;
}
#endif

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
* �������ܣ��ӽ��ջ����н���ATָ����Ӧ���ݣ�
* ����˵����
*     [out]pRsp  :   ָ��洢AT��Ӧ���ݵĻ���
*     [in] delay :   �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��ms)�� 
*     [in] pTail :   ������Ӧ��β�ַ�������">"��NULLʱĬ��ƥ��"OK"��"ERROR"
*     [in] isCfn :   TRUEʱ��Ҫ���յ�"OK"��"ERROR"�ŷ��أ�FLASHʱֻҪƥ�䵽pTail�ͷ���
*
* ����ֵ����ȡ������ɹ�("OK"��"ERROR")����ʱ�����ջ������
*
* ע�⣺��pStr��ΪNULLʱ��һƥ�䵽pStr�����������������շ���
*/ 
static eMDErrCode MD_GetATRsp(sMDAtCmdRsp *pRsp, uint32_t delay, const uint8_t *pTail, bool isCfn)
{
    uint32_t waitCnt = 0;   //�ȴ�ģ����ӦATָ���ʱ
    uint16_t rcvIndex = 0;
    uint16_t rcvLen;        //һ�ζ�ȡ���ջ���õ������ݳ���
    uint8_t *pCurFind;
    bool isTailFind = FALSE;//��Ӧƥ���ַ����Ƿ����ҵ�
    
    pRsp->buf = s_ATRcvBuf;
    pCurFind = pRsp->buf;

    do{
        if(rcvIndex < MD_RCV_BUF_SIZE-1){
            rcvLen = MD_ReadByte(pRsp->buf+rcvIndex);
        }else{
            return MDE_BUFOVFL;
        }

        if(rcvLen > 0){
            
            if(LF == pRsp->buf[rcvIndex]){
                if(CR == pRsp->buf[rcvIndex-1]){//���յ�������һ��
                    pRsp->buf[rcvIndex+1] = '\0';

                    /*���������ַ���*/
                    if((NULL != pTail) && (FALSE == isTailFind) && strstr(pCurFind, pTail)){
                        isTailFind = TRUE;
                        if(!isCfn)break;//�����˳�������������"OK"����"ERROR"
                    }

                    /*���в��ң�"OK"��"ERROR"*/
                    if(IS_AT_RSP_OK(pCurFind)){
                        pRsp->isPositive = TRUE;
                        break;
                    }else if(IS_AT_RSP_ERR(pCurFind)){
                        pRsp->isPositive = FALSE;
                        break;
                    }
                    pCurFind = &(pRsp->buf[rcvIndex+1]);//�´δ���һ�п�ʼ��������ٶ�
                }
            }else if(RTR == pRsp->buf[rcvIndex]){
                if(0 == strcmp(AT_RTR, pTail)){
                    pRsp->buf[rcvIndex+1] = '\0';
                    isTailFind = TRUE;
                    break;//'>'���ٷ���"OK"����ֱ���˳�����
                }
            }

            rcvIndex ++;
        }else{
            if(delay)MD_Delay(1);
            waitCnt++;
        }
    }while(waitCnt <= delay);

    pRsp->len = rcvIndex;

    if(waitCnt <= delay){

        if(NULL != pTail){
            /*����Ҫƥ�����ⷵ���ַ���������£��������ǰ�յ�"OK"��"ERROR"����������Ҫ����
            �Ƿ����յ�Ŀ���ַ��������ؽ��*/
            if(TRUE == isTailFind){
                return MDE_OK;
            }else{
                return MDE_ERROR;
            }
        }else{
            return MDE_OK;
        }

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
    MD_DEBUG_AT("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*����ָ��ظ�*/
    ret = MD_GetATRsp(pRsp, 1000*delay, NULL, TRUE);
    if(MDE_OK == ret){
        MD_DEBUG_AT("Rsp:%s", pRsp->buf);
    }else if(MDE_TIMEOUT == ret){
        MD_DEBUG("AT cmd rsp timeout!\r\n");
    }else if(MDE_BUFOVFL == ret){
        MD_DEBUG("AT cmd rsp overflow!\r\n");
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
*             ���ݷ���"OK"����"ERROR"���ؽ����
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
eMDErrCode MD_ATCmdSndRich(const uint8_t *pCmd, uint8_t delay, ATCmdRspHdl pRspHdl, void *pArg)
{
    eMDErrCode ret;
    sMDAtCmdRsp rsp;

    /*�������*/
    if(NULL == pCmd)return MDE_PARAM_ERR;

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
        }else{                  //3.��������Ӧ���ݷ�����Ϣ���ؽ��
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
*   [out]pRsp : ATָ����Ӧ���ݽ��ջ��棻
*   [in]pTable: ָ��Ҫ���͵�ATָ��� 
*   [in]pData : ָ��Ҫͨ������ָ��͵����ݣ�
*   [in]len :   Ҫ�������ݵĳ���
*   [in]delay:  �ȴ�ģ�鷵��������Ӧ���ʱ��(��λ��s) 
*   [in]pTail:  ����ƥ���ַ�
* ����ֵ��  �ɹ���ʧ�ܡ�ָ���ʵ������ʱ �����ջ������
*/ 
eMDErrCode MD_ATDataSendRich(sMDAtCmdRsp *pRsp, const uint8_t *pCmd, const uint8_t *pData, uint16_t len, uint8_t delay, const uint8_t *pTail)
{
    eMDErrCode ret;

    /*����AT���������*/
    MD_DEBUG_AT("Snd:%s", pCmd);
    ret = MD_SndATCmd(pCmd);
    if(MDE_OK != ret){
        MD_DEBUG("AT cmd snd failed!\r\n");
        return ret;
    }

    /*�ȴ�ģ��ȷ��*/
    //MD_DEBUG("Rcving '>'...\r\n");
    ret = MD_GetATRsp(pRsp, 1000*delay, AT_RTR, FALSE);
    MD_DEBUG_AT("Rsp:%s\r\n", pRsp->buf);
    if(MDE_OK != ret){
        MD_DEBUG("Snd faild!\r\n");
        return ret;
    }

    /*��������*/
    MD_DEBUG_AT("Snd data len:%d\r\n", len);
    if(len != MD_WriteBuf(pData, len)){
        MD_DEBUG("Data snd failed!\r\n");
        return MDE_TTYSERR;
    }

    /*�ȴ�ȷ��*/
    ret = MD_GetATRsp(pRsp, 1000*delay, pTail, TRUE);
    if(MDE_OK == ret){
        MD_DEBUG_AT("Rsp:%s", pRsp->buf);
        if(TRUE == pRsp->isPositive){//�յ�"OK"
            return MDE_OK;
        }else{//�յ�"ERROR"
            MD_DEBUG("Data snd failed!\r\n");
            return MDE_ERROR;
        }
    }else if(MDE_TIMEOUT == ret){
        MD_DEBUG("Data snd timeout!\r\n");
    }else{
        MD_DEBUG("Data snd failed! %d\r\n", ret);
    }

    return ret;
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
    sMDAtCmdRsp rsp;
    return MD_ATDataSendRich(&rsp, pCmd, pData, len, delay, NULL);
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
        
        ret = MD_ATCmdSndRich(pCmd,
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
*     [in]delay:    �������ȴ�ʱ��(ms)��
* ����ֵ�� �ɹ�����ʱ �����ջ������
*/ 
eMDErrCode MD_AtGetURCMsg(const uint8_t *pUrc, sMDAtCmdRsp *pRsp, uint32_t delay)
{
    eMDErrCode ret;

    ret = MD_GetATRsp(pRsp, delay, pUrc, FALSE);//FALSE: URC ���ݲ���"OK" "ERROR"
    if(MDE_OK == ret){
        MD_DEBUG_AT("Urc:%s", pRsp->buf);
    }
    return ret;
}

/*
* �������ܣ���ATָ����ջ����н���ָ����������
* ����˵����
*     [out]pRsp:    ��ŷ���������Ӧ�� 
*     [out]pRdLen:  ʵ�ʴӽ��ջ����ж�ȡ���������ݳ��ȣ�
*     [in]delay:    �������ȴ�ʱ��(ms)��
* ����ֵ�� �ɹ�����ʱ
*/ 
eMDErrCode MD_ATRcvData(uint8_t *pDes, int *pRdLen, int maxLen, uint32_t delay)
{
    int curReadLen;
    int index = 0;
    uint32_t waitCnt = 0;

    do{
        curReadLen = MD_ReadBuf(pDes+index, maxLen-index);
        if(curReadLen > 0){
            index += curReadLen;
        }else{
            if(delay)MD_Delay(1);
            waitCnt++;
        }
    }while((waitCnt < delay) && (index < maxLen));

    *pRdLen = index;

    if(waitCnt < delay){
        return MDE_OK;
    }else{
        return MDE_TIMEOUT;
    }
}


/*
* �������ܣ������ջ�����ȡ����������(URC)��������URC���ݵ�����Ӧ�Ĵ���
*           �������д���
* ����˵����
*     [in]urcHdlTable:      URC�������� 
*     [in]delay:���յȴ�ʱ�䣬�����յ���������������ʱ�������ݲ����������
*               ���ʵ��ĵȴ���������������URC���ݣ��Խ��յ�"\r\n"Ϊ�ж����ݣ�
*
* ����ֵ�� �ɹ�����ʱ�����ջ������
*/ 
eMDErrCode MD_ATURCProc(const sMDURCHdlItem *urcHdlTable, uint32_t delay)
{
    eMDErrCode ret = MDE_OK;
    uint32_t waitCnt = 0;   //�ȴ�ֱ�����յ�����("\r\n")һ��URC���ݵ�ʱ��
    uint16_t rcvIndex = 0;
    uint16_t rcvLen = 0;

    //AT rcv buf lock

    /*����URC*/
    do{
        if(rcvIndex < MD_RCV_BUF_SIZE-1){
            rcvLen = MD_ReadByte(s_ATRcvBuf+rcvIndex);
        }else{
            ret = MDE_BUFOVFL;
            break;
        }

        if(rcvLen > 0){

            /*���������յ����У�����������ݽ��д��������������*/
            if(LF == s_ATRcvBuf[rcvIndex]){
                if(CR == s_ATRcvBuf[rcvIndex-1]){//���յ�������һ��
                    uint16_t tableIndex;
                    s_ATRcvBuf[rcvIndex+1] = '\0';
                    
                    if(rcvIndex+1 > 2){//�յ������ݲ�ֹ��"\r\n"
                        /*����URC�����б��ҵ����ʵĺ��������URC����*/
                        for(tableIndex = 0; ;tableIndex++){
                            if(NULL != urcHdlTable[tableIndex].pURC){
                                if(strstr(s_ATRcvBuf, urcHdlTable[tableIndex].pURC)){
                                    ret = urcHdlTable[tableIndex].handle(s_ATRcvBuf, urcHdlTable[tableIndex].pArg);
                                    break;
                                }
                                //else{//����ר��
                                //    MD_DEBUG("\r\n--------%d--------+++++\r\n", tableIndex);
                                //    MD_DEBUG("%s", s_ATRcvBuf);
                                //    MD_DEBUG("\r\n---\r\n");
                                //    MD_DEBUG("%s", urcHdlTable[tableIndex].pURC);
                                //    MD_DEBUG("\r\n--------end------+++++\r\n");
                                //}
                            }else{
                                MD_DEBUG("UnRegisted URC len:%d Content:\r\n%s", rcvIndex+1, s_ATRcvBuf);
                                ret = urcHdlTable[tableIndex].handle(s_ATRcvBuf, urcHdlTable[tableIndex].pArg);
                                break;
                            }
                        }
                    }

                    /*������ϣ������˳���*/
                    break;//do{}while()
                }
            }

            rcvIndex ++;
        }else{
            if(rcvIndex > 0){
                MD_Delay(1);//��ǰ�н����ҽ��յ�һ�룬��΢�ȴ�һ��
                waitCnt++;
            }else{
                ret = MDE_BUFEMPTY;
                break;//��ǰ����������û�����ݣ�ֱ���˳�����
            }
        }
    }while(waitCnt < delay);

    if(waitCnt >= delay){
        s_ATRcvBuf[rcvIndex] = '\0';
        MD_DEBUG("Uncompelet URC:%s\r\n", s_ATRcvBuf);
    }

    //AT rcv buf unlock

    return ret;
}