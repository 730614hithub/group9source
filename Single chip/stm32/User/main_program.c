#include "main_program.h"
#include <stdio.h>
#include "time_handle.h"
#include "oled.h"
#include "dht11.h"
#include "esp8266.h"
#include "led.h"

/*
*************************************
宏定义
*************************************
*/

/*
*************************************
变量声明
*************************************
*/
extern uint16_t index_oled;
extern uint16_t index_dht11;
extern uint8_t index_send_msg;
extern uint8_t index_led;
/*
*************************************
函数声明
*************************************
*/

/*
*************************************
变量定义
*************************************
*/
uint8_t led_status =0;
uint8_t temp_value =0;
uint8_t humi_value =0;
/*
*************************************
函数定义
*************************************
*/

/**
  * @brief          The initialization function, equivalent to Arduino's setup() function, is executed only once at initialization
  * @param[in]      none
  * @retval         none
  */
void user_init_program(void)
{
  OLED_init(); 
  esp8266_init();
  while(dht11_init())
  {
    HAL_Delay(500);
    printf("dht11 init faild\r\n");
    OLED_printf (0,0,"dht11 init faild");
  }
  OLED_printf (0,0,"  Alibaba Cloud IOT ");
  time_slot_start();
}
/**
  * @brief          The initialization function, equivalent to Arduino's loop() function, is always executed
  * @param[in]      none
  * @retval         none
  */
void user_main_program(void)
{
  if(esp8266_receive_msg()==1){}
  else printf("msg receive error\r\n");
  if(index_dht11 ==1)
  {
    dht11_read_data(&temp_value,&humi_value);
    index_dht11=0;
  }
  if(index_led==1)
  {
    set_led(led_status);
    index_led=0;
  }
  if(index_oled==1)
  {
    OLED_printf (2,0,"temp:%d",temp_value);
    OLED_printf (4,0,"humi:%d",humi_value);
    index_oled=0;
  }
  if(index_send_msg==1)
	{
		index_send_msg =0;
		if(esp8266_send_msg()==0){}
		else printf("msg send error\r\n");
	}
    
   if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)==1)HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
   else HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
}

