#include <Windows.h>
#include <process.h>
#include <stdio.h>

#include "os_cpu.h"
#include "ucos_ii.h"
#define log printf

typedef unsigned (__stdcall * _StartAddress) (void *);

static HANDLE s_thread[0xff];
static DWORD s_threadId[0xff];

static int s_iMaxPrio=0;

void OSStart(void){}
void OSInit(){}
void OSStatInit(){}
void OSCtxSw(void){}
void OSTickISR(void){}
INT8U OSTaskStkChk(INT8U prio, OS_STK_DATA *p_stk_data){return 1;}

void Set_EnableAuthRxLcp(unsigned char c)
{
	c=c;
}

INT8U OSTaskCreateExt(void (*task)(void *p_arg),void *p_arg,OS_STK *ptos,INT8U prio,
					  INT16U id,OS_STK *pbos,INT32U stk_size,void *pext,INT16U opt)
{
	HANDLE hThread = NULL;

	if(prio > s_iMaxPrio)
		s_iMaxPrio = prio;

	s_thread[prio] = hThread = CreateThread(NULL,stk_size,(LPTHREAD_START_ROUTINE)task,p_arg,0,&s_threadId[prio]);

	//log("create thread %d:%d\r\n",prio,hThread);
	return 0;
}

INT32U   OSTimeGet(void){
	return (GetTickCount()/10);
}

void OSTimeDly(INT16U tick){
	Sleep(tick*10);
}

OS_EVENT *OSSemCreate(INT16U cnt){
	return (OS_EVENT*)CreateSemaphore(NULL,cnt,0xffff,NULL);
}

void OSSemPend(OS_EVENT *pevent, INT16U timeout, INT8U *err){
	DWORD dwWaitResult;

	if(timeout==0){
		dwWaitResult = WaitForSingleObject((HANDLE)pevent, INFINITE);
	}
	else{
		timeout *= 10;
		dwWaitResult = WaitForSingleObject((HANDLE)pevent, timeout);
	}

	if(err)
		*err = 0;

	//log("win_sem_pend:h=%d time=%d ret=%d\r\n",event,timeout,dwWaitResult);

	if(dwWaitResult == WAIT_TIMEOUT)
		return;
	return ;
}

INT8U OSSemPost(OS_EVENT *pevent){
	if( ReleaseSemaphore((HANDLE)pevent, 1, NULL)==TRUE)
		return 1;
	else
		return 0;
}

OS_EVENT *OSSemDel(OS_EVENT *pevent, INT8U opt, INT8U *err){
	CloseHandle((HANDLE)pevent);
	if(err)
		*err = 0;
	return NULL;
}

int thread_new(void (* function)(void *arg), void *arg, void *stk_base, int stk_size,int prio){
	HANDLE hThread = NULL;

	if(prio > s_iMaxPrio)
		s_iMaxPrio = prio;

	s_thread[prio] = hThread = CreateThread(NULL,stk_size,(LPTHREAD_START_ROUTINE)function,arg,0,&s_threadId[prio]);
	
	//log("create thread %d:%d\r\n",prio,hThread);
	return prio;
}

INT8U OSTaskSuspend(INT8U prio){
	SuspendThread(s_thread[prio]);
	return 0;
}

INT8U OSTaskResume(INT8U prio){
	ResumeThread(s_thread[prio]);
	return 0;
}

INT8U OSTaskDel(INT8U prio){
	if(prio == 0xFFu){
		HANDLE hThread = NULL;
		
		prio = thread_get_cur_prio();

		hThread = s_thread[prio];
		s_thread[prio] = NULL;
		s_threadId[prio] = 0;

		//log("del thread:%d\r\n",prio);
		CloseHandle(hThread);
		ExitThread(0);
	}
	return 0;
}

int thread_get_cur_prio(){
	DWORD hThreadId = GetCurrentThreadId();
	int i;

	for(i=0; i<s_iMaxPrio; i++)
	{
		if(s_threadId[i] == hThreadId)
			return i;
	}

	return 0;
}


typedef struct  
{
	void **start;
	int rd;
	int wd;
	int size;
	OS_EVENT* sem;
	OS_EVENT *lock;
}Q_INFO;

OS_EVENT *OSQCreate(void **start, INT16U size){
	Q_INFO *q_info = (Q_INFO*)malloc(sizeof(Q_INFO));
	
	memset(q_info, 0, sizeof(Q_INFO));
	q_info->start = start;
	q_info->size = size;
	q_info->sem = OSSemCreate(0);
	q_info->lock =OSSemCreate(1);
	return (OS_EVENT*)q_info;
}

void *OSQPend(OS_EVENT *pevent, INT16U timeout, INT8U *err){
	Q_INFO *info = (Q_INFO*)pevent;
	int retErr=0;
	int iRet = 0;

	if(info==NULL || info->start == NULL)
	{
        
		if(err)
			*err = 0;

		return 0;
	}	

	OSSemPend(info->sem, timeout,&retErr);	

	OSSemPend(info->lock, 0, NULL);

	if((0 == retErr) && (info->rd!=info->wd))
	{
		iRet = (int)info->start[info->rd];

		if(info->rd+1 >= info->size)
			info->rd =0 ;
		else
			info->rd++;

		//log("get:%d %d\r\n", (int)pevent, iRet);
	}

	OSSemPost(info->lock);

	if(err){
		*err = retErr;
	}


	return iRet;
}

INT8U OSQPost(OS_EVENT* event, void *pmsg){
	Q_INFO *info = (Q_INFO*)event;
	int putData = (int)pmsg;

	if(info==NULL || info->start==NULL)
		return 0;

	if(info->wd+1%info->size == info->rd)
		return 0;


	OSSemPend(info->lock, 0, NULL);
	//log("put:%d %d\r\n", event, putData);
	
	info->start[info->wd] = pmsg;

	if(info->wd+1 >= info->size)
		info->wd =0 ;
	else
		info->wd++;	

	OSSemPost(info->lock);
	
	OSSemPost(info->sem);

	return 1;
}

INT8U OSQFlush(OS_EVENT* event){
	Q_INFO *info = (Q_INFO*)event;
	unsigned char err;
	if(info==NULL)
		return 0;

	log("q_flush:%d\r\n",event);

	while(1){
		OSQPend(event,1,&err);
		if(!err)
			break;
	}

	return 1;
}

OS_EVENT *OSQDel(OS_EVENT* event, unsigned char *err){
	Q_INFO *info = (Q_INFO*)event;
	if(info==NULL)
		return 0;
	
	log("q_del:%d\r\n", event);

	OSQFlush(event);
	OSSemDel(info->sem,0,NULL);
	OSSemDel(info->lock,0,NULL);

	memset(info, 0, sizeof(Q_INFO));

	OSTimeDly(100);

	free(info);
	return 0;
}

typedef struct  
{
	void *pool;
	int max_queues;
	int queue_size;
	int cur_queues;
	int *queue_flag;
	OS_EVENT *lock;
}MEM_INFO;

OS_MEM *OSMemCreate(void *pool, INT32U max_queues, INT32U queue_size, INT8U* err){
	MEM_INFO *mem_info = (MEM_INFO*)malloc(sizeof(MEM_INFO));

	memset(mem_info, 0, sizeof(MEM_INFO));
	mem_info->cur_queues = max_queues;
	mem_info->max_queues = max_queues;
	mem_info->pool = pool;
	mem_info->queue_size = queue_size;
	mem_info->lock = OSSemCreate(1);
	mem_info->queue_flag = (int*)malloc(sizeof(int)*max_queues);
	
	memset(mem_info->queue_flag, 0, sizeof(int)*max_queues);
	memset(pool, 0, max_queues*queue_size);

	if(err)
		*err = 1;

	return (OS_MEM*)mem_info;
}

void *OSMemGet(OS_MEM *mem, INT8U *err){
	MEM_INFO *mem_info = (MEM_INFO*)mem;
	void *pRet=NULL;
	INT8U retErr = 0;

	if(mem_info==NULL){
		if(err)
			*err = 0;
		return NULL;
	}

	OSSemPend(mem_info->lock, 0, &retErr);

	if(mem_info->cur_queues > 0){
		int index = 0;

		for(index=0; index<mem_info->max_queues;  index++)
		{
			if(!mem_info->queue_flag[index])
				break;
		}

		mem_info->queue_flag[index] = 1;
		pRet = (char*)mem_info->pool+mem_info->queue_size*index;

		log("mem get:%d\r\n",index);
		mem_info->cur_queues--;
		retErr = 1;
	}

	OSSemPost(mem_info->lock);


	if(err)
		*err = retErr;

	return pRet;
}

INT8U OSMemPut(OS_MEM *mem, void *box){
	MEM_INFO *mem_info = (MEM_INFO*)mem;
	int iRet = 0;

	if(mem_info==NULL){		
		return 0;
	}

	OSSemPend(mem_info->lock, 0, NULL);
	if(mem_info->cur_queues < mem_info->max_queues){
		int index = ((char*)box-(char*)mem_info->pool)/mem_info->queue_size;
		
		memset(box, 0, mem_info->queue_size);
		mem_info->queue_flag[index] = 0;

		mem_info->cur_queues++;
		log("mem put:%d\r\n",index);
		iRet = 1;
	}
	OSSemPost(mem_info->lock);

	return iRet;
}

//
static OS_EVENT *s_SysLock = NULL;

void OSSchedLock(){
	if(s_SysLock==NULL)
		s_SysLock = OSSemCreate(1);

	OSSemPend(s_SysLock, 0, NULL);
}

void OSSchedUnlock(){
	OSSemPost(s_SysLock);
}

void LockMainTask(){}

void UnLockMainTask(){}
