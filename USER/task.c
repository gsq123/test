#include "task.h"
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "os.h"
#include "includes.h"
#include <string.h>

//UCOSIII���������ȼ��û�������ʹ�ã�ALIENTEK
//����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()

void print1_task(void *p_arg);
void print2_task(void *p_arg);
void print3_task(void *p_arg);
void timer1_callback(void *p_tmr,void *p_arg);
void timer2_callback(void *p_tmr,void *p_arg);

//��ʼ����
#define START_TASK_PRIO 3
#define START_STK_SIZE 128 //�����ջ��С
CPU_STK START_TASK_STK[START_STK_SIZE];//�����ջ
OS_TCB start_task_tcp;//������ƿ�

#define PRINT1_TASK_PRIO 4
#define PRINT1_STK_SIZE 128 //�����ջ��С
CPU_STK PRINT1_TASK_STK[PRINT1_STK_SIZE];//�����ջ
OS_TCB print1_task_tcp;//������ƿ�

#define PRINT2_TASK_PRIO 5
#define PRINT2_STK_SIZE 128 //�����ջ��С
CPU_STK PRINT2_TASK_STK[PRINT2_STK_SIZE];//�����ջ
OS_TCB print2_task_tcp;//������ƿ�

#define PRINT3_TASK_PRIO 6
#define PRINT3_STK_SIZE 128 //�����ջ��С
CPU_STK PRINT3_TASK_STK[PRINT3_STK_SIZE];//�����ջ
OS_TCB print3_task_tcp;//������ƿ�

OS_TMR timer1;//��ʱ��1���ƿ�
OS_TMR timer2;//��ʱ��2���ƿ�
OS_SEM sem1;
uint8_t share[10] = {0};

void start_ucos(OS_ERR *err)//����ucos
{
	CPU_SR_ALLOC();
	OSInit(err); //��ʼ��OS
	OS_CRITICAL_ENTER();//�����ٽ���
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
	OS_CRITICAL_EXIT();	//�˳��ٽ���
    OSStart(err);  //����UCOSIII
}

void start_task(void *p_arg)
{
    OS_ERR err;
    CPU_SR_ALLOC();
    p_arg = p_arg;

    CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);  	//ͳ������
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
    //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
    OSSchedRoundRobinCfg(DEF_ENABLED, 1, &err);
#endif

    OS_CRITICAL_ENTER();	//�����ٽ���
	//�����ź���1
	OSSemCreate(
				&sem1,
				"sem1",
				1,
				&err
				);
	
    //����1����
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

    //����2����
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
    //����3����
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
//		printf("��ӡ3�����Ѵ���\r\n");			 
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
    OS_TaskSuspend((OS_TCB*)&start_task_tcp, &err);		//����ʼ����
    OS_CRITICAL_EXIT();	//�����ٽ���
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
		printf("\r\n����1:\r\n");
		OSSemPend(&sem1,300,OS_OPT_PEND_BLOCKING,(CPU_TS*)0,&err);
		memcpy(share,task1_string,sizeof task1_string);
		task_cnt++;
		//delay_ms(200);//�����������
		printf("%s \r\n",share);
		OSSemPost (&sem1,OS_OPT_POST_1,&err);				//�����ź���

		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err); //��ʱ500ms
	}
    //OS_CRITICAL_EXIT();	//�����ٽ���
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
		printf("\r\n����2:\r\n");
		task_cnt++;
		OSSemPend(&sem1,0,OS_OPT_PEND_BLOCKING,(CPU_TS*)0,&err);
		memcpy(share , task2_string , sizeof task2_string);
		//delay_ms(200);
		printf("%s \r\n",share);
		OSSemPost (&sem1,OS_OPT_POST_1,&err);				//�����ź���
		//OSSched();
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err); //��ʱ500ms
	}
    //OS_CRITICAL_EXIT();	//�����ٽ���
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
		//printf("��ӡ2������ִ��: %d ��\r\n",task_cnt);
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
	printf("��ʱ��1������: %d \r\n",cnt);
}
void timer2_callback(void *p_tmr,void *p_arg)
{
	static uint8_t cnt;
	cnt++;
	printf("��ʱ��2������: %d \r\n",cnt);
}

