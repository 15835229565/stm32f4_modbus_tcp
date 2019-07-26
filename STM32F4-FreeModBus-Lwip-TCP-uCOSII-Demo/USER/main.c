/**********************************************************************************************************
** �ļ���		:main.c
** ����			:maxlicheng<licheng.chn@outlook.com>
** ����github	:https://github.com/maxlicheng
** ���߲���		:https://www.maxlicheng.com/	
** ��������		:2018-02-18
** ����			:stm32f4 modbus ������
************************************************************************************************************/
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "lcd.h"
#include "sram.h"
#include "malloc.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwipopts.h"
#include "mb.h"
#include "lsens.h"
#include "adc.h"
#include "dac.h"
#include "beep.h"

//eMBPoll����
#define eMBPoll_TASK_PRIO 		7				//�����������ȼ� 
#define eMBPoll_STK_SIZE		128				//�����ջ��С
OS_STK eMBPoll_TASK_STK[eMBPoll_STK_SIZE]; 		//�����ջ
void eMBPoll_task(void *pdata);  				//������

//KEY_Poll����
#define KEY_POLL_TASK_PRIO       8   			//�����������ȼ� 
#define KEY_POLL_STK_SIZE  		 64  			//���������ջ��С
OS_STK  KEY_POLL_TASK_STK[KEY_POLL_STK_SIZE];   //�����ջ	
void    key_poll_task(void *pdata);             //������    

//LED_Poll����
#define LED_POLL_TASK_PRIO       9   			//�����������ȼ� 
#define LED_POLL_STK_SIZE  		 64  			//���������ջ��С
OS_STK  LED_POLL_TASK_STK[LED_POLL_STK_SIZE];   //�����ջ	
void    led_poll_task(void *pdata);             //������


//��LCD����ʾ��ַ��Ϣ����
#define DISPLAY_TASK_PRIO	10					//�������ȼ�
#define DISPLAY_STK_SIZE	128					//�����ջ��С
OS_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];		//�����ջ
void display_task(void *pdata);					//������

//START����
#define START_TASK_PRIO		11					//�������ȼ�
#define START_STK_SIZE		128					//�����ջ��С
OS_STK START_TASK_STK[START_STK_SIZE];			//�����ջ
void start_task(void *pdata); 					//������

//ADC_Poll����
#define ADC_POLL_TASK_PRIO       12   			//�����������ȼ� 
#define ADC_POLL_STK_SIZE  		 64  			//���������ջ��С
OS_STK  ADC_POLL_TASK_STK[ADC_POLL_STK_SIZE];   //�����ջ	
void    adc_poll_task(void *pdata);             //������

//DAC_Poll����
#define DAC_POLL_TASK_PRIO       13   			//�����������ȼ� 
#define DAC_POLL_STK_SIZE  		 64  			//���������ջ��С
OS_STK  DAC_POLL_TASK_STK[DAC_POLL_STK_SIZE];   //�����ջ	
void    dac_poll_task(void *pdata);             //������

//��LCD����ʾ��ַ��Ϣ
//mode:1 ��ʾDHCP��ȡ���ĵ�ַ
//	  ���� ��ʾ��̬��ַ
void show_address(u8 mode)
{
	u8 buf[30];
	POINT_COLOR = RED; 		//��ɫ����
	if(mode==2)
	{
		sprintf((char*)buf,"DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		LCD_ShowString(30,130,210,16,16,buf); 
		sprintf((char*)buf,"DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		LCD_ShowString(30,150,210,16,16,buf); 
		sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ
		LCD_ShowString(30,170,210,16,16,buf); 
//		LCD_ShowString(30,230,210,16,16,"Port:8088!"); 
	}
	else 
	{
		sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		LCD_ShowString(30,130,210,16,16,buf); 
		sprintf((char*)buf,"Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		LCD_ShowString(30,150,210,16,16,buf); 
		sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);		//��ӡ���������ַ
		LCD_ShowString(30,170,210,16,16,buf); 
//		LCD_ShowString(30,230,210,16,16,"Port:8088!"); 
	}	
}

int main(void)
{
	delay_init(168);       	//��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	uart_init(115200);   	//���ڲ���������
//	usmart_dev.init(84);	//��ʼ��USMART
	LED_Init(); 			//LED��ʼ��
	KEY_Init();  			//������ʼ��
	LCD_Init();  			//LCD��ʼ��
	Adc_Init();
	Lsens_Init();
	Dac1_Init();
	BEEP_Init();
	FSMC_SRAM_Init(); 		//SRAM��ʼ��
	
	mymem_init(SRAMIN);  	//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);  	//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM); 	//��ʼ��CCM�ڴ��
	
	POINT_COLOR = RED; 		//��ɫ����
	LCD_ShowString(30,10,200,20,16,"Explorer STM32F4");
	LCD_ShowString(30,30,200,20,16,"FreeModBus-1.5 Test");
	LCD_ShowString(30,50,200,20,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,70,200,20,16,"2018/2/22");
	POINT_COLOR = BLUE; //��ɫ����

	OSInit(); 					//UCOS��ʼ��
	while(lwip_comm_init()) 	//lwip��ʼ��
	{
		LCD_ShowString(30,90,200,20,16,"Lwip Init failed!"); 	//lwip��ʼ��ʧ��
		delay_ms(500);
		LCD_Fill(30,90,230,150,WHITE);
		delay_ms(500);
	}
	LCD_ShowString(30,90,200,20,16,"Lwip Init Success!"); 		//lwip��ʼ���ɹ�
//	while(tcp_server_init()) 									//��ʼ��tcp_server(����tcp_server�߳�)
//	{
//		LCD_ShowString(30,150,200,20,16,"TCP Server failed!!"); //tcp����������ʧ��
//		delay_ms(500);
//		LCD_Fill(30,150,230,170,WHITE);
//		delay_ms(500);
//	}
//	LCD_ShowString(30,150,200,20,16,"TCP Server Success!"); 	//tcp�����������ɹ�
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
}

//start����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();	//����DHCP����
#endif
	OSTaskCreate(led_poll_task,(void *)0,(OS_STK*)&LED_POLL_TASK_STK[LED_POLL_STK_SIZE-1],LED_POLL_TASK_PRIO);	  	//����LED����
	OSTaskCreate(key_poll_task,(void *)0,(OS_STK*)&KEY_POLL_TASK_STK[KEY_POLL_STK_SIZE-1],KEY_POLL_TASK_PRIO); 		//����KEY����
	OSTaskCreate(eMBPoll_task, (void *)0,(OS_STK*)&eMBPoll_TASK_STK[eMBPoll_STK_SIZE-1],   eMBPoll_TASK_PRIO); 		//����eMBPoll����
	OSTaskCreate(adc_poll_task,(void *)0,(OS_STK*)&ADC_POLL_TASK_STK[ADC_POLL_STK_SIZE-1],ADC_POLL_TASK_PRIO);		//����ADC����
	OSTaskCreate(dac_poll_task,(void *)0,(OS_STK*)&DAC_POLL_TASK_STK[DAC_POLL_STK_SIZE-1],DAC_POLL_TASK_PRIO);		//����ADC����
	OSTaskCreate(display_task, (void *)0,(OS_STK*)&DISPLAY_TASK_STK[DISPLAY_STK_SIZE-1],   DISPLAY_TASK_PRIO); 		//��ʾ����
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  //���ж�
}

//��ʾ��ַ����Ϣ��������
void display_task(void *pdata)
{
	while(1)
	{ 
#if LWIP_DHCP									//������DHCP��ʱ��
		if(lwipdev.dhcpstatus != 0) 			//����DHCP
		{
			show_address(lwipdev.dhcpstatus );	//��ʾ��ַ��Ϣ
			OSTaskSuspend(OS_PRIO_SELF); 		//��ʾ���ַ��Ϣ�������������
		}
#else
		show_address(0); 						//��ʾ��̬��ַ
		OSTaskSuspend(OS_PRIO_SELF); 			//��ʾ���ַ��Ϣ�������������
#endif //LWIP_DHCP
		OSTimeDlyHMSM(0,0,0,500);
	}
}

//������ѯ���� 
//Slave mode:DiscreteInputs variables
//************************************************************************//
//**02������ 
//**02 Read Discrete Inputs (1x)
//**��ʼ��ַ 100
//************************************************************************//
void key_poll_task(void *pdata)
{
	POINT_COLOR = BLUE; //��ɫ����
	LCD_ShowString(30,270,200,20,16,"KEYPoll is Start!"); 		//lwip��ʼ���ɹ�
	while(1)
	{
		Key_Poll();
		OSTimeDlyHMSM(0,0,0,10);  //��ʱ10ms
	}
}

//led��ѯ���� 
//************************************************************************//
//**01 05 15������
//**01 Read Coils��0x��  
//**05 Write Single Coil
//**15 Write Multiple Coils
//**��ʼ��ַ 000
//************************************************************************//
void led_poll_task(void *pdata)
{
	POINT_COLOR = BLUE; //��ɫ����
	LCD_ShowString(30,250,200,20,16,"LEDPoll is Start!"); 		//lwip��ʼ���ɹ�
    while(1)
    {
        LED_Poll();
		BEEP_Poll();
        OSTimeDlyHMSM(0,0,0,10);  //��ʱ10ms
    }
}

//eMBPoll����
//************************************************************************//
//**03 06 16������ 
//**03 Read Holding Registers(4x)
//**06 Write Single Registers
//**16 Write Multiple Registers
//**��ʼ��ַ 400
//************************************************************************//
void eMBPoll_task(void *pdata)
{
	eMBErrorCode    xStatus;
	eMBTCPInit(0);
	eMBEnable();
	POINT_COLOR = BLUE; //��ɫ����
	LCD_ShowString(30,210,200,20,16,"eMBPoll is Start!"); 		//
	while(1)
	{
		do
		{
			xStatus = eMBPoll(  );
			OSTimeDlyHMSM(0,0,0,10);  	//��ʱ10ms
		}while( xStatus == MB_ENOERR );
		( void )eMBDisable(  );
		( void )eMBClose(  );
		OSTimeDlyHMSM(0,0,0,10);  		//��ʱ10ms
	}
}

//Slave mode:InputRegister variables
//************************************************************************//
//**04������ (3x)
//**04 Read Input Registers
//**��ʼ��ַ 300
//************************************************************************//
void adc_poll_task(void *pdata)
{
	POINT_COLOR = BLUE; //��ɫ����
	LCD_ShowString(30,230,200,20,16,"ADCPoll is Start!"); 		//
    while(1)
    {
        ADC_Poll();
        OSTimeDlyHMSM(0,0,0,10);  //��ʱ10ms
    }
}

//Slave mode:HoldingRegister variables
//************************************************************************//
//**03 06 16������ 
//**03 Read  Holding Registers(4x)
//**06 Write Single Registers
//**16 Write Multiple Registers
//**��ʼ��ַ 40000
//************************************************************************//
void dac_poll_task(void *pdata)
{
	POINT_COLOR = BLUE; //��ɫ����
	LCD_ShowString(30,290,200,20,16,"DACPoll is Start!"); 		//
	while(1)
	{
		DAC_Poll();
		OSTimeDlyHMSM(0,0,0,10);  //��ʱ10ms
	}
}









