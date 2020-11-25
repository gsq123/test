#include "task.h"
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "os.h"
#include "includes.h"
#include <string.h>

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()

void print1_task(void *p_arg);
void print2_task(void *p_arg);
void print3_task(void *p_arg);
void timer1_callback(void *p_tmr,void *p_arg);
void timer2_callback(void *p_tmr,void *p_arg);

//开始任务
#define START_TASK_PRIO 3
#define START_STK_SIZE 128 //任务堆栈大小
CPU_STK START_TASK_STK[START_STK_SIZE];//任务堆栈
OS_TCB start_task_tcp;//任务控制块

#define PRINT1_TASK_PRIO 4
#define PRINT1_STK_SIZE 128 //任务堆栈大小
CPU_STK PRINT1_TASK_STK[PRINT1_STK_SIZE];//任务堆栈
OS_TCB print1_task_tcp;//任务控制块

#define PRINT2_TASK_PRIO 5
#define PRINT2_STK_SIZE 128 //任务堆栈大小
CPU_STK PRINT2_TASK_STK[PRINT2_STK_SIZE];//任务堆栈
OS_TCB print2_task_tcp;//任务控制块

#define PRINT3_TASK_PRIO 6
#define PRINT3_STK_SIZE 128 //任务堆栈大小
CPU_STK PRINT3_TASK_STK[PRINT3_STK_SIZE];//任务堆栈
OS_TCB print3_task_tcp;//任务控制块

OS_TMR timer1;//定时器1控制块
OS_TMR timer2;//定时器2控制块
OS_SEM sem1;
uint8_t share[10] = {0};

void start_ucos(OS_ERR *err)//启动ucos
{
	CPU_SR_ALLOC();
	OSInit(err); //初始化OS
	OS_CRITICAL_ENTER();//进入临界区
	OSTaskCreate(
				(OS_TCB 	*)&start_task_tcp,
				(CPU_CHAR	*)"start task",
				(OS_TASK_PTR )start_task,
				(void       *)0,
				(OS_PRIO	 )START_TASK_PRIO,
				(CPU_STK   * )START_TASK_STK,
				(CPU_STK_SIZE)START_STK_SIZE/10,
				(CPU_STK_SIZE)START_STK_SIZE,
				(OS_MSG_QTY  )0,
				(OS_TICK	 )0,
				(void   	*)0,
				(OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
				(OS_ERR 	*)err				
				);
	OS_CRITICAL_EXIT();	//退出临界区
    OSStart(err);  //开启UCOSIII
}

void start_task(void *p_arg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
    p_arg = p_arg;

    CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);  	//统计任务
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
    //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
    OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
#endif

    OS_CRITICAL_ENTER();	//进入临界区
	//创建信号量1
	OSSemCreate(
				&sem1,
				"sem1",
				1,
				&err
				);
	
    //创建1任务
    OSTaskCreate((OS_TCB 	* )&print1_task_tcp,
                 (CPU_CHAR	* )"print1 task",
                 (OS_TASK_PTR )print1_task,
                 (void		* )0,
                 (OS_PRIO	  )PRINT1_TASK_PRIO,
                 (CPU_STK   * )&PRINT1_TASK_STK[0],
                 (CPU_STK_SIZE)PRINT1_STK_SIZE / 10,
                 (CPU_STK_SIZE)PRINT1_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK	  )0,
                 (void   	* )0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);

    //创建2任务
    OSTaskCreate((OS_TCB 	* )&print2_task_tcp,
                 (CPU_CHAR	* )"print2 task",
                 (OS_TASK_PTR )print2_task,
                 (void		* )0,
                 (OS_PRIO	  )PRINT2_TASK_PRIO,
                 (CPU_STK   * )&PRINT2_TASK_STK[0],
                 (CPU_STK_SIZE)PRINT2_STK_SIZE / 10,
                 (CPU_STK_SIZE)PRINT2_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK	  )0,
                 (void   	* )0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
    //创建3任务
//    OSTaskCreate((OS_TCB 	* )&print3_task_tcp,
//                 (CPU_CHAR	* )"print3 task",
//                 (OS_TASK_PTR )print3_task,
//                 (void		* )0,
//                 (OS_PRIO	  )PRINT3_TASK_PRIO,
//                 (CPU_STK   * )&PRINT3_TASK_STK[0],
//                 (CPU_STK_SIZE)PRINT3_STK_SIZE / 10,
//                 (CPU_STK_SIZE)PRINT3_STK_SIZE,
//                 (OS_MSG_QTY  )0,
//                 (OS_TICK	  )1,
//                 (void   	* )0,
//                 (OS_OPT      )OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
//                 (OS_ERR 	* )&err);
//		printf("打印3任务已创建\r\n");			 
	OSTmrCreate(
				 &timer1,
				 "timer1",
				 10,//20*0.01s
				 0,
				 OS_OPT_TMR_ONE_SHOT,
				 timer1_callback,
				 0,
				 &err	 
				);	
	OSTmrCreate(
				 &timer2,
				 "timer2",
				 10,
				 10,
				 OS_OPT_TMR_PERIODIC,
				 timer2_callback,
				 0,
				 &err	 
				);
	//OSTmrStart(&timer1,&err);				
    OS_TaskSuspend((OS_TCB*)&start_task_tcp, &err);		//挂起开始任务
    OS_CRITICAL_EXIT();	//进入临界区
}



void print1_task(void *p_arg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
	uint8_t task_cnt = 0;
    p_arg = p_arg;
	uint8_t task1_string[]={"TASK1\r\n"};
	while(1)
	{
		printf("\r\n任务1:\r\n");
		OSSemPend(&sem1,300,OS_OPT_PEND_BLOCKING,(CPU_TS*)0,&err);
		memcpy(share,task1_string,sizeof task1_string);
		task_cnt++;
		//delay_ms(200);//发起任务调度
		printf("%s \r\n",share);
		OSSemPost (&sem1,OS_OPT_POST_1,&err);				//发送信号量

		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err); //延时500ms
	}
    //OS_CRITICAL_EXIT();	//进入临界区
}



void print2_task(void *p_arg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
	uint8_t task_cnt = 0;
    p_arg = p_arg;
	uint8_t task2_string[] = {"TASK2\r\n"};

	while(1)
	{
		printf("\r\n任务2:\r\n");
		task_cnt++;
		OSSemPend(&sem1,0,OS_OPT_PEND_BLOCKING,(CPU_TS*)0,&err);
		memcpy(share , task2_string , sizeof task2_string);
		//delay_ms(200);
		printf("%s \r\n",share);
		OSSemPost (&sem1,OS_OPT_POST_1,&err);				//发送信号量
		//OSSched();
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err); //延时500ms
	}
    //OS_CRITICAL_EXIT();	//进入临界区
}
void print3_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	uint8_t task_cnt = 0;
    CPU_SR_ALLOC();
	while(1)
	{
		task_cnt++;
		//printf("打印2任务已执行: %d 次\r\n",task_cnt);
		//OSSched();
	}				 
}
void timer1_callback(void *p_tmr,void *p_arg)
{
	static uint8_t cnt = 0;
	OS_ERR err;
	if(cnt==0)
	{
		OSTmrStart(&timer2,&err);
	}
	cnt++;
	printf("定时器1已运行: %d \r\n",cnt);
}
void timer2_callback(void *p_tmr,void *p_arg)
{
	static uint8_t cnt;
	cnt++;
	printf("定时器2已运行: %d \r\n",cnt);
}

