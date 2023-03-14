#include "stm32f4xx.h"
#include "uart2.h"
#include <stdio.h>
/******************************************************************************
* TIM3 & TIM4

  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트C (PC0 ~ PC7)를 8핀 케이블를 이용해서
        Array FND모듈의 SA_A ~ SA_H에 연결한다. (SA_A가 PC0로 연결되야 한다.)
    2)  ARM Cortex-M4 모듈의 포트C (PC8 ~ PC11)를 4핀 케이블를 이용해서
        Array FND모듈의 C0 ~ C3에 연결한다. (C0가 PC8로 연결되야 한다.)
    3)  ARM Cortex-M4 모듈의 포트A의 PA6을 2핀 케이블를 이용해서 
        포트B의 PB6에 연결한다.
******************************************************************************/

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

uint16_t IC1ReadValue1 = 0, IC1ReadValue2 = 0;
volatile uint16_t CaptureNumber = 0;
volatile unsigned int c_value;


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

void TIM4_IRQHandler(void)
{ 
  if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET) 
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    if(CaptureNumber == 0)
    {
      IC1ReadValue1 = TIM_GetCapture1(TIM4);
      CaptureNumber = 1;
    }
    else if(CaptureNumber == 1)
    {
      IC1ReadValue2 = TIM_GetCapture1(TIM4);

      if (IC1ReadValue2 > IC1ReadValue1)
      {
        c_value = (IC1ReadValue2 - IC1ReadValue1); 
      }
      else
      {
        c_value = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2); 
      }
      CaptureNumber = 0;
    }
  }
}


int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_ICInitTypeDef    TIM_ICInitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
                                GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //PWM 출력핀, TIM3 channel 1 pin (PA.06)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;             // TIM3_CH1
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //입력캡쳐핀, TIM4 channel 1 pin (PB.06)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;             // TIM4_CH1
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);

  // 타이머3 PWM 설정
  TIM_TimeBaseStructure.TIM_Prescaler = 0;           //(168Mhz/2)/1 = 84MHz
  TIM_TimeBaseStructure.TIM_Period = 1000-1;        //(1000(999+1) /84MHz = 11.904us)
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 500;
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  //타이머 4 입력 캡쳐 설정
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; // 분주비가 1 이면 하강 엣지가 발생할때 마다 입력 캡쳐가 된다.
  TIM_ICInitStructure.TIM_ICFilter = 0x00;
  TIM_ICInit(TIM4, &TIM_ICInitStructure);

  //타이머4와 3을 동작시킨다.
  TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
  TIM_Cmd(TIM3, ENABLE);

  while(1)
  {
    Segment(c_value);
  }
}
