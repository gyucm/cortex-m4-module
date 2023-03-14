// ÇÉ ¿¬°á
// PA8 :  M_SEN(Motor Sensor)
#include "stm32f4xx.h"
#include "tim1_counter.h"
volatile unsigned int tim1_counter = 0;
void TIM1_Counter_Init()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;     //TIM1_C
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_TRG_COM_TIM11_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  TIM_TIxExternalClockConfig(TIM1,TIM_TIxExternalCLK1Source_TI1,TIM_ICPolarity_Rising,0x03);
  
  TIM_ClearITPendingBit(TIM1, TIM_IT_Trigger);
  TIM_ITConfig(TIM1, TIM_IT_Trigger, ENABLE);
  
  TIM_Cmd(TIM1,ENABLE); 
}

void TIM1_TRG_COM_TIM11_IRQHandler()
{
  if(TIM_GetITStatus(TIM1, TIM_IT_Trigger) != RESET)
  {
    tim1_counter++;
    TIM_ClearITPendingBit(TIM1, TIM_IT_Trigger);
  } 
}