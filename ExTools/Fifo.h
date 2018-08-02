#ifndef __FIFO_H
#define __FIFO_H

#ifdef __cplusplus 
extern "C" { 
#endif

#define FIFO_PARAM_CHECK_EN     0
#define FIFO_DEBUG_EN           0

#define FIFO_ENTER_CRITICAL
#define FIFO_EXIT_CRITICAL

typedef struct{
    unsigned char *buf;
    unsigned int size;
    unsigned int w;
    unsigned int r;
}FIFO_BUF;

#define isFifoEmpty(pBuf) (pBuf->r == pBuf->w)
#define FifoFlush(pBuf) (pBuf->r = pBuf->w)
int FifoBufInit(FIFO_BUF *pBuf, void *pStore, unsigned int size);
int FifoWriteByte(FIFO_BUF *pBuf, unsigned char ch);
int FifoReadByte(FIFO_BUF *pBuf, unsigned char *ch);
unsigned int FifoGetDataLen(FIFO_BUF *pBuf);
unsigned int FifoGetRestSpace(FIFO_BUF *pBuf);
int FifoReadMultByte(FIFO_BUF *pBuf, unsigned char *pDes, unsigned int len);
int FifoWriteMultByte(FIFO_BUF *pBuf, unsigned char *pSrc, unsigned int len);

#ifdef __cplusplus 
}
#endif

#endif //__FIFO_H

