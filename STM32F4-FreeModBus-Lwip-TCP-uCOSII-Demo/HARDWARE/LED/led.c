#include "led.h"
#include "user_mb_app.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //ʹ��GPIOD��ʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //����
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //����GPIO
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOF,GPIO_Pin_9|GPIO_Pin_10); //GPIOF9,10�ߵ�ƽ
}

void Set_Led(u8 LED,u8 state)
{
	if(LED==0)
	{
		switch(state)
		{
			case 0: LED0=1;
				break;
			case 1: LED0=0;
				break;
		}
	}else if(LED==1)
	{
		switch(state)
		{
			case 0: LED1=1;
				break;
			case 1: LED1=0;
				break;
		}
	}
}

//Slave mode:Coils variables
///***********************************************************************************************************
//**01 05 15������
//**01 Read  Coils��0x��  
//**05 Write Single Coil
//**15 Write Multiple Coils
//**������LED_Poll();
//**���ܣ�LED��ѯ��������ȡ��Ȧ���棬�ı�LED״̬
///**********************************************************************************************************/
extern UCHAR    ucSCoilBuf[];
void LED_Poll(void)
{
    u8 LED_Status = ucSCoilBuf[0];
    if(LED_Status & 0x01) {LED0 = 0;} else {LED0 = 1;}
    if(LED_Status & 0x02) {LED1 = 0;} else {LED1 = 1;}
}

