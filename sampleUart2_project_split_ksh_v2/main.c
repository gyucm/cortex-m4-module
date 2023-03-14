#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "uart2.h"
#include "uart4.h"
#include "adc1.h"
#include "tim7_fnd.h"
#include "tim10_motor.h"
#include "tim1_counter.h"
#include "key.h"
#include "lcd.h"
#include "led.h"


/******************************************************************************
* General-purpose timers TIM10

  포트 연결:

  PA0,PA1 : UART4 RX,TX
  PA2,PA3 : UART2 RX,TX
//  PA8 :  M_SEN(Motor Sensor)

  PA5 ~ PA12 : CLCD D0~D7
  PB0 ~ PB2  : CLCD RS, RW, E

   PB8  : DC Motor M_EN
   PB9  : DC Motor M_D2
   PB10 : DC Motor M_D1

   PC0~PC11 ==> FND A~H, C0~C3
   PC12~PC15 ==> Button0 ~ 3 

   FND 출력값
   FND12 ==> PWM
   FND34 ==> Duty rate

******************************************************************************/
#define CMD_SIZE 50
#define ARR_CNT 5  
//#define PORTC_FND 

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

#ifdef PORTC_FND
  TIM7_Fnd_Init();
#else
   PORTC_Led_Init();  
#endif  
  UART2_init();
  UART4_init();
  TIM10_Motor_Init();
  TIM1_Counter_Init();        //PORTA  사용

  Init_ADC1();
  ADC_SoftwareStartConv(ADC1); 
  
  Serial2_Send_String("start main()\r\n");
  pwmCount = pwm * 100;
  while(1)
  {
    if(rx2Flag)
    {
      printf("rx2Data : %s\r\n",rx2Data);
//      Serial4_Send_String(rx2Data);
//      Serial4_Send_String("\r");
      rx2Flag = 0;
    }  
    if(rx4Flag)
    {
      Serial4_Event();
      rx4Flag = 0;
    }
    
    if(key != 0)
    {
      printf("Key : %d  \r\n",key);
      if(key == 1) //motor right
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_SET); 
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);          
//          GPIO_SetBits(GPIOC,GPIO_Pin_0);
      }
      else if(key == 2) //motor left
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET); 
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_SET);  
//          GPIO_SetBits(GPIOC,GPIO_Pin_1);
      }
      else if(key == 3) //motor stop 
      {
          GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);  
          GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET); 
//          GPIO_SetBits(GPIOC,GPIO_Pin_2);
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

#ifdef PORTC_FND
    fndNumber = pwmCount + tim1_counter%100;
#endif  

  }
}

void Serial4_Event()
{
  
//  Serial2_Send_String(rx4Data);
//  Serial2_Send_String("\r\n");

  int i=0;
  int num = 0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0};
  char sendBuf[CMD_SIZE]={0}; 
  strcpy(recvBuf,rx4Data);
  i=0; 
//  Serial2_Send_String(recvBuf);
//  Serial2_Send_String("\n\r");
  printf("rx4Data : %s\r\n",recvBuf);
     
  pToken = strtok(recvBuf,"[@]");

  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
       
  if(!strcmp(pArray[1],"LED"))
  {
    if(pArray[3] != NULL)              
      num = atoi(pArray[3]);
    else
      num = 0;
    if(!strcmp(pArray[2],"ON"))
    {
     GPIO_SetBits(GPIOC, 0x01 << num); 
    }
    else 
    {
      GPIO_ResetBits(GPIOC, 0x01 << num); 
    }
  } 
  else if(!strncmp(pArray[1]," New conn",sizeof(" New conn")))
  {
      return;
  }
  else if(!strncmp(pArray[1]," Already log",sizeof(" Already log")))
  {
      return;
  }    
  sprintf(sendBuf,"[%s]%s@%s@%d\n",pArray[0],pArray[1],pArray[2],num);
  Serial4_Send_String(sendBuf);
}


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}