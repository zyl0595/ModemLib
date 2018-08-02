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
    MDE_BUFEMPTY,   /*��������*/
    MDE_TIMEOUT,    /*��ʱ*/
    MDE_ALREADYON,  /*�Ѵ�*/
    MDE_PARAM_ERR,  /*��������*/
    MDE_ONEXIT,     /*����ִ���˳�*/
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
    MDSOCK_STREAM = 1,    /*TCP*/
    MDSOCK_DGRAM  = 2,    /*UDP*/
    //MDSOCK_RAW  = 3     /*RAW(��֧��)*/
}eMDSockType;

/*ģ�鷵��ATָ����Ӧ����*/ 
typedef struct{
    uint8_t isPositive;           /*�Ƿ��ǻ�����Ӧ��ָ���"OK"ʱΪTRUE������"ERROR"������ʱΪFALSE*/
    uint8_t *buf;                 /*ָ������ݽ��ջ��� �̶�ָ��ȫ������ s_ATRcvBuf[MD_RCV_BUF_SIZE] */
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
}sMDSockData; //------------------------------------------------------------------------�������Ҫȥ��������

typedef union{
    uint32_t val;
    struct{
        uint8_t v1; /*��λ*/
        uint8_t v2; /*�ε�λ*/
        uint8_t v3; /*�θ�λ*/
        uint8_t v4; /*��λ*/
    }sVal;
}sMDIPv4Addr;


/*Socket���ӶԶ˵�ַ*/
typedef struct{
    sMDIPv4Addr IP;     //IP��ַ
    uint16_t    port;   //�˿ں�
}sMDSockPeerAddr;

/*������������ʹ�õĽṹ��*/
typedef struct{
    uint8_t  *pBuf;    //ָ��洢��
    uint32_t bufSize;  //�洢����С
    uint32_t len;      //�洢�������ݳ���
}sMDDataHdl;

/*����TCP/IP���ջ������ݽṹ��*/
typedef struct{
    bool            isNewRcved; //�Ƿ���յ��µ�����
    uint8_t         sockNum;    //Socket���
    sMDSockPeerAddr srcAddr;    //Socket�Զ�IP��ַ�Ͷ˿ں�
    sMDDataHdl      rcvData;    //���ͻ��߽��յ���������Ϣ�����ͺͽ���ǰ�����bufָ����и�ֵ����
}sMDIPData;


/*Socket Control Block ���ƿ�*/
typedef struct{
//#define MD_MAX_IPV4_STR_LEN (4*4+1) /*�ַ�����IPv4���洢���ȣ��� "255.255.255.255"*/
    uint8_t         id;         /*Socket���*/
    eMDSockType     type;       /*Socket����*/
    eMDSockState    state;      /*Socket����״̬*/
    sMDIPv4Addr     serverIp;   /*�Զ�IP��ַ*/
    uint16_t        serverPort; /*�Զ˶˿ں�*/
    uint16_t        localPort;  /*���ض˿ں�*/
}sMDSocket;


/*ģ��״̬*/
typedef enum{
    MDS_BEGIN = 0,  /*��ʼ״̬*/
    //MDS_CHECK,      /*״̬��⣺�忨״̬���ź���������ǰ������Ӫ�̡�����ע��״̬������ע��״̬*/
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
    NET_MODE_AUTO = 0,
    NET_MODE_CDMA,
    NET_MODE_HDR,
    NET_MODE_GSM,
    NET_MODE_WCDMA,
    NET_MODE_TD_SCDMA,
    NET_MODE_EVDO,
}eMDNetMode;

typedef enum{
    NET_REG_UNREG = 0,  /*δע��*/
    NET_REG_HOME,       /*��ע�ᣬ��������*/
    NET_REG_SEARCHING,  /*δע�ᣬ����������Ӫ��*/
    NET_REG_DENIED,     /*δע�ᣬ����ע�ᱻ�ܾ�*/
    NET_REG_UNKNOW,     /*δ֪״̬*/
    NET_REG_ROAM        /*��ע�ᣬ����״̬*/
}eMDNetRegState;



/*ģ����������ӿڶ���*/
/*ʵ��ԭ��
    1.��Щ������ɵĲ�������Ҫ��һ��
    2.�����ں�������ָ���ط���ָ����ط��ɵ����߸��ݺ������ؽ������*/
////typedef eMDErrCode (*DMF_GetModelInfo)(uint8_t *pBuf);                                              /*��ȡģ���ͺ���Ϣ*/
//typedef eMDErrCode (*DMF_GetIMEI)(uint8_t *pBuf);                                                   /*��ȡģ��IMEI��*/
//typedef eMDErrCode (*DMF_CheckSIM)(void);                                                           /*���SIM��״̬*/
//typedef eMDErrCode (*DMF_GetIMSI)(uint8_t *pBuf);                                                   /*��ȡSIM��IMSI�š���Ӫ��ʶ��*/
//typedef eMDErrCode (*DMF_GetCSQ)(sMDCSQ *pCsq);                                                     /*��ȡ��ǰ�����ź�������Ϣ*/
//
//typedef eMDErrCode (*DMF_SelectNet)(eMDNetMode m);                                                  /*����ģ��������ʽ��ǿ��Ϊĳһ������ʽ���Զ�*/
//typedef eMDErrCode (*DMF_RegToNet)(void);                                                           /*����ģ��ע�ᵽ����*/
//typedef eMDErrCode (*DMF_ChekNetReg)(uint8_t *pStat);                                               /*�������ע��״̬*/
////typedef eMDErrCode (*DMF_GetNetOperAndMode)(uint8_t *pBuf);                                         /*��ȡ��ǰ������Ӫ����Ϣ����ǰ����ģʽ*/
//
//typedef eMDErrCode (*DMF_DefPDPCtx)(eMDNetMode m, uint8_t *pCtx);                                   /*����PDP������*/
//typedef eMDErrCode (*DMF_ActPDPCtx)(uint8_t cid);                                                   /*����PDP������*/
//typedef eMDErrCode (*DMF_SetUsrPwd)(uint8_t *pUsr, uint8_t *pPwd);                                  /*�����û���������*/
//typedef eMDErrCode (*DMF_ChekNetAttch)(uint8_t *pStat);                                             /*���GPRS���總��״̬*/

typedef eMDErrCode (*DMF_SocketInit)(void);                                                         /*���ò�������ʼ��ģ�鵽���Կ�ʼ����Socket���ӵ�״̬*/
typedef eMDErrCode (*DMF_GetLocalAddr)(sMDIPv4Addr *pAddr);                                         /*��ȡ����IP��ַ*/
typedef eMDErrCode (*DMF_SockConnect)(sMDSocket *pS);                                               /*����һ��Socket����*/
typedef eMDErrCode (*DMF_SockClose)(sMDSocket *pS);                                                 /*�ر�һ��Socket����*/
//typedef eMDErrCode (*DMF_SockGetState)(sMDSocket *pSocks, uint8_t num);                             /*������num��Socket���ӵ�״̬*/
typedef eMDErrCode (*DMF_SockSend)(sMDSocket *pS, const uint8_t *pData, uint16_t len);              /*ͨ��һ��Socket���ӷ�������*/
//typedef eMDErrCode (*DMF_SockRcv)(uint8_t *pSN, uint8_t *pDes, int *pLen, sMDSockPeerAddr *pSrcIp, uint32_t delay);/*�������ݣ�����ģ����Ҫ����ȥ��������*/
typedef eMDErrCode (*DMF_GetHostByName)(const uint8_t *pName, sMDIPv4Addr *pIp);                    /*ͨ��ģ��DNS�����ȡ����������ȡIP��ַ*/

/*ģ����������б�ÿ��ģ�鶼��һ���Լ��б�ģ��ά������ͨ�����ø��б��еĺ���������ж�ģ��Ĳ�����
������б��еĺ���ָ��ΪNULLʱ��Ĭ��ʹ�ö�Ӧ��ͨ�ú�����ģ����в���*/
/*��ǶTCP/IPЭ��ջ����*/
typedef struct{
    DMF_SocketInit      SocketInit;
    DMF_GetLocalAddr    GetLocalAddr;
    DMF_SockConnect     SockConnect;
    DMF_SockClose       SockClose;
    DMF_SockSend        SockSend;
    DMF_GetHostByName   GetHostByName;
}sMDFucTable;


/*���������봦����ض���*/
typedef eMDErrCode (*MDF_URCHandle)(const uint8_t *pBuf, void *pArg);

typedef struct{
    const uint8_t *pURC;    //�����������ʶ�ַ���
    MDF_URCHandle handle;   //���������봦����
    void          *pArg;    //���������봦��������/��������
}sMDURCHdlItem;


/*�洢ģ�����״̬��Ϣ*/
typedef struct{
    eMDStates           state;
    uint16_t            type;           /*ģ���ͺ�*/
    const uint8_t       *pTypeName;     /*ģ���ͺ�����*/
    const sMDFucTable   *pFunTable;     /*����ģ��ĸ��ຯ���б�*/
    const sMDURCHdlItem *pURCHdlTable;  /*URC���ݽ��մ�������*/
    bool                isSIMReady;     /*SIM��״̬*/
    sMDCSQ              csq;            /*�ź�����*/
    uint8_t             IMEI[20];
    uint8_t             IMSI[20];
    uint8_t             maxSockNum;     /*����ģ�鲻֧ͬ�ֵ����Socket��������ͬ*/
    eMDNetRegState      netRegState;    /*����ע��״̬*/
    sMDIPv4Addr         localAddr;      /*������������֮���õı���IP��ַ*/
    sMDSocket           sockets[MD_MAX_SOCK_NUM];   /*Socket�б��ڸ������е�λ��������Socket�ı��*/
}sMDModem;

/*ģ����������̽����յ��ĸ�����Ϣ���ͣ���Щ��Ϣ��Ҫģ������������������*/
typedef enum{
    MSG_UART_RCV,       /*��ģ����յ��µ����ݣ�����һ���źţ�����ģ�����������Ҫȥ��������*/
    MSG_CONNECT_REG,    /*����Socket��������*/
    MSG_DISCONN_REQ,    /*�Ͽ�Socket����*/
    MSG_SEND_REQ,       /*����TCP��UDP��������*/
    MSG_SQC_REQ,        /*�ź�������ѯ����*/
    MSG_DNS_REG,        /*����������������*/
}eMDMsgType;

/*һ������Ϣ*/
typedef struct{
    /*��Ϣ���ͣ�ǿ�ƣ�*/
    eMDMsgType type;    /*��Ϣ����*/

    void *pContent;     /*��Ϣ���ݣ�һ��ָ��һ������������*/
}sMDMsg;

/*����������Ϣ������*/
typedef struct{
    /*��Ϣ���ͣ�ǿ�ƣ�*/
    eMDMsgType type;

    sMDSocket sock; /*Ҫִ�����ӵ�socket��Ϣ*/
}sMDConnectReq;

/*���ݷ���������Ϣ������*/
typedef struct{
    /*��Ϣ���ͣ�ǿ�ƣ�*/
    eMDMsgType type;

    uint8_t sock;   /*Socket���*/
    uint8_t *pBuf;  /*ָ��Ҫ����������ʼ��ַ*/
    uint32_t len;   /*�������ݳ���*/
}sMDSockSndReq;

#endif //__MD_TYPE_H

