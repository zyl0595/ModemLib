/*
*********************************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                         (c) Copyright 1992-2003, Jean J. Labrosse, Weston, FL
*                                          All Rights Reserved
*
*                                       80x86/80x88 Specific code
*                                          LARGE MEMORY MODEL
*
*                                          Borland C/C++ V4.51
*
* File         : OS_CPU.H
* By           : Jean J. Labrosse
*********************************************************************************************************
*/

#ifndef OS_CPU_H
#define OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                              CPU REGISTERS DEFINE
*                                                 (CPU Specific)
*********************************************************************************************************
*/
#define PCBoffset	0xFF00
#define WDTCON       (PCBoffset+0xE6)
#define TIMEICON     (PCBoffset+0x32)
#define  TIMER0_VECTOR        0x08   //R1122 timer0 interrupt vector
/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   int   INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 16-bit wide                    */
typedef unsigned short OS_CPU_SR;                /* Define size of CPU status register (PSW = 16 bits) */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

/* 
*********************************************************************************************************
*                              Intel 80x86 (Real-Mode, Large Model)
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if 
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to 
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/
#define  OS_CRITICAL_METHOD    2

#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()  asm  CLI                    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   asm  STI                    /* Enable  interrupts                        */
#endif

#if      OS_CRITICAL_METHOD == 2
#define  OS_ENTER_CRITICAL()      asm{\
	                              push ax;\
								  push dx;\
								  mov  dx,WDTCON;\
								  mov  ax,05555h;\
								  out  dx,ax;\
								  mov  ax,0aaaah;\
								  out  dx,ax;\
								  in   ax,dx;\
								  xor ax,ax; \
								  mov dx,TIMEICON;\
								  mov ax,00fh;\
								  out dx,ax;\
								  pop dx;\
								  pop ax;\
};
#define  OS_EXIT_CRITICAL()     asm{\
	                              push ax;\
								  push dx;\
								  mov  dx,WDTCON;\
								  mov  ax,05555h;\
								  out  dx,ax;\
								  mov  ax,0aaaah;\
								  out  dx,ax;\
								  in   ax,dx;\
								  xor ax,ax; \
								  mov dx,TIMEICON;\
								  mov ax,007h;\
								  out dx,ax;\
								  pop dx; \
								  pop ax; \
								  };
#endif

#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts                        */
#endif

/*
*********************************************************************************************************
*                           Intel 80x86 (Real-Mode, Large Model) Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1                       /* Stack grows from HIGH to LOW memory on 80x86  */

#define  uCOS                 0x80                    /* Interrupt vector # used for context switch    */

#define  OS_TASK_SW()         asm  INT   uCOS

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_CPU_EXT  INT8U         OSTickDOSCtr;   /* Counter used to invoke DOS's tick handler every 'n' ticks */

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

extern      INT8U  const  OSTickDOSCtrReload;    /* Reload value for OSTickDOSCtr when it reaches 0    */

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

void       OSTaskStkInit_FPE_x86(OS_STK **pptos, OS_STK **ppbos, INT32U *psize);

#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
OS_CPU_SR  OSCPUSaveSR(void);
void       OSCPURestoreSR(OS_CPU_SR cpu_sr);
#endif

#endif
