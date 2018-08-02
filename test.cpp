#include <Windows.h>
#include <stdio.h> 
#include <string.h>
#include "MD.h"

unsigned char buf[] = "a string ,write %d %sto com";

int getInput(char *pBuf, int maxLen)
{
    int index = 0;
    char ch;

    do{
        ch = getchar();
        pBuf[index] = ch;
        index++;
    }while((ch != '\n') && (index < maxLen));
    pBuf[index] = '\0';
    return index;
}

unsigned char rcvBuf[2048];
int rcvLen;
static unsigned char testString[] = "这是一个测试字符串，用来测试上行数据发送。";

int main(int argc, char* argv[])
{
    if(!MD_Init(rcvBuf, sizeof(rcvBuf))){
       printf("Init failed!\r\n");
       goto THE_END;
    }
 
    if(0 == MD_connect()){
        printf("Connect failed!\r\n");
        goto THE_END;
    }

    do{
        if(!MD_SendDataB(0, testString, sizeof(testString))){
            printf("Send failed!\r\n");
        }
        Sleep(5000);
    }while(1);

THE_END:
    printf("press any key to the end!\r\n");
    getchar();

	return 0;
}

