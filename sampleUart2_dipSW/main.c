#include "stm32f4xx.h"
#include "uart2.h"
#include <stdio.h>



int main()
{
  unsigned short dipSw=0,dipSwOld=0;
  GPIO_InitTypeDef   GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                             GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; // C 포트 하위 8비트를 출력으로 선언한다.
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|
                                             GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; // C 포트 하위 8비트를 출력으로 선언한다.
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;                                            // A 포트 하위 8비트를 입력으로 선언한다.
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  UART2_init();
  printf("Start main()\n\r");
  
  while(1)
  {
    dipSw = GPIO_ReadInputData(GPIOB)&0xFF00;
    dipSw >>= 8;    
    if(dipSw != dipSwOld)
    {

      printf("Dip : %d %#04x\r\n",dipSw, dipSw);
      GPIO_Write(GPIOC,dipSw);// Dip 스위치 입력을 포트 E의 LED로 직접 출력한다.
      dipSwOld = dipSw;
    }
  }
}

