#include "lock.h"   
#include "stm32f10x.h" 
#include "Delay.h"

// ��������������
#define SERVO_PIN GPIO_Pin_12
#define SERVO_PORT GPIOB

// �����������������
#define BUZZER_PIN GPIO_Pin_13
#define BUZZER_PORT GPIOB

void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ����GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // ���ö����������Ϊ���ģʽ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = SERVO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SERVO_PORT, &GPIO_InitStructure);
}

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ���÷�������������Ϊ���ģʽ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
	
		GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
    Delay_ms(50);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
	
}

void Servo_Write(uint16_t pulseWidth)
{
    // �������Ÿߵ�ƽ
    GPIO_SetBits(SERVO_PORT, SERVO_PIN);
    // ���ָߵ�ƽ����ʱ��
    Delay_us(pulseWidth);
    // �������ŵ͵�ƽ
    GPIO_ResetBits(SERVO_PORT, SERVO_PIN);
    // ���20ms����
    Delay_us(20000 - pulseWidth);
}

void lock_on(void)
{
    // ���ƶ��ת������λ�� (����180��)
    for (int i = 0; i < 50; i++) // ����λ��һ��ʱ��
    {
        Servo_Write(1800); // ���ݾ���Ķ���������ֵ��2ms��
    }
}

void lock_off(void)
{
    // ���ƶ��ת�����ر�λ�� (����0��)
    for (int i = 0; i < 50; i++) // ����λ��һ��ʱ��
    {
        Servo_Write(1000); // ���ݾ���Ķ���������ֵ��1ms��
    }
}

void lock_4_seconds(void)
{
    lock_on();
    Delay_ms(4000);
    lock_off();
}

void buzz_ok(void)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
    Delay_ms(100);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
    Delay_ms(100);
		GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
    Delay_ms(100);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}

void buzz_no(void)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
    Delay_ms(1000);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}

