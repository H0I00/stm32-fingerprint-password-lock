#include "lock.h"   
#include "stm32f10x.h" 
#include "Delay.h"

// 定义舵机控制引脚
#define SERVO_PIN GPIO_Pin_12
#define SERVO_PORT GPIOB

// 定义蜂鸣器控制引脚
#define BUZZER_PIN GPIO_Pin_13
#define BUZZER_PORT GPIOB

void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 启用GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置舵机控制引脚为输出模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = SERVO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SERVO_PORT, &GPIO_InitStructure);
}

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置蜂鸣器控制引脚为输出模式
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
    // 设置引脚高电平
    GPIO_SetBits(SERVO_PORT, SERVO_PIN);
    // 保持高电平持续时间
    Delay_us(pulseWidth);
    // 设置引脚低电平
    GPIO_ResetBits(SERVO_PORT, SERVO_PIN);
    // 完成20ms周期
    Delay_us(20000 - pulseWidth);
}

void lock_on(void)
{
    // 控制舵机转动到打开位置 (例如180度)
    for (int i = 0; i < 50; i++) // 保持位置一段时间
    {
        Servo_Write(1800); // 根据具体的舵机调整这个值（2ms）
    }
}

void lock_off(void)
{
    // 控制舵机转动到关闭位置 (例如0度)
    for (int i = 0; i < 50; i++) // 保持位置一段时间
    {
        Servo_Write(1000); // 根据具体的舵机调整这个值（1ms）
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

