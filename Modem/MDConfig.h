/* 
* 文件名称：MDConfig.h
* 摘    要：ModemLib的参数配置文件，可以通过调整该文件的相关宏定义值来改变库的参数，如接收
*           缓存大小、AT指令交互等待时间、指令发送间隔及调试输出等等。
*  
* 作    者：
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
#define MD_MAX_SOCK_NUM 10          /*最大Socket链接数*/

#endif /*__MD_CONFIG_H*/ 

