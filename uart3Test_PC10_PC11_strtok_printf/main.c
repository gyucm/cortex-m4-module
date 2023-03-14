/******************************************************************************
  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트C의 10~110번핀을(PC10~PC11) 2핀 케이블로
        UART모듈의 RXD, TXD 핀에 연결한다.
******************************************************************************/
// stm32f4xx의 각 레지스터들을 정의한 헤더파일
#include "stm32f4xx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CMD_SIZE 50
#define ARR_CNT 5  


volatile unsigned char rx3Flag = 0;
char rx3Data[50];
void Serial3_Event();
void Serial3_Send(unsigned char t);
void Serial3_Send_String(char* s);
int putchar(int ch);            //printf() 사용시 필요
void print_2d1(double number);

int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //LED0~7

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                             GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; // C 포트 하위 8비트를 출력으로 선언한다.
  GPIO_Init(GPIOC, &GPIO_InitStructure);  


  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);     //USART3_TX
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);    //USART3_RX

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);

  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // USART3 Interrupt enable
  USART_Cmd(USART3, ENABLE);

  Serial3_Send_String("Start Main()\n\r");
  while(1)
  {
    if(rx3Flag)  // '\r' 까지 입력되면
      Serial3_Event();
  }
}

void Serial3_Event()
{
  int i=0;
  int num = 0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0};       
  strcpy(recvBuf,rx3Data);

  rx3Flag = 0; // 다시 Rflag 를 0으로 놓는다.    
  Serial3_Send_String(recvBuf);
  Serial3_Send_String("\n\r");
  printf("rx : %s\r\n",recvBuf);
//  printf("test : %f\r\n",12.3) ; 
//  printf("\n\r");  
//  print_2d1(12.3);
  pToken = strtok(recvBuf,"[@]");

  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
       
  if(!strcmp(pArray[1],"LED"))
  {
    if(pArray[3] != NULL)              
      num = atoi(pArray[3]);
    else
      num = 0;
    if(!strcmp(pArray[2],"ON"))
    {
     GPIO_SetBits(GPIOC, 0x01 << num); 
    }
    else 
    {
      GPIO_ResetBits(GPIOC, 0x01 << num); 
    }
  }  
}

void Serial3_Send(unsigned char t)
{
  USART_SendData(USART3, t);
  while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void Serial3_Send_String(char* s)
{
  while( *s != '\0')
  {
   Serial3_Send((unsigned char)(*s));
   s++;   //s = s + 1;
  }
}

int putchar(int ch)
{
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3,ch);
	return ch;
}

void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {
	static int i=0;
	rx3Data[i] = USART_ReceiveData(USART3);
	if(rx3Data[i] == '\r')  
	{
		rx3Data[i] = '\0';
		rx3Flag = 1;
		i = 0;
	}
	else
	{
		i++;
	}
  }
}

void print_2d1(double number)		        /* floating-point number xx.x */
{ 
        unsigned int i, j;

	j = (int)(number*10. + 0.5);
	i = j / 100;					// 10^1
	if(i == 0) Serial3_Send(' ');
	else       Serial3_Send(i + '0');

	j = j % 100;					// 10^0
	i = j / 10;
	Serial3_Send(i + '0');
	Serial3_Send('.');

	i = j % 10;					// 10^-1
	Serial3_Send(i + '0');
}