#include "stm32f4xx.h"
#include "fnd.h"


// 7-세그먼트 폰트를 배열로 지정한다.
unsigned char Font[19] = {0x3F, 0X06, 0X5B, 0X4F, 
                                         0X66, 0X6D, 0X7C, 0X07,
                                         0X7F, 0X67, 0X77, 0X7C, 
                                         0X39, 0X5E, 0X79, 0X71, 
                                         0X08, 0X80, 0x40};

void FND_init()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                             GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
                                             GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

}

void display_fnd( int N )  // Segment 함수 선언
{
  int  i ;
  unsigned char arrayNum[4];
  int Buff ;

  if(N < 0)
  {
    N=-N;
    arrayNum[0] = 18;
  }
  else
    arrayNum[0] = N /1000;  // 세그먼트에서 사용하는 천의 자리를 추출
  
  Buff = N % 1000 ;
  arrayNum[1] = Buff / 100 ; // 세그먼트에서 사용하는 백의자리 추출
  Buff = Buff % 100;
  arrayNum[2] = Buff /10 ;     // 세그먼트에서 사용하는 십의 자리 추출
  arrayNum[3] =  Buff % 10 ;    // 세그먼트에서 사용하는 일의 자리 추출    

  for( i=0;i<4;i++)
      display_digit(i,arrayNum[i]);
}
static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}
void display_digit(int positon,int number)
{
   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) | (0x0f00)));  //LED all off 
   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) &  ~( 0x0100 << positon)));
   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) &  0xff00 ) | Font[number]);
   Delay(1);
}
