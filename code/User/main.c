#include "stm32f10x.h"           
#include "button4_4.h"
#include "stmflash.h"
#include "usart2.h"
#include "usart.h"
#include "string.h"
#include "Delay.h"
#include "as608.h"
#include "timer.h"
#include "OLED.h"
#include "lock.h" 


void Add_FR(void);


#define usart2_baund  9600//串口2波特率，根据指纹模块波特率更改

//要写入到STM32 FLASH的字符串数组
const u8 TEXT_Buffer[]={0x17,0x23,0x6f,0x60,0,0};
#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//数组长度	
#define SIZE TEXT_LENTH/4+((TEXT_LENTH%4)?1:0)
#define FLASH_SAVE_ADDR  0X0802C124 	//设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
																			//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.

SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数
u8** kbd_tbl;

int starting();

int Set_password();


u8 passwd1[4] = { '0', '7', '2', '1' };	



int main(void)
{
	int Error;
	int key_num;
	int time1;
	int time2;		//锁屏时间
	char arrow=0;  //箭头位子
	
	SysPara AS608Para;					//指纹模块AS608参数
	u16 ValidN;									//模块内有效指纹个数
	u8** kbd_tbl;

	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	OLED_Init();
	Servo_Init();
	Delay_init();
	uart_init(9600);	 										//串口初始化为9600 
	Button4_4_Init();        						  //初始化与按键连接的硬件接口
	PS_StaGPIO_Init();
	usart2_init(usart2_baund);       	    //初始化指纹模块
	Buzzer_Init();
	

	OLED_Clear(); 
	starting();
	
		while (1)
		{		
MAIN:		OLED_Clear(); 
				while(1)
				{
					time1++;
					OLED_ShowNum(1,1,time1/170,4);
					OLED_ShowChinese(30,30,49);
					OLED_ShowChinese(30,45,50);
					OLED_ShowChinese(30,60,51);    //未开锁				

					if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
					{
						while(PS_Sta)
						{
							Error=press_FR();//刷指纹
							if(Error==0)
							{
								goto MENU;   //跳到解锁界面
							}								
							else 
							{
								OLED_ShowChinese(30,30,49);
								OLED_ShowChinese(30,45,50);
								OLED_ShowChinese(30,60,51);    //未开锁
							}
						}
					}
						//密码锁
					key_num=Button4_4_Scan();	//按键扫描
					if(key_num!=-1)
						{
							Error=password();//密码解锁函数
							if(Error==0)
							{
								goto MENU;	//跳到解锁界面
							}
							else 
							{
								OLED_ShowChinese(30,30,49);
								OLED_ShowChinese(30,45,50);
								OLED_ShowChinese(30,60,51);    //未开锁
							}
						}
				
						Delay_ms(1);

				}
MENU:
			OLED_Clear();
			OLED_ShowChinese(0,75,0);
      OLED_ShowChinese(0,90,1);//菜单					
				
MENUNOCLR:
			time2 = 30;
			OLED_ShowString(10, 1, "1:");
			OLED_ShowChinese(10, 20, 2);
			OLED_ShowChinese(10, 35, 3);
			OLED_ShowChinese(10, 50, 9);
			OLED_ShowChinese(10, 65, 10);  // 录入指纹

			OLED_ShowString(15, 1, "2:");
			OLED_ShowChinese(20, 20, 2);
			OLED_ShowChinese(20, 35, 3);
			OLED_ShowChinese(20, 50, 45);
			OLED_ShowChinese(20, 65, 46);  // 修改密码

    while (1)
    {
        time2--;
        OLED_ShowNum(1, 1, time2, 4);
			
			 if (time2 < 20)
            {
							lock_off();
            }		
			
        if (time2 < 0 || key_num == 4)
        {
            OLED_Clear();
						lock_off();
            OLED_ShowChinese(30, 30, 49);
            OLED_ShowChinese(30, 45, 50);
            OLED_ShowChinese(30, 60, 51);  // 未开锁
					
			
					
					
					
            if (time2 < 0)
            {
                time2 = 30;
                Delay_ms(1000);
                OLED_Clear();
								lock_off();
                goto MAIN;
            }
        }

        // 功能选项选择
        key_num = Button4_4_Scan();

            switch (key_num)
            {
                case 1:
                    Add_FR(); // 录指纹
                    break;

                case 2:
                    Error = Set_password(); // 修改密码
                    if (Error == 0)
                    {
                        lock_off(); // 解锁
                        goto MAIN; // 成功设置密码后跳回主标签
                    }
                    break;

                case 16:case 4:
                    lock_off(); // 其他处理
                    goto MAIN; // 按键16时跳回主标签
            }
        
        Delay_ms(1000);
    }
}
  
}




int password(void)
{
    int DisFlag;
    int key_num = 0, i = 0, satus = 0;
    uint16_t time3 = 0;
    u8 pwd[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }; // 支持8位密码输入
    u8 hidepwd[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' }; // 星号显示的密码
    u8 targetStr[9]; // 目标密码串，用于检查虚位掩码

    // 将预设密码转为字符串形式，以便后续比较
    sprintf((char *)targetStr, "%c%c%c%c", passwd1[0], passwd1[1], passwd1[2], passwd1[3]);

    OLED_Clear(); // 清屏
    OLED_ShowChinese(2, 15, 4);
    OLED_ShowChinese(2, 30, 43);
    OLED_ShowChinese(2, 45, 44);
    OLED_ShowChinese(2, 60, 45);
    OLED_ShowChinese(2, 75, 46); // 请输入密码

    while (1)
    {
        key_num = Button4_4_Scan();
        if (key_num != -1)
        {
            DisFlag = 1;
            time3 = 0;

            switch (key_num)
            {
                case 1: case 2: case 3:
                    pwd[i] = '0' + (key_num - 0);
                    hidepwd[i] = '*';
                    i++;
                    break;
                
                case 4: // 返回
                    OLED_Clear();
                    Delay_ms(500);
                    return -1;
                
                case 5: case 6: case 7:
                    pwd[i] = '0' + (key_num - 1);
                    hidepwd[i] = '*';
                    i++;
                    break;
                
                case 8: // 删除键
                    if (i > 0)
                    {
                        pwd[--i] = ' ';
                        hidepwd[i] = ' ';
                    }
                    break;

                case 9: case 10: case 11:
                    pwd[i] = '0' + (key_num - 2);
                    hidepwd[i] = '*';
                    i++;
                    break;
                
                case 12:
                    satus = !satus;
                    break;

                case 13: case 16: // 确认键
                    goto UNLOCK;

                case 15: // 清空键
                    memset(pwd, ' ', sizeof(pwd));
                    memset(hidepwd, ' ', sizeof(hidepwd));
                    i = 0;
                    break;
                
                case 14:
                    pwd[i] = '0';
                    hidepwd[i] = '*';
                    i++;
                    break;
            }

            if (DisFlag == 1)
            {
                if (satus == 0) OLED_ShowString(15, 1, hidepwd);
                else OLED_ShowString(20, 1, hidepwd);
            }

            time3++;
            if (time3 % 1000 == 0)
            {
                OLED_Clear(); // 清屏
                return -1;
            }
        }
    }

UNLOCK:
    OLED_ShowChinese(10, 15, 39);
    OLED_ShowChinese(10, 30, 40);
    OLED_ShowChinese(10, 45, 45);
    OLED_ShowChinese(10, 60, 46); // 检测密码
    Delay_ms(3000);

    // 验证密码
    int correct = 0;
    int len = strlen((char *)pwd); // 获取实际输入密码的长度
    for (int start = 0; start <= len - 4; start++)
    {
        // 检查密码中每个可能的4位子串是否包含预设密码
        if (strncmp((char *)&pwd[start], (char *)targetStr, 4) == 0)
        {
            correct = 1;
            break;
        }
    }

    if (correct)
    {
			  OLED_Clear();
        OLED_ShowChinese(10, 15, 37);
        OLED_ShowChinese(10, 30, 35);
        OLED_ShowChinese(10, 45, 50);
        OLED_ShowChinese(10, 60, 51); // 正确 开锁
				buzz_ok();
        lock_on();
        Delay_ms(4000);
        OLED_Clear(); // 清屏
        return 0;
    }
    else
    {
        OLED_Clear(); // 清屏
				buzz_no();
        OLED_ShowChinese(10, 15, 45);
        OLED_ShowChinese(10, 30, 46);
        OLED_ShowChinese(10, 45, 47);
        OLED_ShowChinese(10, 60, 48); // 密码错误
        Delay_ms(1500);
        OLED_Clear(); // 清屏
        return -1;
    }
}


//录指纹
void Add_FR(void)
{
    Delay_ms(500);
	u8 i,ensure ,processnum=0;
	int key_num;
	int ID=2;
	OLED_Clear();//清屏
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num==16){
			OLED_Clear();//清屏
			return ;
		}
		switch (processnum)
		{
			case 0:
				OLED_Clear();//清屏
				i++;
                OLED_ShowChinese(10,15,2);
                OLED_ShowChinese(10,30,3);    
                OLED_ShowChinese(10,45,9);
                OLED_ShowChinese(10,60,10);  //录入指纹
                OLED_ShowChinese(20,15,4);
                OLED_ShowChinese(20,30,5);    
                OLED_ShowChinese(20,45,9);
                OLED_ShowChinese(20,60,10);  //请按指纹	

                Delay_ms(6000);
                OLED_Clear();//清屏	
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer1);//生成特征

					if(ensure==0x00)
					{
                        OLED_ShowChinese(10,15,9);
                        OLED_ShowChinese(10,30,10);    
                        OLED_ShowChinese(10,45,11);
                        OLED_ShowChinese(10,60,12);  //指纹正常	

                        Delay_ms(5000);	
						i=0;
						processnum=1;//跳到第二步						
					}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //错误
                    }				
				}else {
                        OLED_ShowChinese(15,40,47);
                        OLED_ShowChinese(15,55,48);  //错误
                    }
				OLED_Clear();//清屏
				break;
			
			case 1:
				i++;
                OLED_ShowChinese(20,15,4);
                OLED_ShowChinese(20,30,5);    
                OLED_ShowChinese(20,45,6);
                OLED_ShowChinese(20,60,7);
                OLED_ShowChinese(20,75,8);
                OLED_ShowChinese(20,90,9); 
                OLED_ShowChinese(20,105,10);//请再按一次指纹	

                Delay_ms(5000);			
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{

                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer2);//生成特征
					if(ensure==0x00)
					{
												OLED_ShowChinese(10,15,9);
                        OLED_ShowChinese(10,30,10);    
                        OLED_ShowChinese(10,45,11);
                        OLED_ShowChinese(10,60,12);  //指纹正常	

                        Delay_ms(5000);	
						i=0;
						processnum=2;//跳到第三步
					}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //错误
                    }	
				}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //错误
                    }	
				OLED_Clear();//清屏
				break;

			case 2:		
				    OLED_ShowChinese(10,15,13);
                    OLED_ShowChinese(10,30,14);    
                    OLED_ShowChinese(10,45,9);
                    OLED_ShowChinese(10,60,10);  //对比指纹
                    Delay_ms(5000);	

				ensure=PS_Match();
				if(ensure==0x00) 
				{
					OLED_ShowChinese(20,15,9);
          OLED_ShowChinese(20,30,10);    
          OLED_ShowChinese(70,45,11);
          OLED_ShowChinese(20,60,52);  //指纹一样	
                    Delay_ms(5000);	

					processnum=3;//跳到第四步
				}
				else 
				{   
                    OLED_ShowChinese(20,40,9);
										OLED_ShowChinese(20,55,10);
										OLED_ShowChinese(20,70,47);
                    OLED_ShowChinese(20,85,48);  //指纹错误
                    Delay_ms(5000);	

                    OLED_Clear();//清屏
					OLED_ShowChinese(20,35,47);
                    OLED_ShowChinese(20,50,48);  //错误
					i=0;
                    Delay_ms(3000);
					OLED_Clear();//清屏
					processnum=0;//跳回第一步		
				}
				Delay_ms(1200);
				OLED_Clear();//清屏
				break;

			case 3:
			    OLED_ShowChinese(20,40,21);
          OLED_ShowChinese(20,55,22);
					OLED_ShowChinese(20,70,23);
          OLED_ShowChinese(20,85,24);  //生成模板
                Delay_ms(5000);		

				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					OLED_ShowChinese(20,40,23);
          OLED_ShowChinese(20,55,24); 
          OLED_ShowChinese(20,70,25);
          OLED_ShowChinese(20,85,26);  //模板成功
          Delay_ms(5000);	

					processnum=4;//跳到第五步
				}else {
                    processnum=0;
                    OLED_ShowString(5,4,"shengchengcuowu==");
                }
				Delay_ms(5200);
				break;
				
			case 4:	
		        OLED_Clear();//清屏
            Delay_ms(1000);	
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{				
          OLED_Clear();//清屏
					OLED_ShowChinese(10,15,2);   
          OLED_ShowChinese(10,45,9);
          OLED_ShowChinese(10,60,10); 
          OLED_ShowChinese(10,75,25); 
          OLED_ShowChinese(10,90,26);  //录指纹成功	
					OLED_ShowNum(10,2,ID,3);
					ID++;
          Delay_ms(5000);	
					OLED_Clear();	
					return ;
				}
				else 
				{
					processnum=0;
                    OLED_ShowChinese(10,40,47);
                    OLED_ShowChinese(10,55,48);  //错误
                    	
				}
						Delay_ms(3500);
				    OLED_Clear();//清屏					
				break;				
		}
		Delay_ms(400);
		if(i==10)//超过5次没有按手指则退出
		{
			OLED_Clear();
			break;
		}				
	}
}



//刷指纹
int press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char str[256];
	ensure=PS_GetImage();
	OLED_Clear();
    OLED_ShowChinese(10,15,39);
    OLED_ShowChinese(10,30,40);    
    OLED_ShowChinese(10,45,9);
    OLED_ShowChinese(10,60,10);
    OLED_ShowChinese(10,75,53);   //检测指纹中
	if(ensure==0x00)//获取图像成功 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{				
				OLED_Clear();
				
								buzz_ok();
								lock_on();
                OLED_ShowChinese(10,15,9);
                OLED_ShowChinese(10,30,10);
								OLED_ShowChinese(10,45,37);
                OLED_ShowChinese(10,60,35);    
                OLED_ShowChinese(10,75,50);
                OLED_ShowChinese(10,90,51);   //指纹正确开锁


				sprintf(str,"ID:%d     匹配分",seach.pageID);
	
				sprintf(str,":%d",seach.mathscore);
				
				
				
				Delay_ms(3800);
				OLED_Clear();
				return 0;
			}
			else {
				
				buzz_no();
				OLED_ShowChinese(20,40,9);
        OLED_ShowChinese(20,55,10);
				OLED_ShowChinese(20,40,47);
        OLED_ShowChinese(20,55,48);  //指纹错误
				
				OLED_ShowChinese(20,40,58);
        OLED_ShowChinese(20,55,59);
				OLED_ShowChinese(20,40,47);
        OLED_ShowChinese(20,55,48);  //错误
				Delay_ms(3000);

			
				OLED_Clear();
				return -1;
			}				
	  }
		else
			buzz_no();
			OLED_ShowChinese(10,40,47);
      OLED_ShowChinese(10,55,48);  //错误

	    //OLED_Refresh_Gram();//更新显示
	    Delay_ms(2000);
		OLED_Clear();
		
	}
	return -1;	
}



//开机信息
int starting(void)
{
	u8 ensure;
	char str[64];			  
	u8 key;
		OLED_ShowChinese(10,40,54);
    OLED_ShowChinese(10,55,55);  //欢迎
		Delay_ms(3000);
	
/*********************************开机信息提示***********************************/
		OLED_Clear();
		OLED_ShowChinese(10,10,56);
    OLED_ShowChinese(10,25,57);
    OLED_ShowChinese(10,40,58);
    OLED_ShowChinese(10,55,59);
    OLED_ShowChinese(10,70,60);
    OLED_ShowChinese(10,85,61);  //连接指纹模块
		Delay_ms(400);
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
				Delay_ms(400);
				OLED_Clear();
        OLED_ShowChinese(10,40,56);
        OLED_ShowChinese(10,55,57);
        OLED_ShowChinese(10,70,17);
        OLED_ShowChinese(10,85,18);  //连接失败
				OLED_Clear();
	}
				OLED_Clear();
        OLED_ShowChinese(10,40,56);
        OLED_ShowChinese(10,55,57);
        OLED_ShowChinese(10,70,25);
        OLED_ShowChinese(10,85,26);  //连接成功
				Delay_ms(1500);
				OLED_Clear();
	
	

				OLED_ShowString(10,5,"0221120000");
				OLED_ShowChinese(20,55,62);
				OLED_ShowChinese(20,70,63);
				OLED_ShowChinese(20,85,64);
	
	
	
				ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
				if(ensure!=0x00)
					OLED_Clear();
					ensure=PS_ReadSysPara(&AS608Para);  //读参数 

				OLED_ShowString(25,4,str);

	
			Delay_ms(1000);


			OLED_Clear();
}





int Set_password(void)
{
    int DisFlag;
    int key_num = 0, i = 0, status = 0;
    uint16_t time3 = 0;
    u8 pwd[4] = { ' ', ' ', ' ', ' ' };
    u8 hidepwd[5] = { ' ', ' ', ' ', ' ', '\0' };

    OLED_Clear();
    OLED_ShowChinese(2, 15, 4);
    OLED_ShowChinese(2, 30, 43);
    OLED_ShowChinese(2, 45, 44);
    OLED_ShowChinese(2, 60, 68);
    OLED_ShowChinese(2, 75, 69);
    OLED_ShowChinese(2, 90, 45);
    OLED_ShowChinese(2, 105, 46);

    while (1)
    {
        key_num = Button4_4_Scan();
        if (key_num != -1)
        {
            DisFlag = 1;
            time3 = 0;

            switch (key_num)
            {
                case 1: case 2: case 3:
                    pwd[i] = '0' + (key_num - 0);
                    hidepwd[i] = '*';
                    i++;
                    break;

                case 4:
                    OLED_Clear();
                    Delay_ms(500);
                    return -1; // 返回-1，表示取消设置密码

                case 5: case 6: case 7:
                    pwd[i] = '0' + (key_num - 1);
                    hidepwd[i] = '*';
                    i++;
                    break;

                case 8:
                    if (i > 0)
                    {
                        pwd[--i] = ' ';
                        hidepwd[i] = ' ';
                    }
                    break;

                case 9: case 10: case 11:
                    pwd[i] = '0' + (key_num - 2);
                    hidepwd[i] = '*';
                    i++;
                    break;

                case 12:
                    status = !status;
                    break;

                case 16:
                    if (i == 4) // 确认键，只有当输入完整四位密码时才确认
                    {
                        memcpy(passwd1, pwd, sizeof(pwd));
                        OLED_Clear();
                        Delay_ms(500);
                        return 0; // 返回0，表示成功设置密码
                    }
                    break;

                case 15:
                    memset(pwd, ' ', sizeof(pwd));
                    memset(hidepwd, ' ', sizeof(hidepwd));
                    i = 0;
                    break;

                case 14:
                    pwd[i] = '0';
                    hidepwd[i] = '*';
                    i++;
                    break;
            }

            if (DisFlag == 1)
            {
                if (status == 0) OLED_ShowString(15, 1, hidepwd);
                else OLED_ShowString(20, 1, hidepwd);
            }

            time3++;
            if (time3 % 1000 == 0)
            {
                OLED_Clear();
                return -1; // 超时返回-1
            }
        }
    }
}

