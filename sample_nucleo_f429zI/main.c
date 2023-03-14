/******************************************************************************
  ��Ʈ ����:
    1)  ARM Cortex-M4 ����� ��ƮC (PC0 ~ PC7)�� 8�� ���̺� �̿��ؼ�
        LED����� LED0 ~ LED7�� �����Ѵ�. (LED0�� PC0�� ����Ǿ� �Ѵ�.)
******************************************************************************/
// stm32f4xx�� �� �������͵��� ������ �������
#include "stm32f4xx.h"


// delay �Լ�
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
    GPIO_SetBits(GPIOB, GPIO_Pin_0 );  //��high'�� ����Ѵ�
    Delay(500); // 500 ms ����

   GPIO_ResetBits(GPIOB, GPIO_Pin_0 ); // low'�� ����Ѵ�
 //  Delay(500); // 500 ms ����
   
   GPIO_SetBits(GPIOB, GPIO_Pin_7 );  //��high'�� ����Ѵ�
   Delay(500); // 500 ms ����

   GPIO_ResetBits(GPIOB, GPIO_Pin_7 ); // low'�� ����Ѵ�
 //  Delay(500); // 500 ms ����
  }
}

