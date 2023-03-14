#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "uart2.h"
#include "uart4.h"
#include "adc1.h"
#include "tim7_fnd.h"
#include "tim10_motor.h"
#include "tim1_counter.h"
#include "key.h"

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

volatile int pwm = 50;
volatile int counter = 0;
extern uint16_t adc_data;
extern volatile int fndNumber;
extern volatile int adc1Flag;
extern volatile unsigned long systick_sec;            //1sec
extern volatile int systick_secFlag;
extern volatile unsigned char rx2Flag;
extern char rx2Data[50];
extern volatile unsigned char rx4Flag;
extern char rx4Data[50];
extern int key;
extern unsigned int tim1_counter;
long map(long x, long in_min, long in_max, long out_min, long out_max);

int main()
{
  int old_pwm=50;
  int adc_pwm=50;
  int ccr1;
  int pwmCount = 0;
  
  Key_Init();
  TIM7_Fnd_Init();
  UART2_init();
  UART4_init();
  TIM10_Motor_Init();
  TIM1_Counter_Init();
  Init_ADC1();
  ADC_SoftwareStartConv(ADC1); 
  
  Serial2_Send_String("start main()\r\n");
  pwmCount = pwm * 100;
  while(1)
  {
    if(rx2Flag)
    {
      printf("rxData : %s\r\n",rx2Data);
      rx2Flag = 0;
    }  
    if(rx4Flag)
    {
      Serial4_Send_String(rx4Data);
      rx4Flag = 0;
    }
    
    if(key != 0)
    {
      printf("Key : %d  \r\n",key);
      if(key == 1) //motor right
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_SET); 
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);
      }
      else if(key == 2) //motor left
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET); 
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_SET);        
      }
      else if(key == 3) //motor stop 
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);  
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);       
      }
      key = 0;
    } 
    
    if(systick_secFlag)
    {
      ADC_SoftwareStartConv(ADC1); 
      systick_secFlag = 0;
    }
    
    if(adc1Flag)
    {  
//    adc_pwm = (int)((adc_data/4096.0) * 100);      
      adc_pwm = map(adc_data,0,4095,0,100);
      if(abs(adc_pwm - old_pwm)>=5)
          pwm = adc_pwm;
      adc1Flag = 0;
    }

    if(pwm != old_pwm  )
    {
        if(pwm == 0)          
             ccr1 = 1;
        else if(pwm == 100)
             ccr1 = 177 * 100 - 1;
        else
             ccr1 = 177 * pwm;
        
        TIM10->CCR1 = ccr1;
        old_pwm = pwm;
        pwmCount = pwm * 100;
    }

//    counter = TIM_GetCounter(TIM1);
//    if(counter >= 100)
//      counter = counter % 100;

    fndNumber = pwmCount + tim1_counter%100;
  }
}
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}