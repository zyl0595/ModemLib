#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus 
extern "C" { 
#endif

#define SPEED_230400    0
#define SPEED_115200    1
#define SPEED_57600     2
#define SPEED_56000     3
#define SPEED_38400     4
#define SPEED_19200     5
#define SPEED_14400     6
#define SPEED_9600      7
#define SPEED_4800      8
#define SPEED_2400      9
#define SPEED_1200      10
#define SPEED_600       11
#define SPEED_300       12
#define SPEED_110       13

enum
{
	TTYS0=0,
	TTYS1,
	TTYS2,
	TTYS3,
	TTYS4,
	TTYS5,
};

int uart_open(int type, int rate);
int uart_write(int type, void *pSrc, int len);
int uart_read(int type, void *pRet, int retLen);
int uart_read_count(int type);
int uart_close(int type);
void uart_empty(int type);
void uart_set_speed(int type, int speed);

#ifdef __cplusplus 
}
#endif

#endif