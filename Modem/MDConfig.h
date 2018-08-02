/* 
* 文件名称：MDConfig.h
* 摘    要：ModemLib的参数配置文件，可以通过调整该文件的相关宏定义值来改变库的参数，如接收
*           缓存大小、AT指令交互等待时间、指令发送间隔及调试输出等等。
*
*           注意：修改了该文件内容的任何值之后，都要重新编译整个库！！！
*  
* 作    者：张云龙
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#ifndef __MD_CONFIG_H 
#define __MD_CONFIG_H


/************ 调试相关 **************/
#define MD_DEBUG_EN                             1       /*调试输出开关*/
#define MD_DEBUG_AT_LOG_EN                      1       /*是否输出详细的AT指令交互Log*/

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


/************ 配置参数 **************/
#define MD_OS_ENABLE                            1       /*是否在操作系统多进程环境下使用*/
#define MD_RCV_BUF_SIZE                         1024    /*接收AT指令响应的缓存区大小（BYTE)*/
#define MD_ATCMD_ITV                            50      /*AT指令表指令间发送时间间隔（单位：ms）*/
#define MD_ATCMD_ITV_WHENERR                    1000    /*AT指令表发送指令响应错误时重发时间间隔（单位：ms）*/
#define MD_MAX_SOCK_NUM                         10      /*最大Socket链接数，实际可使用Socket个数根据模块不同而不同，取二者中最小值*/
#define MD_PROC_MSG_Q_SIZE                      50      /*模块事务处理进程消息队列大小*/
#define MD_MAX_RESND_TIMES                      3       /*TCP/UDP数据发送时最大重发次数*/

#endif /*__MD_CONFIG_H*/ 

