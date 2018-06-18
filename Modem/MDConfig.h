#ifndef __MD_CONFIG_H 
#define __MD_CONFIG_H


/************ start:调试相关 **************/
#define MD_DEBUG_EN 1           /*调试输出开关*/

#if MD_DEBUG_EN
#include <stdio.h>
#define MD_DEBUG printf
#else
#define MD_DEBUG
#endif
/************ end :调试相关 **************/

#define MD_RCV_BUF_SIZE 200     /*接收AT指令响应的缓存区大小（BYTE)*/

#endif /*__MD_CONFIG_H*/ 

