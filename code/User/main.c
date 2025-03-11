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


#define usart2_baund  9600//����2�����ʣ�����ָ��ģ�鲨���ʸ���

//Ҫд�뵽STM32 FLASH���ַ�������
const u8 TEXT_Buffer[]={0x17,0x23,0x6f,0x60,0,0};
#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//���鳤��	
#define SIZE TEXT_LENTH/4+((TEXT_LENTH%4)?1:0)
#define FLASH_SAVE_ADDR  0X0802C124 	//����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
																			//����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.

SysPara AS608Para;//ָ��ģ��AS608����
u16 ValidN;//ģ������Чָ�Ƹ���
u8** kbd_tbl;

int starting();

int Set_password();


u8 passwd1[4] = { '0', '7', '2', '1' };	



int main(void)
{
	int Error;
	int key_num;
	int time1;
	int time2;		//����ʱ��
	char arrow=0;  //��ͷλ��
	
	SysPara AS608Para;					//ָ��ģ��AS608����
	u16 ValidN;									//ģ������Чָ�Ƹ���
	u8** kbd_tbl;

	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	
	OLED_Init();
	Servo_Init();
	Delay_init();
	uart_init(9600);	 										//���ڳ�ʼ��Ϊ9600 
	Button4_4_Init();        						  //��ʼ���밴�����ӵ�Ӳ���ӿ�
	PS_StaGPIO_Init();
	usart2_init(usart2_baund);       	    //��ʼ��ָ��ģ��
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
					OLED_ShowChinese(30,60,51);    //δ����				

					if(PS_Sta)	 //���PS_Sta״̬���������ָ����
					{
						while(PS_Sta)
						{
							Error=press_FR();//ˢָ��
							if(Error==0)
							{
								goto MENU;   //������������
							}								
							else 
							{
								OLED_ShowChinese(30,30,49);
								OLED_ShowChinese(30,45,50);
								OLED_ShowChinese(30,60,51);    //δ����
							}
						}
					}
						//������
					key_num=Button4_4_Scan();	//����ɨ��
					if(key_num!=-1)
						{
							Error=password();//�����������
							if(Error==0)
							{
								goto MENU;	//������������
							}
							else 
							{
								OLED_ShowChinese(30,30,49);
								OLED_ShowChinese(30,45,50);
								OLED_ShowChinese(30,60,51);    //δ����
							}
						}
				
						Delay_ms(1);

				}
MENU:
			OLED_Clear();
			OLED_ShowChinese(0,75,0);
      OLED_ShowChinese(0,90,1);//�˵�					
				
MENUNOCLR:
			time2 = 30;
			OLED_ShowString(10, 1, "1:");
			OLED_ShowChinese(10, 20, 2);
			OLED_ShowChinese(10, 35, 3);
			OLED_ShowChinese(10, 50, 9);
			OLED_ShowChinese(10, 65, 10);  // ¼��ָ��

			OLED_ShowString(15, 1, "2:");
			OLED_ShowChinese(20, 20, 2);
			OLED_ShowChinese(20, 35, 3);
			OLED_ShowChinese(20, 50, 45);
			OLED_ShowChinese(20, 65, 46);  // �޸�����

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
            OLED_ShowChinese(30, 60, 51);  // δ����
					
			
					
					
					
            if (time2 < 0)
            {
                time2 = 30;
                Delay_ms(1000);
                OLED_Clear();
								lock_off();
                goto MAIN;
            }
        }

        // ����ѡ��ѡ��
        key_num = Button4_4_Scan();

            switch (key_num)
            {
                case 1:
                    Add_FR(); // ¼ָ��
                    break;

                case 2:
                    Error = Set_password(); // �޸�����
                    if (Error == 0)
                    {
                        lock_off(); // ����
                        goto MAIN; // �ɹ������������������ǩ
                    }
                    break;

                case 16:case 4:
                    lock_off(); // ��������
                    goto MAIN; // ����16ʱ��������ǩ
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
    u8 pwd[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }; // ֧��8λ��������
    u8 hidepwd[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' }; // �Ǻ���ʾ������
    u8 targetStr[9]; // Ŀ�����봮�����ڼ����λ����

    // ��Ԥ������תΪ�ַ�����ʽ���Ա�����Ƚ�
    sprintf((char *)targetStr, "%c%c%c%c", passwd1[0], passwd1[1], passwd1[2], passwd1[3]);

    OLED_Clear(); // ����
    OLED_ShowChinese(2, 15, 4);
    OLED_ShowChinese(2, 30, 43);
    OLED_ShowChinese(2, 45, 44);
    OLED_ShowChinese(2, 60, 45);
    OLED_ShowChinese(2, 75, 46); // ����������

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
                
                case 4: // ����
                    OLED_Clear();
                    Delay_ms(500);
                    return -1;
                
                case 5: case 6: case 7:
                    pwd[i] = '0' + (key_num - 1);
                    hidepwd[i] = '*';
                    i++;
                    break;
                
                case 8: // ɾ����
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

                case 13: case 16: // ȷ�ϼ�
                    goto UNLOCK;

                case 15: // ��ռ�
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
                OLED_Clear(); // ����
                return -1;
            }
        }
    }

UNLOCK:
    OLED_ShowChinese(10, 15, 39);
    OLED_ShowChinese(10, 30, 40);
    OLED_ShowChinese(10, 45, 45);
    OLED_ShowChinese(10, 60, 46); // �������
    Delay_ms(3000);

    // ��֤����
    int correct = 0;
    int len = strlen((char *)pwd); // ��ȡʵ����������ĳ���
    for (int start = 0; start <= len - 4; start++)
    {
        // ���������ÿ�����ܵ�4λ�Ӵ��Ƿ����Ԥ������
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
        OLED_ShowChinese(10, 60, 51); // ��ȷ ����
				buzz_ok();
        lock_on();
        Delay_ms(4000);
        OLED_Clear(); // ����
        return 0;
    }
    else
    {
        OLED_Clear(); // ����
				buzz_no();
        OLED_ShowChinese(10, 15, 45);
        OLED_ShowChinese(10, 30, 46);
        OLED_ShowChinese(10, 45, 47);
        OLED_ShowChinese(10, 60, 48); // �������
        Delay_ms(1500);
        OLED_Clear(); // ����
        return -1;
    }
}


//¼ָ��
void Add_FR(void)
{
    Delay_ms(500);
	u8 i,ensure ,processnum=0;
	int key_num;
	int ID=2;
	OLED_Clear();//����
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num==16){
			OLED_Clear();//����
			return ;
		}
		switch (processnum)
		{
			case 0:
				OLED_Clear();//����
				i++;
                OLED_ShowChinese(10,15,2);
                OLED_ShowChinese(10,30,3);    
                OLED_ShowChinese(10,45,9);
                OLED_ShowChinese(10,60,10);  //¼��ָ��
                OLED_ShowChinese(20,15,4);
                OLED_ShowChinese(20,30,5);    
                OLED_ShowChinese(20,45,9);
                OLED_ShowChinese(20,60,10);  //�밴ָ��	

                Delay_ms(6000);
                OLED_Clear();//����	
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer1);//��������

					if(ensure==0x00)
					{
                        OLED_ShowChinese(10,15,9);
                        OLED_ShowChinese(10,30,10);    
                        OLED_ShowChinese(10,45,11);
                        OLED_ShowChinese(10,60,12);  //ָ������	

                        Delay_ms(5000);	
						i=0;
						processnum=1;//�����ڶ���						
					}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //����
                    }				
				}else {
                        OLED_ShowChinese(15,40,47);
                        OLED_ShowChinese(15,55,48);  //����
                    }
				OLED_Clear();//����
				break;
			
			case 1:
				i++;
                OLED_ShowChinese(20,15,4);
                OLED_ShowChinese(20,30,5);    
                OLED_ShowChinese(20,45,6);
                OLED_ShowChinese(20,60,7);
                OLED_ShowChinese(20,75,8);
                OLED_ShowChinese(20,90,9); 
                OLED_ShowChinese(20,105,10);//���ٰ�һ��ָ��	

                Delay_ms(5000);			
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{

                    Delay_ms(500);
					ensure=PS_GenChar(CharBuffer2);//��������
					if(ensure==0x00)
					{
												OLED_ShowChinese(10,15,9);
                        OLED_ShowChinese(10,30,10);    
                        OLED_ShowChinese(10,45,11);
                        OLED_ShowChinese(10,60,12);  //ָ������	

                        Delay_ms(5000);	
						i=0;
						processnum=2;//����������
					}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //����
                    }	
				}else {
                        OLED_ShowChinese(10,40,47);
                        OLED_ShowChinese(10,55,48);  //����
                    }	
				OLED_Clear();//����
				break;

			case 2:		
				    OLED_ShowChinese(10,15,13);
                    OLED_ShowChinese(10,30,14);    
                    OLED_ShowChinese(10,45,9);
                    OLED_ShowChinese(10,60,10);  //�Ա�ָ��
                    Delay_ms(5000);	

				ensure=PS_Match();
				if(ensure==0x00) 
				{
					OLED_ShowChinese(20,15,9);
          OLED_ShowChinese(20,30,10);    
          OLED_ShowChinese(70,45,11);
          OLED_ShowChinese(20,60,52);  //ָ��һ��	
                    Delay_ms(5000);	

					processnum=3;//�������Ĳ�
				}
				else 
				{   
                    OLED_ShowChinese(20,40,9);
										OLED_ShowChinese(20,55,10);
										OLED_ShowChinese(20,70,47);
                    OLED_ShowChinese(20,85,48);  //ָ�ƴ���
                    Delay_ms(5000);	

                    OLED_Clear();//����
					OLED_ShowChinese(20,35,47);
                    OLED_ShowChinese(20,50,48);  //����
					i=0;
                    Delay_ms(3000);
					OLED_Clear();//����
					processnum=0;//���ص�һ��		
				}
				Delay_ms(1200);
				OLED_Clear();//����
				break;

			case 3:
			    OLED_ShowChinese(20,40,21);
          OLED_ShowChinese(20,55,22);
					OLED_ShowChinese(20,70,23);
          OLED_ShowChinese(20,85,24);  //����ģ��
                Delay_ms(5000);		

				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					OLED_ShowChinese(20,40,23);
          OLED_ShowChinese(20,55,24); 
          OLED_ShowChinese(20,70,25);
          OLED_ShowChinese(20,85,26);  //ģ��ɹ�
          Delay_ms(5000);	

					processnum=4;//�������岽
				}else {
                    processnum=0;
                    OLED_ShowString(5,4,"shengchengcuowu==");
                }
				Delay_ms(5200);
				break;
				
			case 4:	
		        OLED_Clear();//����
            Delay_ms(1000);	
				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
				if(ensure==0x00) 
				{				
          OLED_Clear();//����
					OLED_ShowChinese(10,15,2);   
          OLED_ShowChinese(10,45,9);
          OLED_ShowChinese(10,60,10); 
          OLED_ShowChinese(10,75,25); 
          OLED_ShowChinese(10,90,26);  //¼ָ�Ƴɹ�	
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
                    OLED_ShowChinese(10,55,48);  //����
                    	
				}
						Delay_ms(3500);
				    OLED_Clear();//����					
				break;				
		}
		Delay_ms(400);
		if(i==10)//����5��û�а���ָ���˳�
		{
			OLED_Clear();
			break;
		}				
	}
}



//ˢָ��
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
    OLED_ShowChinese(10,75,53);   //���ָ����
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		
			
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{				
				OLED_Clear();
				
								buzz_ok();
								lock_on();
                OLED_ShowChinese(10,15,9);
                OLED_ShowChinese(10,30,10);
								OLED_ShowChinese(10,45,37);
                OLED_ShowChinese(10,60,35);    
                OLED_ShowChinese(10,75,50);
                OLED_ShowChinese(10,90,51);   //ָ����ȷ����


				sprintf(str,"ID:%d     ƥ���",seach.pageID);
	
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
        OLED_ShowChinese(20,55,48);  //ָ�ƴ���
				
				OLED_ShowChinese(20,40,58);
        OLED_ShowChinese(20,55,59);
				OLED_ShowChinese(20,40,47);
        OLED_ShowChinese(20,55,48);  //����
				Delay_ms(3000);

			
				OLED_Clear();
				return -1;
			}				
	  }
		else
			buzz_no();
			OLED_ShowChinese(10,40,47);
      OLED_ShowChinese(10,55,48);  //����

	    //OLED_Refresh_Gram();//������ʾ
	    Delay_ms(2000);
		OLED_Clear();
		
	}
	return -1;	
}



//������Ϣ
int starting(void)
{
	u8 ensure;
	char str[64];			  
	u8 key;
		OLED_ShowChinese(10,40,54);
    OLED_ShowChinese(10,55,55);  //��ӭ
		Delay_ms(3000);
	
/*********************************������Ϣ��ʾ***********************************/
		OLED_Clear();
		OLED_ShowChinese(10,10,56);
    OLED_ShowChinese(10,25,57);
    OLED_ShowChinese(10,40,58);
    OLED_ShowChinese(10,55,59);
    OLED_ShowChinese(10,70,60);
    OLED_ShowChinese(10,85,61);  //����ָ��ģ��
		Delay_ms(400);
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
				Delay_ms(400);
				OLED_Clear();
        OLED_ShowChinese(10,40,56);
        OLED_ShowChinese(10,55,57);
        OLED_ShowChinese(10,70,17);
        OLED_ShowChinese(10,85,18);  //����ʧ��
				OLED_Clear();
	}
				OLED_Clear();
        OLED_ShowChinese(10,40,56);
        OLED_ShowChinese(10,55,57);
        OLED_ShowChinese(10,70,25);
        OLED_ShowChinese(10,85,26);  //���ӳɹ�
				Delay_ms(1500);
				OLED_Clear();
	
	

				OLED_ShowString(10,5,"0221120000");
				OLED_ShowChinese(20,55,62);
				OLED_ShowChinese(20,70,63);
				OLED_ShowChinese(20,85,64);
	
	
	
				ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
				if(ensure!=0x00)
					OLED_Clear();
					ensure=PS_ReadSysPara(&AS608Para);  //������ 

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
                    return -1; // ����-1����ʾȡ����������

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
                    if (i == 4) // ȷ�ϼ���ֻ�е�����������λ����ʱ��ȷ��
                    {
                        memcpy(passwd1, pwd, sizeof(pwd));
                        OLED_Clear();
                        Delay_ms(500);
                        return 0; // ����0����ʾ�ɹ���������
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
                return -1; // ��ʱ����-1
            }
        }
    }
}

