/* 
* �ļ����ƣ�MD.c
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

#include <string.h>
#include "MDType.h"
#include "MDConfig.h"
#include "MDPort.h"
#include "MDAtProc.h"
#include "MDTools.h"
#include "ucos_ii.h"

//֧�ֵ�����ģ��ͷ�ļ���ӵ���
#include "MD_CLM920.h"

/*************************** ˽�ж��� ******************************/
typedef struct{
    uint8_t type;                       /*ģ�����ͱ�ţ�Ψһ*/
    const uint8_t *pInfo;               /*ģ������������Ϣ�������ַ���*/
    const sMDFucTable *pFunTable;       /*��ģ����и�������ĺ�����*/
    const sMDURCHdlItem *urcHdlTable;   /*���������봦���*/
}sMDModemTypeInfo;

typedef enum{
    MD_STATE_BEGIN,   /*������ʼ״̬*/
    MD_STATE_NET_REG, /*��ע�����磬socket����*/
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

/*************************** ˽�б��� ******************************/
/*TCP��UDP���ݷ���ȷ�϶���*/
static void *s_SndCfm[5];
OS_EVENT *s_SndCfmQ = NULL;
static int s_sndCfmVal = 0;

/*TCP��UDP���ݷ���ȷ�϶���*/
static void *s_ConnCfm[5];
OS_EVENT *s_ConnCfmQ = NULL;
static int s_sndConnVal = 0;


/*************************** ȫ�ֱ��� ******************************/
sMDModem g_Md; /*�洢��¼ģ��״̬����Դ*/
static void *s_ProcMsg[MD_PROC_MSG_Q_SIZE];/*ģ��������Ϣ���д洢*/
OS_EVENT *g_MsgQ = NULL;    /*ģ��������Ϣ���У��û���ģ��Ĳ������ȷ�װ��������Ϣ��Ȼ���͵�����Ϣ���У���
                              ģ����������̰�������*/


/*
���еȴ�״̬������ѭ�����(���ڴ��ڽ�����ɴ��������¼���)�����ջ������������ݽ��ջ���������
���������Ϣ
��������״̬��������ģ�鷢��ATָ����ȴ�ֱ�����յ�ģ��ظ����ݵĹ���
*/
uint8_t g_AtIsIdle = TRUE; //AT����״̬�л���־��TRUE:���еȴ�״̬  FALSE:��������״̬


/*************************** ����ʵ�� ******************************/

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

    /*���ó�ʼ״̬*/
    pMd->state = MDS_BEGIN;

    /*���Socket������Ϣ*/
    for(i=0;i<MD_MAX_SOCK_NUM;i++){
        MD_SockClean(&(pMd->sockets[i]));
    }

    /*���ģ���ͺ���Ϣ�ͺ���������*/
    pMd->pTypeName = NULL;
    pMd->pFunTable = NULL;
}


/*
* �������ܣ�ʶ��ģ�����Ͳ���ȡģ����Ϣ������������
*
* ����˵����
*     [out]pMd :   ����ģ��������Ϣ�Ͳ���������
*
* ����ֵ���Ƿ�ɹ���ȡ��ģ����Ϣ��������pMdģ���������
*/ 
static eMDErrCode MD_ModemDetect(sMDModem *pMd)
{
    eMDErrCode err;
    sMDAtCmdRsp rsp;
    int i;

    err = MD_ATCmdSnd("ATI\r\n", 2, &rsp);
    if((MDE_OK == err) && (rsp.isPositive)){
        /*����ƥ��ģ����Ϣ*/
        for(i = 0; i < TABLE_SIZE(s_modelInfoTable); i++){
            if(strstr(rsp.buf, s_modelInfoTable[i].pInfo)){
                pMd->type = s_modelInfoTable[i].type;//ģ���ͺ���
                pMd->pTypeName = s_modelInfoTable[i].pInfo;//�ͺ�����
                pMd->pFunTable = s_modelInfoTable[i].pFunTable;//ģ�����������
                pMd->pURCHdlTable = s_modelInfoTable[i].urcHdlTable;//URC����
                MD_DEBUG("Detect model type:%s\r\n", pMd->pTypeName);

                /*ģ�������������*/
                if(NULL == pMd->pFunTable){
                    MD_DEBUG("model fun table empty error!\r\n");
                    return MDE_ERROR;
                }

                /*URC���������������*/
                if(NULL == pMd->pURCHdlTable){
                    MD_DEBUG("model URC handle table empty error!\r\n");
                    return MDE_ERROR;
                }
                
                /*ģ���������������*/
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


//��������
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

//����Socket����
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
* �������ܣ�����һ��Socket���ӣ�����ʽ��
*
* ����˵����
*     [in]pSock :   Ҫ�����������ӵ�socket��Ϣ�� 
*
* ����ֵ���ɹ�����1��ʧ�ܷ���0
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

    /*�ȴ����Ӳ������*/
    connResult = OSQPend(s_ConnCfmQ, 0, &err);
    if(*connResult){
        pSock->state = SOCK_OPENED;
        return 1;
    }else{
        return 0;
    }
}


/*
* �������ܣ�ͨ��socket����TCP/UDP���ݣ�����ʽ��
*
* ����˵����
*     [in]fd :   ���ڷ������ݵ�socket��ţ� 
*     [in]pBuf : ָ��Ҫ�������ݴ洢����
*     [in]len :  Ҫ���͵����ݳ��ȣ�
*
* ����ֵ���ɹ����ط��͵��ܳ��ȣ�ʧ�ܷ���0
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

    /*�ȴ����ݷ������*/
    sndResult = OSQPend(s_SndCfmQ, 0, &err);
    if(*sndResult){
        return len;
    }else{
        return 0;
    }
}

//���Ժ�������������ָ��tcp������
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

    /*������Ϣ����ִ����Ӧ�Ĵ���*/
    switch(pMsg->type){
        case MSG_UART_RCV:                                                  //���յ�ģ������
            {
                //MD_DEBUG("Proc uart rcv msg!\r\n");
                do{
                    ret = MD_ATURCProc(g_Md.pURCHdlTable, 5000);
                }while(MDE_BUFEMPTY != ret);
            }
            break;
        case MSG_CONNECT_REG:                                               //����Socket����
            {
                MD_DEBUG("Proc socket connect msg!\r\n");
                if((NULL != g_Md.pFunTable) && (NULL != g_Md.pFunTable->SockConnect)){
                    ret = g_Md.pFunTable->SockConnect(&((sMDConnectReq *)pMsg)->sock);
                    if(MDE_OK == ret){
                        s_sndConnVal = 1;//���ӳɹ�
                        ((sMDConnectReq *)pMsg)->sock.state = SOCK_OPENED;
                    }else{
                        s_sndConnVal = 0;//����ʧ��
                    }
                }else{
                    MD_DEBUG("Fuc err:%s missing SockConnect()\r\n", g_Md.pTypeName);
                    s_sndConnVal = 0;//����ʧ��
                }
                OSQPost(s_ConnCfmQ, &s_sndConnVal);
            }
            break;
        case MSG_SEND_REQ:                                                  //������������
            {
                pSockSndReq = (sMDSockSndReq *)pMsg;

                if(SOCK_OPENED == g_Md.sockets[pSockSndReq->sock].state){

                    /*ִ�����ݷ���*/
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
                    
                    s_sndCfmVal = 1;//���ͳɹ�
                }else{
                    s_sndCfmVal = 0;
                }

                /*��֪��Ϣ�����߷��Ͳ������*/
                OSQPost(s_SndCfmQ, &s_sndCfmVal);
            }
            break;
        default:                                                            //δ֪����
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
            /*ִ��һЩ״̬���������·״̬��*/
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
            //������Դ��ʼ��������ע�ᡢSocket���ӳ�ʼ

            /*ģ���ͺ�ʶ��*/
            if(MDE_OK != MD_ModemDetect(&g_Md)){
                MD_DEBUG("Detect MD model failed!\r\n");
                break;
            }

            /*ģ���ʼ��*/
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
            //�ȴ��¼�
            //�¼����ͣ��������ӡ��������ݡ���������
            if(MDE_ONEXIT == MD_Run()){//��û�г��ִ��󲻷���
                g_Md.state = MDS_FAILED;
            }
            break;
        case MDS_FAILED:
            //�ر����ӡ�������Դ������״̬���������
            MD_DEBUG("Modem err occour!\r\n");

            g_Md.state = MDS_BEGIN;
            break;
        default:
            //��Ӧִ�е��⡣
            g_Md.state = MDS_BEGIN;
            break;
        }

        MD_Delay(10);
    }while(1);

    MD_DEBUG("*************MD_ProcTask end*************\r\n");//��Ӧ��ִ�е���
    while(1);
}


/*
* ʶ��ģ�����ͣ���ʼ��ģ���������� 
*/ 
int MD_Init(unsigned char *pBuf, unsigned int bufSize)
{
    /*������Ϣ����*/
    if(NULL == g_MsgQ){//ģ����������������
        g_MsgQ = OSQCreate(&s_ProcMsg[0], MD_PROC_MSG_Q_SIZE);
        if(NULL == g_MsgQ){
            MD_DEBUG("Message quere create failed!\r\n");
            return 0;
        }
    }

    if(NULL == s_SndCfmQ){//TCP/IP���ݷ���ȷ������
        s_SndCfmQ = OSQCreate(&s_SndCfm[0], (sizeof(s_SndCfm)/sizeof(s_SndCfm[0])));
        if(NULL == s_SndCfmQ){
            MD_DEBUG("Message snd confim quere create failed!\r\n");
            return 0;
        }
    }

    if(NULL == s_ConnCfmQ){//Socket����ȷ�Ϸ���ȷ������
        s_ConnCfmQ = OSQCreate(&s_ConnCfm[0], (sizeof(s_ConnCfm)/sizeof(s_ConnCfm[0])));
        if(NULL == s_ConnCfmQ){
            MD_DEBUG("Message snd confim quere create failed!\r\n");
            return 0;
        }
    }

    /*����ģ���ͨ�Ŵ��ڣ���ʼ�����ݽ���*/
    if(!MD_LowLayInit()){
        MD_DEBUG("MD_LowLayInit failed!\r\n");
        return 0;
    }

    /*�������ƽ���*/
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



