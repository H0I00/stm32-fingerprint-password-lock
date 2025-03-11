#include "button4_4.h"
#include "Delay.h"


void Button4_4_Init(void)//
{
	GPIO_InitTypeDef GPIO_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

}

int Button4_4_Scan(void)
{
    int keyValue = 0;
    
    GPIO_Write(KEY4_4PORT, ((KEY4_4PORT->ODR & 0xff00) | 0x1803));
    if((KEY4_4PORT->IDR & 0x00f0) == 0x0000)
        return -1;
    else
    {
        Delay_ms(20);
        
        if((KEY4_4PORT->IDR & 0x00f0) == 0x0000)
            return -1;    
    }
    
    GPIO_Write(KEY4_4PORT, (KEY4_4PORT->ODR & 0xe7fc) | 0x0001);
    switch(KEY4_4PORT->IDR & 0x00f0)
    {
        case 0x0080: keyValue = 16; break;
        case 0x0040: keyValue = 12; break;
        case 0x0020: keyValue = 8; break;
        case 0x0010: keyValue = 4; break;
    }
    while((KEY4_4PORT->IDR & 0x00f0) != 0x0000);

    GPIO_Write(KEY4_4PORT, (KEY4_4PORT->ODR & 0xe7fc) | 0x0002);
    switch(KEY4_4PORT->IDR & 0x00f0)
    {
        case 0x0080: keyValue = 15; break;
        case 0x0040: keyValue = 11; break;
        case 0x0020: keyValue = 7; break;
        case 0x0010: keyValue = 3; break;
    }
    while((KEY4_4PORT->IDR & 0x00f0) != 0x0000);

    GPIO_Write(KEY4_4PORT, (KEY4_4PORT->ODR & 0xe7fc) | 0x0800);
    switch(KEY4_4PORT->IDR & 0x00f0)
    {
        case 0x0080: keyValue = 14; break;
        case 0x0040: keyValue = 10; break;
        case 0x0020: keyValue = 6; break;
        case 0x0010: keyValue = 2; break;
    }
    while((KEY4_4PORT->IDR & 0x00f0) != 0x0000);

    GPIO_Write(KEY4_4PORT, (KEY4_4PORT->ODR & 0xe7fc) | 0x1000);
    switch(KEY4_4PORT->IDR & 0x00f0)
    {
        case 0x0080: keyValue = 13; break;
        case 0x0040: keyValue = 9; break;
        case 0x0020: keyValue = 5; break;
        case 0x0010: keyValue = 1; break;
    }
    while((KEY4_4PORT->IDR & 0x00f0) != 0x0000);

    return keyValue;
}



