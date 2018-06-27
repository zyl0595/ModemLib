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
#include "MD_CLM920.h"

/*************************** ˽�ж��� ******************************/

/*************************** ˽�б��� ******************************/

/*************************** ȫ�ֱ��� ******************************/
sMDModem g_MdInfo; /*�洢��¼ģ��״̬����Դ*/


/*************************** ����ʵ�� ******************************/

static void MD_SocketInit(sMDSocket *pSock)
{
    if(NULL != pSock){
        pSock->ip.val = 0; //0.0.0.0
        pSock->port   = 0;
        pSock->state  = SOCK_CLOSED;
        pSock->type   = SOCK_STREAM;
    }
}

/*
* ʶ��ģ�����ͣ���ʼ��ģ���������� 
*/ 
int MD_Init(void)
{
    int i;
    int ret = 0;

    /*��ʼ��ģ�������Ϣ*/
    g_MdInfo.state = MDS_BEGIN;

    for(i=0;i<MD_MAX_SOCK_NUM;i++){
        MD_SocketInit(&g_MdInfo.sockets[i]);
    }

    if(MD_TtysOpen()){
        if(MDE_OK == CLM920_Init()){
            MD_DEBUG("CLM920 init succ!\r\n");
            CLM920_SokctInit(&g_MdInfo);
            ret = 1;
        }else{
            MD_DEBUG("CLM920 init failed!\r\n");
        }
    }else{
         MD_DEBUG("ttys open failed!\r\n");
    }
    
    return ret;
}

//��������
eMDErrCode MD_GetHostByName(const char *pName, sMDIPv4Addr *pAddr)
{
    //switch(g_MdInfo.mdInfo.type){
    //    case:
    //}
    return CLM920_GetHostByName(pName, pAddr);
}


eMDErrCode MD_Connect(int s)
{
    return MDE_OK;
}



void MD_test(void)
{
    eMDErrCode ret;
    sMDSockData tcpData;
    const uint8_t str[] = "a test string";
    
    tcpData.pIp = "120.42.46.98";
    tcpData.port = 5700;
    tcpData.pData = str;
    tcpData.len = sizeof(str);
    
    do{
        ret = CLM920_SendIpData(3, &tcpData);
		if(MDE_OK != ret)MD_Delay(5000);
    }while(MDE_OK != ret);
}

