#include <stdio.h> 
#include <string.h>
#include "MD.h"

unsigned char buf[] = "a string ,write %d %sto com";
unsigned char rcvBuf[10];

#define RCV_SIZE (sizeof(rcvBuf)-1)

int main(int argc, char* argv[])
{
    if(MD_Init()){
        MD_test();
    }else{
        printf("Init failed!\r\n");
    }

    printf("press anykey to the end!\r\n");
	getchar();
	return 0;
}

