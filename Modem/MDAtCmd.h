/* 
* 文件名称：MDATCmd.h
* 摘    要：通用AT指令常量字符串声明，该文件可以被需要使用这些通用AT指令的模块直接
*           引用而不必重新定义这些命令
*  
* 作    者：
* 创建日期：2018年6月18日 
*
* 修改历史
* 修改摘要：
* 修改作者：
* 修改时间：
*/

#ifndef __MD_AT_CMD_H
#define __MD_AT_CMD_H

extern const unsigned char cmdATE0[];
extern const unsigned char cmdATE1[];
extern const unsigned char cmdATI[];
extern const unsigned char cmdATCPIN[];
extern const unsigned char cmdATCGDCONT[];
extern const unsigned char cmdATCGATT[];
extern const unsigned char cmdATCGACT[];
extern const unsigned char cmdATCGREG[];

extern const unsigned char urcIPNETOPEN[];

#endif //__MD_AT_CMD_H
 
