#ifndef __MD_PORT_H
#define __MD_PORT_H

#ifdef __cplusplus 
extern "C" { 
#endif

/************ 移植需要实现的函数 ************/

//延时函数
void MD_Delay(unsigned short ms);

//串口操作
int MD_TtysOpen(void);
int MD_TtysClose(void);
int MD_ReadByte(unsigned char *pCh);
int MD_WriteStr(const unsigned char *pSrc, int len);

#ifdef __cplusplus 
	}
#endif

#endif
