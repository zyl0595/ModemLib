/* 
* �ļ����ƣ�MDConfig.h
* ժ    Ҫ��ModemLib�Ĳ��������ļ�������ͨ���������ļ�����غ궨��ֵ���ı��Ĳ����������
*           �����С��ATָ����ȴ�ʱ�䡢ָ��ͼ������������ȵȡ�
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

#if MD_DEBUG_EN
#include <stdio.h>
#define MD_DEBUG printf
#else
#define MD_DEBUG
#endif

/************ ���ò��� **************/
#define MD_RCV_BUF_SIZE                         200     /*����ATָ����Ӧ�Ļ�������С��BYTE)*/
#define MD_ATCMD_ITV                            50      /*ATָ���ָ��䷢��ʱ��������λ��ms��*/
#define MD_ATCMD_ITV_WHENERR                    1000    /*ATָ�����ָ����Ӧ����ʱ�ط�ʱ��������λ��ms��*/
#define MD_MAX_SOCK_NUM                         10      /*���Socket��������ʵ�ʿ�ʹ��Socket��������ģ�鲻ͬ����ͬ��ȡ��������Сֵ*/

#endif /*__MD_CONFIG_H*/ 

