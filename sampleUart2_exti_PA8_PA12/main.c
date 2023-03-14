#include "stm32f4xx.h"
#include "uart2.h"
#include <stdio.h>


// delay 함수
static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}

// 7-세그먼트 폰트를 배열로 지정한다.
unsigned char Font[18] = {0x3F, 0X06, 0X5B, 0X4F, 
                                         0X66, 0X6D, 0X7C, 0X07,
                                         0X7F, 0X67, 0X77, 0X7C, 
                                         0X39, 0X5E, 0X79, 0X71, 
                                         0X08, 0X80};

volatile int count = 0; // 인터럽트가 몇 번 수행되었는가를 나타내주는 변수이다.
volatile int intNum = 0;
volatile int intFlag = 0;
// Dec Segment 함수 선언 ==> 이 함수는 입력되는 값을 10진값으로 
//  7-세그먼트에 표시해 주는 함수이다  
void DecSegment ( unsigned char N )
{
  int  i ;
  unsigned char  N10, N1 ;


  N10 = N /10;  // 세그먼트에서 사용하는 첫번째의 자리를 추출
  N1 = N % 10 ;


  for( i = 0 ; i < 30; i++ )
  {
    GPIO_Write(GPIOC, Font[N10]|0x0b00); // 왼쪽 세 번째 세그먼트를 ON하고, 첫 번째  숫자를 출력해 준다 
    Delay(1);        

    GPIO_Write(GPIOC, Font[N1]|0x0700); // 왼쪽 네 번째 세그먼트를 ON하고, 두 번째  숫자를 출력해 준다 
    Delay(1);     
  }
} 

void EXTI9_5_IRQHandler(void)
{
  intFlag = 1;
  if(EXTI_GetITStatus(EXTI_Line8) != RESET)
  {
    intNum = 8;
    count++;
    EXTI_ClearITPendingBit(EXTI_Line8);
  }
  else if(EXTI_GetITStatus(EXTI_Line9) != RESET)
  {
    intNum = 9;
    count--;
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
}
void EXTI15_10_IRQHandler(void)
{
  intFlag = 1;
  if(EXTI_GetITStatus(EXTI_Line10) != RESET)
  {
    intNum = 10;
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
  if(EXTI_GetITStatus(EXTI_Line11) != RESET)
  {
    intNum = 11;
    EXTI_ClearITPendingBit(EXTI_Line11);
  }
  if(EXTI_GetITStatus(EXTI_Line12) != RESET)
  {
    intNum = 12;
    EXTI_ClearITPendingBit(EXTI_Line12);
  }
}

int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
                         
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
                                GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //포트A의 0번핀을 외부 인터럽트 0 과 연결한다.
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource9);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource11);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource12);
  
  //외부 인터럽트0 인터럽트를 허용한다.(우선순위는 가장높게 설정)
  //외부 인터럽트0가 상승엣지에서 발생되게 설정한다.
  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line9|EXTI_Line10|EXTI_Line11|EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  count = 0;
  
  
  UART2_init();
  printf("Start main()\r\n");
  while(1)
  {
    DecSegment(count);
    if(intFlag)
    {
      int test = count;
      printf("EXTI : %d, count : %d\r\n",intNum,test);
      intFlag = 0;
    }  
  }
}