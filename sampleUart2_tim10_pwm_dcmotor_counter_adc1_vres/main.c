#include <stdio.h>
#include "stm32f4xx.h"
#include "uart2.h"
#include "adc1.h"
#include "tim7_fnd.h"

/******************************************************************************
* General-purpose timers TIM10

  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트C (PC0 ~ PC7)를 8핀 케이블를 이용해서
        Array FND모듈의 SA_A ~ SA_H에 연결한다. (SA_A가 PC0로 연결되야 한다.)
    2)  ARM Cortex-M4 모듈의 포트C (PC8 ~ PC11)를 4핀 케이블를 이용해서
        Array FND모듈의 C0 ~ C3에 연결한다. (C0가 PC8로 연결되야 한다.)
    3)  ARM Cortex-M4 모듈의 포트B (PB8)를 2핀 케이블를 이용해서
        LED모듈의 LED0에 연결한다. (LED0가 PB8(TIM10_CH1)로 연결되야 한다.)

    4) PA0 ~ PA7 button0~7

     PB8  ==> DC Motor M_EN
     PB9  ==> DC Motor M_D2
     PB10 ==> DC Motor M_D1

     PA0 ==> VRES
    Key2 ==> Left
    Key3 == > Right
    Key4 ==> Stop

    Key5 ==> PWM - 10%
    Key6 ==> PWM + 10%

    FND ==> Duty rate

******************************************************************************/




int tr_cnt = 0,mr_cnt=0;
int tl_cnt =0 ,ml_cnt = 0;
volatile int pwm = 50;
volatile int counter = 0;
extern uint16_t adc_data;
extern volatile int fndNumber;
extern volatile int adc1Flag;
long map(long x, long in_min, long in_max, long out_min, long out_max);

void TIM1_UP_TIM10_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM10, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM10, TIM_IT_CC1);
    tl_cnt++;
    if(tl_cnt >= 28)   //500ms / 17.7ms = 28.2
    {
      tl_cnt = 0;
      ml_cnt ++ ;       //500ms count
      if(ml_cnt > 99) ml_cnt = 0;
    }  
  }
  if(TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
    tr_cnt++ ;
    if(tr_cnt >= 56)   //1000ms / 17.7ms = 56.4
    {
      tr_cnt = 0;
      mr_cnt ++ ;       //1000ms count
      if(mr_cnt > 99) mr_cnt = 0;
    } 
  }
}

void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_9);
    GPIO_SetBits(GPIOB,GPIO_Pin_10);
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_10);
    GPIO_SetBits(GPIOB,GPIO_Pin_9);
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}
void EXTI3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_10);
    GPIO_ResetBits(GPIOB,GPIO_Pin_9);
    EXTI_ClearITPendingBit(EXTI_Line3);
  }
}
void EXTI4_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line4) != RESET)
  {

    if(pwm >= 10)
      pwm -= 10;
    EXTI_ClearITPendingBit(EXTI_Line4);
  }
}
void EXTI9_5_IRQHandler(void)
{
 
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {
    if(pwm <= 90)
      pwm += 10;
    
    EXTI_ClearITPendingBit(EXTI_Line5);
  }
  else if(EXTI_GetITStatus(EXTI_Line6) != RESET)
  {

    EXTI_ClearITPendingBit(EXTI_Line6);
  }
  else if(EXTI_GetITStatus(EXTI_Line7) != RESET)
  {

    EXTI_ClearITPendingBit(EXTI_Line7);
  }  
}

int main()
{
  int old_pwm=50;
  int ccr1;
  GPIO_InitTypeDef   GPIO_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    
//+ tim1 counter
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
//-

    
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);                          
  
 
//+ tim1 counter
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;             //TIM1_CH1
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
  TIM_TIxExternalClockConfig(TIM1,TIM_TIxExternalCLK1Source_TI1,TIM_ICPolarity_Rising,0x03);
  TIM_Cmd(TIM1,ENABLE); 
//-  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;     // TIM10_CH1
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;     // LED2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM10);

  //외부 인터럽트1이 상승엣지에서 발생되게 설정한다.
  EXTI_InitStructure.EXTI_Line =  EXTI_Line1 |  EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5 |  EXTI_Line6 | EXTI_Line7;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);  
  
   //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
  
  
  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //system_stm32f4xx.c 참조
  //TIM10 input clock (TIM10CLK) is set to 2 * APB2 clock (PCLK2),
  //since APB2 prescaler is different from 1.
  //TIM10CLK = 2 * PCLK2
  //PCLK2 = HCLK / 2  => TIM10CLK = HCLK = SystemCoreClock
  //TIM_CounterMode를 TIM_CounterMode_Up로 설정하면, 카운트값이 TIMx_ARR값과
  //같아질때, Update 이벤트가 발생한다.
  //TIM_Prescaler를 167로, TIM_ClockDivision를 TIM_CKD_DIV1(1분주)로
  //설정하면, 타이머10 카운트 클럭은 (168MHz/1) /(167+1) = 1MHz (1us)이다.
  //TIM_Period를 17699으로 설정했으니(TIMx_ARR), 카운터가 이값과 같아지면,
  //(1us x 17700(17699+1) = 17.7ms)
  
  
  //SYSCLK(168MHz) / AHB Pre(1) == 168Mhz / APB2  Pre(2) == 84Mhz * 2  == 168MHz (Timer Clock) 
  TIM_TimeBaseStructure.TIM_Prescaler = 168-1;         //168Mhz/168 = 1MHz(1us)
  TIM_TimeBaseStructure.TIM_Period = 17700-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_Pulse = 7080-1;         //40% duty
  TIM_OCInitStructure.TIM_Pulse = 8850-1;         //50% duty
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM10, &TIM_OCInitStructure);

  //타이머10을 동작시킨다.
  TIM_ClearITPendingBit(TIM10, TIM_IT_CC1|TIM_IT_Update);
  TIM_ITConfig(TIM10, TIM_IT_CC1|TIM_IT_Update, ENABLE);

  TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM10, ENABLE);

  TIM_Cmd(TIM10, ENABLE);

  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);  //motor stop
  GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);//motor stop
  
  Init_ADC1();
  ADC_SoftwareStartConv(ADC1); 
  
  TIM7_Fnd_Init();
  int pwmCount = 0;
  while(1)
  {
    
//    pwm = (int)((adc_data/4096.0) * 100);
    if(adc1Flag)
    {
      pwm = map(adc_data,0,4095,0,100);
      adc1Flag = 0;
    }
    if(pwm != old_pwm)
    {
//        ccr1 = pwm;
        if(pwm == 0)          
             ccr1 = 1;
        else if(pwm == 100)
             ccr1 = 177 * 100 - 1;
        else
             ccr1 = 177 * pwm;
        
        TIM10->CCR1 = ccr1;
        old_pwm = pwm;
    }

    
//    LSegment(pwm);
    pwmCount = pwm * 100;
    
    counter = TIM_GetCounter(TIM1);
    if(counter >= 100)
      counter = counter % 100;
    
//    RSegment(counter);
    fndNumber = pwmCount + counter;
  }
}
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}