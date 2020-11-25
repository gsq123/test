#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "task.h"
#include "key.h"

int main()
{
	OS_ERR err;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	uart_init(115200);    //串口波特率设置
	LED_Init();         //LED初始化
	KEY_Init();
	start_ucos(&err);
	while(1);
}
