/////////////////////////////////////////////////////////////////////////////////////	 
//�ļ�����led.c
//�ļ�������LED����
//ʵ�ֹ��ܣ�����LED�Ƶ�����
/////////////////////////////////////////////////////////////////////////////////////

#include "led.h"   
#include "stm32f10x.h" 
#include "Delay.h"

//��ʼ��PA0��PA1Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��

void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_15);
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_15);
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_15);
}

void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_15) == 0)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_15);
	}
	else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_15);
	}
}

void LED2_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}

void LED2_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
}

void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_14) == 0)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_14);
	}
	else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_14);
	}
}
void LED3_ON(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void LED3_OFF(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void LED3_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
}
