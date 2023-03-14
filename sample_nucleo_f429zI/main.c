/******************************************************************************
  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트C (PC0 ~ PC7)를 8핀 케이블를 이용해서
        LED모듈의 LED0 ~ LED7에 연결한다. (LED0가 PC0로 연결되야 한다.)
******************************************************************************/
// stm32f4xx의 각 레지스터들을 정의한 헤더파일
#include "stm32f4xx.h"


// delay 함수
static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (18000* Count); index != 0; index--);
}


int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);


  while(1)
  {
    GPIO_SetBits(GPIOB, GPIO_Pin_0 );  //‘high'를 출력한다
    Delay(500); // 500 ms 지연

   GPIO_ResetBits(GPIOB, GPIO_Pin_0 ); // low'를 출력한다
 //  Delay(500); // 500 ms 지연
   
   GPIO_SetBits(GPIOB, GPIO_Pin_7 );  //‘high'를 출력한다
   Delay(500); // 500 ms 지연

   GPIO_ResetBits(GPIOB, GPIO_Pin_7 ); // low'를 출력한다
 //  Delay(500); // 500 ms 지연
  }
}

