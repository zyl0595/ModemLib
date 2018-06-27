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
    eMDSockState state; /*Socket����״̬(������������ʱ����ֶβ������)*/
    sMDIPv4Addr ip;     /*�Զ�IP��ַ*/
    uint16_t    port;   /*�Զ˶˿ں�*/
}sMDSocket;

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


/*�洢ģ�����״̬��Ϣ*/
typedef struct{
    uint16_t type;      /*ģ���ͺ�*/
    sMDCSQ   csq;       /*�ź�����*/
    uint8_t IMEI[20];
    uint8_t IMSI[20];

}sMDModemInfo;


typedef struct{
    eMDStates   state;
    sMDModemInfo mdInfo;                    /*ģ����Ϣ�����͡�������Ϣ���ź�����������״̬��IMEI��IMSI*/
    sMDIPv4Addr localAddr;                  /*������������֮���õı���IP��ַ*/
    sMDSocket   sockets[MD_MAX_SOCK_NUM];   /*Socket�б�*/
    uint8_t     maxSockNum;                 /*����ģ�鲻֧ͬ�ֵ����Socket��������ͬ*/
}sMDModem;

#endif //__MD_TYPE_H

