#include <Windows.h>
#include "uart.h"

/*移植时需要实现的函数在这个文件中定义*/

void MD_Delay(unsigned short ms)
{
    Sleep(ms);
}

int MD_TtysOpen(void)
{
    return uart_open(TTYS0,SPEED_115200);
}

int MD_ReadByte(unsigned char* pCh)
{
    return uart_read(TTYS0, pCh, 1);
}


int MD_WriteStr(const unsigned char *pSrc, int len)
{
    return uart_write(TTYS0, pSrc, len);
}
