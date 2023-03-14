/******************************************************************************
* Basic timers TIM7

  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트C (PC0 ~ PC7)를 8핀 케이블를 이용해서
        Array FND모듈의 SA_A ~ SA_H에 연결한다. (SA_A가 PC0로 연결되야 한다.)
    2)  ARM Cortex-M4 모듈의 포트C (PC8 ~ PC11)를 4핀 케이블를 이용해서
        Array FND모듈의 C0 ~ C3에 연결한다. (C0가 PC8로 연결되야 한다.)
******************************************************************************/

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

int t_cnt = 0,m_cnt=0;


void Segment ( int N )  // Segment 함수 선언
{
  int  i ;
  unsigned char N1000, N100, N10, N1 ;
  int Buff ;

  N1000 = N /1000;  // 세그먼트에서 사용하는 천의 자리를 추출
  Buff = N % 1000 ;
  N100 = Buff / 100 ; // 세그먼트에서 사용하는 백의자리 추출
  Buff = Buff % 100;
  N10 = Buff /10 ;     // 세그먼트에서 사용하는 십의 자리 추출
  N1 =  Buff % 10 ;    // 세그먼트에서 사용하는 일의 자리 추출      

  for( i = 0 ; i < 30; i++ )
  {
    GPIO_Write(GPIOC, Font[N1000]|0x0e00); // 왼쪽 첫 번째 세그먼트를 ON하고, 천의 자리  숫자를 출력해 준다 
    Delay(1);

    GPIO_Write(GPIOC, Font[N100]|0x0d00); // 왼쪽 두 번째 세그먼트를 ON하고, 백의 자리  숫자를 출력해 준다 
    Delay(1); 

    GPIO_Write(GPIOC, Font[N10]|0x0b00);  // 왼쪽 세 번째 세그먼트를 ON하고, 십의 자리  숫자를 출력해 준다 
    Delay(1);

    GPIO_Write(GPIOC, Font[N1]|0x0700);   // 왼쪽 네 번째 세그먼트를 ON하고, 일의 자리  숫자를 출력해 준다 
    Delay(1); 
  }
}

void TIM7_IRQHandler(void) //1Sec 마다 호출
{
  if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
/*    t_cnt++ ;   //ms 마다 증가
    if(t_cnt >= 1000)  //1ms * 1000 = 1Sec
    {
      t_cnt = 0;
      
m_cnt ++;;        //1Sec 마다 증가
    }
*/
    m_cnt ++;
  }
}


int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
                                GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //system_stm32f4xx.c 참조
  //TIM7 input clock (TIM7CLK) is set to 2 * APB1 clock (PCLK1),
  //since APB1 prescaler is different from 1.
  //TIM7CLK = 2 * PCLK1
  //PCLK1 = HCLK / 4  => TIM7CLK = HCLK / 2 = SystemCoreClock /2
  //TIM_CounterMode를 TIM_CounterMode_Up로 설정하면, 카운트값이 TIMx_ARR값과
  //같아질때, Update 이벤트가 발생한다.
  //TIM_Prescaler를 83으로, TIM_ClockDivision를 TIM_CKD_DIV1(1분주)로
  //설정하면, 타이머7 카운트 클럭은 (84MHz/1) /(83+1) = 1MHz (1us)이다.
  //TIM_Period를 999으로 설정했으니(TIMx_ARR), 카운터가 이값과 같아지면,
  //(1us x 1000(999+1) = 1ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;         //(168Mhz/2)/84 = 1MHz(1us)
  TIM_TimeBaseStructure.TIM_Period = 10000-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  //타이머7을 동작시킨다.
  TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);

  while(1)
  {
    Segment(m_cnt);
  }
}
