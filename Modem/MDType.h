/* 
* �ļ����ƣ�MDType.h
* ժ    Ҫ����ʹ�õ����Զ������Ͷ���
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#ifndef __MD_TYPE_H
#define __MD_TYPE_H

#include <stdint.h> 
#include "MDConfig.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef bool
#define bool uint8_t 
#endif

/*�������Ͷ���*/ 
typedef enum{
    MDE_OK = 0,     /*��ȷ*/
    MDE_TTYSERR,    /*���ڶ�д����*/
    MDE_BUFOVFL,    /*���ջ������*/
    MDE_TIMEOUT,    /*��ʱ*/
    MDE_ALREADYON,  /*�Ѵ�*/
    MDE_ERROR       /*δ�������*/
}eMDErrCode;

/*Socket����״̬*/
typedef enum{
    SOCK_CLOSED = 0,   /*�Ͽ�*/
    SOCK_OPENED,       /*�򿪣�������*/
    SOCK_ERROR,        /*����*/
}eMDSockState;

/*Socket����*/
typedef enum{
    SOCK_STREAM = 1,    /*TCP*/
    SOCK_DGRAM  = 2,    /*UDP*/
    //SOCK_RAW  = 3     /*RAW*/
}eMDSockType;

/*ģ�鷵��ATָ����Ӧ����*/ 
typedef struct{
    uint8_t isPositive;           /*�Ƿ��ǻ�����Ӧ��ָ���"OK"ʱΪTRUE������"ERROR"������ʱΪFALSE*/
    uint8_t buf[MD_RCV_BUF_SIZE]; /*ָ������ݽ��ջ���*/
    uint16_t len;                 /*��Ӧ���ݳ���*/ 
}sMDAtCmdRsp;

/*
* ATָ����Ӧ����ص�����
* �������壺
*   pRsp��ָ���Ӧ����
*/
typedef eMDErrCode (*ATCmdRspHdl)(sMDAtCmdRsp *pRsp, void *pArg); 

/*ATָ���*/
typedef struct{
    const uint8_t *pCmd;  /*ָ��*/
    uint8_t tryTms;       /*������Դ���*/
    uint8_t delay;        /*ָ����Ӧ�ȴ�ʱ��(��λ��s)*/
    ATCmdRspHdl rspHdl;   /*ָ����Ӧ����ص�������û��ʱ��NULL*/
    void *pArg;           /*�ص�����������ƥ�䷵�����ݵ�Ŀ���ַ���*/
}sMDAtCmdItem;

/*ͨ��ATָ���Tcp���ݵ����ݽṹ����*/
typedef struct{
    const uint8_t *pData;   /*Ҫ���͵�����*/
    uint16_t len;           /*���ݳ���*/
    const uint8_t *pIp;     /*Ŀ��Ip��ַ*/
    uint16_t port;          /*Ŀ�Ķ˿ں�*/
}sMDSockData;

typedef union{
    uint32_t val;
    struct{
        uint8_t v1; /*��λ*/
        uint8_t v2; /*�ε�λ*/
        uint8_t v3; /*�θ�λ*/
        uint8_t v4; /*��λ*/
    }sVal;
}sMDIPv4Addr;

/*Socket Control Block ���ƿ�*/
typedef struct{
    eMDSockType type;   /*Socket����*/
    sMDIPv4Addr ip;     /*�Զ�IP��ַ*/
    uint16_t    port;   /*�Զ˶˿ں�*/
}sMDSocket;

/*Socket Control Block ���ƿ�*/
typedef struct{
    eMDSockType type;   /*Socket����*/
    eMDSockState state; /*Socket����״̬(������������ʱ����ֶβ������)*/
    sMDIPv4Addr ip;     /*�Զ�IP��ַ*/
    uint16_t    port;   /*�Զ˶˿ں�*/
}sMDSocketHdl;

/*ģ��״̬*/
typedef enum{
    MDS_BEGIN = 0,  /*��ʼ״̬*/
    MDS_CHECK,      /*״̬��⣺�忨״̬���ź���������ǰ������Ӫ�̡�����ע��״̬������ע��״̬*/
    MDS_READY,      /*׼���������ɽ���TCP��UDP��·���������շ�״̬*/
    MDS_FAILED      /*δ�������״̬*/
}eMDStates;


/*�����ź�����*/
typedef struct{
    uint8_t rssi;   /*�����ź�ǿ��ָʾ*/
    uint8_t ber;    /*ͨ��������*/
}sMDCSQ;

/*������ʽ*/
typedef enum{
    NET_AUTO = 0,
    NET_CDMA,
    NET_HDR,
    NET_GSM,
    NET_WCDMA,
    NET_TD_SCDMA,
    NET_EVDO,

}eMDNetMode;


/*�洢ģ�����״̬��Ϣ*/
typedef struct{
    uint16_t    type;      /*ģ���ͺ�*/
    sMDCSQ      csq;       /*�ź�����*/
    uint8_t     IMEI[20];
    uint8_t     IMSI[20];

}sMDModemInfo;


typedef struct{
    eMDStates       state;
    sMDModemInfo    mdInfo;                     /*ģ����Ϣ�����͡�������Ϣ���ź�����������״̬��IMEI��IMSI*/
    sMDIPv4Addr     localAddr;                  /*������������֮���õı���IP��ַ*/
    sMDSocketHdl    sockets[MD_MAX_SOCK_NUM];   /*Socket�б�*/
    uint8_t         maxSockNum;                 /*����ģ�鲻֧ͬ�ֵ����Socket��������ͬ*/
}sMDModem;




/*ģ����������ӿڶ���*/
typedef eMDErrCode (*DMF_GetModelInfo)(sMDModem *pMd);                                              /*��ȡģ���ͺż�������Ϣ*/
typedef eMDErrCode (*DMF_GetIMEI)(sMDModem *pMd);                                                   /*��ȡģ��IMEI��*/
typedef eMDErrCode (*DMF_CheckSIM)(sMDModem *pMd);                                                  /*���SIM��״̬*/
typedef eMDErrCode (*DMF_GetIMSI)(sMDModem *pMd);                                                   /*��ȡSIM��IMSI�š���Ӫ��ʶ��*/
typedef eMDErrCode (*DMF_GetCSQ)(sMDModem *pMd);                                                    /*��ȡ��ǰ�����ź�������Ϣ*/

typedef eMDErrCode (*DMF_SelectNet)(sMDModem *pMd, eMDNetMode m);                                   /*ѡ��������ʽ*/
typedef eMDErrCode (*DMF_RegToNet)(sMDModem *pMd);                                                  /*ע�ᵽ����*/
typedef eMDErrCode (*DMF_CheckNetState)(sMDModem *pMd);                                             /*�������ע��״̬*/

typedef eMDErrCode (*DMF_DefPDPContext)(sMDModem *pMd);                                             /*����PDP������*/
typedef eMDErrCode (*DMF_ActPDPContext)(sMDModem *pMd);                                             /*����PDP������*/
typedef eMDErrCode (*DMF_SetUsrPwd)(sMDModem *pMd);                                                 /*�����û���������*/
typedef eMDErrCode (*DMF_ChekNetAttch)(sMDModem *pMd);                                              /*���GPRS���總��״̬*/

typedef eMDErrCode (*DMF_SocketInit)(sMDModem *pMd);                                                /*���ò�������ʼ��ģ�鵽���Կ�ʼ����Socket���ӵ�״̬*/
typedef eMDErrCode (*DMF_GetLocalAddr)(sMDModem *pMd);                                              /*��ȡ����IP��ַ*/
typedef eMDErrCode (*DMF_SockConnect)(sMDModem *pMd, uint8_t s, sMDSocket *pS);                     /*����һ��Socket����*/
typedef eMDErrCode (*DMF_SockClose)(sMDModem *pMd, uint8_t s);                                      /*�ر�һ��Socket����*/
typedef eMDErrCode (*DMF_SockGetState)(sMDModem *pMd, uint8_t s, eMDSockState *pS);                 /*��ȡһ��Socket���ӵ�״̬*/
typedef eMDErrCode (*DMF_SockSend)(sMDModem *pMd, uint8_t s, const uint8_t *pData, uint16_t len);   /*ͨ��һ��Socket���ӷ�������*/
typedef eMDErrCode (*DMF_GetHostByName)(sMDModem *pMd, const uint8_t *pName, sMDIPv4Addr *pIp);     /*ͨ��ģ��DNS�����ȡ����������ȡIP��ַ*/

/*ģ����������б�ÿ��ģ�鶼��һ���Լ��б�ģ��ά������ͨ�����ø��б��еĺ���������ж�ģ��Ĳ�����
������б��еĺ���ָ��ΪNULLʱ��Ĭ��ʹ�ö�Ӧ��ͨ�ú�����ģ����в���*/
typedef struct{
    /*ģ�鼰SIM��״̬��ȡ*/
    DMF_GetModelInfo    GetModelInfo;
    DMF_GetIMEI         GetIMEI;
    DMF_CheckSIM        CheckSIM;
    DMF_GetIMSI         GetIMSI;
    DMF_GetCSQ          GetCSQ;

    /*ѡ�����粢ע��*/
    DMF_SelectNet       SelectNet;
    DMF_RegToNet        RegToNet;
    DMF_CheckNetState   CheckNetState;

    /*GPRS�������*/
    DMF_DefPDPContext   DefPDPContext;
    DMF_ActPDPContext   ActPDPContext;
    DMF_SetUsrPwd       SetUsrPwd;
    DMF_ChekNetAttch    ChekNetAttch;

    /*��ǶTCP/IPЭ��ջ����*/
    DMF_SocketInit      SocketInit;
    DMF_GetLocalAddr    GetLocalAddr;
    DMF_SockConnect     SockConnect;
    DMF_SockClose       SockClose;
    DMF_SockSend        SockSend;
    DMF_GetHostByName   GetHostByName;
}sMDFucTable;




#endif //__MD_TYPE_H

