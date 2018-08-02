/* 
* 文件名称：MD.h
* 摘    要：
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#ifndef __MD_H
#define __MD_H

#ifdef __cplusplus 
extern "C" { 
#endif

int MD_Init(unsigned char *pBuf, unsigned int bufSize);
int MD_connect(void);
int MD_SendDataB(unsigned char fd, unsigned char *pBuf, int len);

#ifdef __cplusplus 
}
#endif

#endif //__MD_H
