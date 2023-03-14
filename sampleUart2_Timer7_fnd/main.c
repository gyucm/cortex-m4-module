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
#include <stdlib.h>
#include "tim7_fnd.h"


extern volatile int fndNumber;
extern volatile unsigned char rx2Flag;
extern char rx2Data[50];
int main()
{

  UART2_init();
  TIM7_Fnd_Init();
  fndNumber = 1234;
  printf("start main()\r\n");
  while(1)
  {
      if(rx2Flag)
      {
          fndNumber = atoi(rx2Data);
          rx2Flag = 0;
      }
  }
}
