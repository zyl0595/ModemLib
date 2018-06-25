#ifndef __MD_CONFIG_H 
#define __MD_CONFIG_H


/************ 调试相关 **************/
#define MD_DEBUG_EN 1           /*调试输出开关*/

#if MD_DEBUG_EN
#include <stdio.h>
#define MD_DEBUG printf
#else
#define MD_DEBUG
#endif


/************ 配置参数 **************/
#define MD_RCV_BUF_SIZE 200         /*接收AT指令响应的缓存区大小（BYTE)*/
#define MD_ATCMD_ITV 50             /*AT指令表指令间发送时间间隔（单位：ms）*/
#define MD_ATCMD_ITV_WHENERR 1000   /*AT指令表发送指令响应错误时重发时间间隔（单位：ms）*/

#endif /*__MD_CONFIG_H*/ 

