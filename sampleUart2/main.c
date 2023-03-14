#include "stm32f4xx.h"
#include "uart2.h"
#include <stdio.h>


static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}


int main()
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                             GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; // C 포트 하위 8비트를 출력으로 선언한다.
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  UART2_init();
  printf("Start main()\n\r");
  printf("float : %f\n\r",3.14);
  while(1)
  {
     
     GPIO_WriteBit(GPIOC,GPIO_Pin_2,Bit_SET);
     Delay(500);
     GPIO_WriteBit(GPIOC,GPIO_Pin_2,Bit_RESET);
     Delay(500);
  }
}
