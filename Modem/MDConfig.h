/* 
* �ļ����ƣ�MDConfig.h
* ժ    Ҫ��ModemLib�Ĳ��������ļ�������ͨ���������ļ�����غ궨��ֵ���ı��Ĳ����������
*           �����С��ATָ����ȴ�ʱ�䡢ָ��ͼ������������ȵȡ�
*
*           ע�⣺�޸��˸��ļ����ݵ��κ�ֵ֮�󣬶�Ҫ���±��������⣡����
*  
* ��    �ߣ�������
* �������ڣ�2018��6��18�� 
*
* �޸���ʷ
* �޸�ժҪ��
* �޸����ߣ�
* �޸�ʱ�䣺
*/

#ifndef __MD_CONFIG_H 
#define __MD_CONFIG_H


/************ ������� **************/
#define MD_DEBUG_EN                             1       /*�����������*/
#define MD_DEBUG_AT_LOG_EN                      1       /*�Ƿ������ϸ��ATָ���Log*/

#if MD_DEBUG_EN

    #include <stdio.h>
    #define MD_DEBUG printf

    #if MD_DEBUG_AT_LOG_EN
    #define MD_DEBUG_AT printf
    #else
    #define MD_DEBUG_AT
    #endif

#else

    #define MD_DEBUG
    #define MD_DEBUG_AT

#endif


/************ ���ò��� **************/
#define MD_OS_ENABLE                            1       /*�Ƿ��ڲ���ϵͳ����̻�����ʹ��*/
#define MD_RCV_BUF_SIZE                         1024    /*����ATָ����Ӧ�Ļ�������С��BYTE)*/
#define MD_ATCMD_ITV                            50      /*ATָ���ָ��䷢��ʱ��������λ��ms��*/
#define MD_ATCMD_ITV_WHENERR                    1000    /*ATָ�����ָ����Ӧ����ʱ�ط�ʱ��������λ��ms��*/
#define MD_MAX_SOCK_NUM                         10      /*���Socket��������ʵ�ʿ�ʹ��Socket��������ģ�鲻ͬ����ͬ��ȡ��������Сֵ*/
#define MD_PROC_MSG_Q_SIZE                      50      /*ģ�������������Ϣ���д�С*/
#define MD_MAX_RESND_TIMES                      3       /*TCP/UDP���ݷ���ʱ����ط�����*/

#endif /*__MD_CONFIG_H*/ 

