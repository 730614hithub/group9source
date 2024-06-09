#include "esp8266.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "oled.h"
#include "core_json.h"

/*
*************************************
宏定义
*************************************
*/
#define WIFI_SSID        "kjpldzt"
#define WIFI_PASSWD      "111111111"
#define MQTT_CLIENT_ID   "AT+MQTTCLIENTID=0,\"FESA234FBDS24|securemode=3\\,signmethod=hmacsha1\\,timestamp=789|\"\r\n"   
#define MQTT_USER_NAME   "MQTT&k17fimjhRbe"
#define MQTT_PASSWD      "ea726be2c6f4d854ce864746945b5d5b61c49af1"
#define BROKER_ADDRESS "AT+MQTTCONN=0,\"k17fimjhRbe.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883,1\r\n"
#define SUB_TOPIC        "/sys/k17fimjhRbe/MQTT/thing/service/property/set"
#define PUB_TOPIC        "/k17fimjhRbe/MQTT/user/MQTT"
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"
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
/*
*************************************
变量定义
*************************************
*/
unsigned char receive_buf[512];	  //串口2接收缓存数组
unsigned char receive_start = 0;	//串口2接收开始标志位
uint16_t receive_count = 0;	      //串口2接收数据计数器
uint16_t receive_finish = 0;	    //串口2接收结束标志位 

extern uint8_t led_status;
extern uint8_t temp_value;
extern uint8_t humi_value;
/**
  * @brief          Parsing json data
  * @param[in]      json msg:json data,json len:json data length
  * @retval         Return 0 to find the specified json data, otherwise return 1
  */
uint8_t parse_json_msg(uint8_t *json_msg,uint8_t json_len)
{
  uint8_t retval =0;
  
  JSONStatus_t result;
  char query[] = "LightSwitch";
  size_t queryLength = sizeof( query ) - 1;
  char * value;
  size_t valueLength;
  result = JSON_Validate((const char *)json_msg, json_len);
  if( result == JSONSuccess)
  {
    result = JSON_Search((char *)json_msg, json_len, query, queryLength,&value, &valueLength );
    if(result == JSONSuccess)
    {
      char save = value[valueLength];
      value[valueLength] = '\0';
      printf("Found: %s %d-> %s\n", query, valueLength,value);
      value[valueLength] = save;
      led_status = atoi(value);
      retval = 0;

    }
    else
    {
      retval = 1;
    }
  }
  else
  {
    retval = 1;

  }  
  return retval;
}
/**
  * @brief          Serial port 2 Data receiving processing function
  * @param[in]      none
  * @retval         none
  */
void uart2_receiver_handle(void)
{
  unsigned char receive_data = 0;   
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE) != RESET)
  {
    HAL_UART_Receive(&huart2, &receive_data, 1, 1000);//Serial port 2 receives 1-bit data
    receive_buf[receive_count++] = receive_data;
    receive_start = 1;	                              //Serial port 2 receives data start sign position 1
    receive_finish = 0;	                              //Serial port 2 receives data complete flag bit clear 0
  }
}
/**
  * @brief          Serial port 2 data receive clear 0 function
  * @param[in]      len: indicates the length of cleared data
  * @retval         none
  */
void uart2_receiver_clear(uint16_t len)	
{
	memset(receive_buf,0x00,len);
	receive_count = 0;
	receive_start = 0;
	receive_finish = 0;
}
/**
  * @brief          esp8266 send command function
  * @param[in]      cmd: the sent command,len: the length of the command,rec_data: expected to receive data
  * @retval         none
  */
uint8_t esp8266_send_cmd(unsigned char *cmd,unsigned char len,char *rec_data)	
{
  unsigned char retval =0;
  unsigned int count = 0;

  HAL_UART_Transmit(&huart2, cmd, len, 1000);	                                   
  while((receive_start == 0)&&(count<1000))
  {
    count++;
    HAL_Delay(1);
  }

  if(count >= 1000)	
  {
    retval = 1;	
  }
  else	
  {
    do
    {
      receive_finish++;
      HAL_Delay(1);
    }
    while(receive_finish < 500);
    retval = 2;
    if(strstr((const char*)receive_buf, rec_data))	
    {
      retval = 0;	
    }
  }
  uart2_receiver_clear(receive_count);
  return retval;
}
/**
  * @brief          esp8266 config wifi network
  * @param[in]      none
  * @retval         0 is returned if the network configuration is successful. 1 is returned if the network configuration is not successful
  */
uint8_t esp8266_config_network(void)
{
	uint8_t retval =0;
	uint16_t count = 0;
	
	HAL_UART_Transmit(&huart2, (unsigned char *)"AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n",strlen("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n"), 1000);
	
	while((receive_start == 0)&&(count<1000))
	{
		count++;
		HAL_Delay(1);
	}
	
	if(count >= 1000)	
	{
		retval = 1;	
	}
	else
	{
		HAL_Delay(8000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
		}
    else
    {
      retval = 1;
    }
	}
  uart2_receiver_clear(receive_count);
	return retval;
}
/**
  * @brief          esp8266 Connection service
  * @param[in]      none
  * @retval         Return 0 if the connection is successful, or 1 if not
  */
uint8_t esp8266_connect_server(void){
    uint8_t retval=0;
    uint16_t count = 0;
    HAL_UART_Transmit(&huart2, (unsigned char *)BROKER_ADDRESS,strlen(BROKER_ADDRESS), 1000);
    while((receive_start == 0)&&(count<=2000))
    {
        count++;
        HAL_Delay(1);
    }
    if(count >= 2000)
    {
        retval = 1;
    }
    else
    {
        HAL_Delay(5000);
        if(strstr((const char*)receive_buf, "OK"))
        {
            retval = 0;
        }
        else
        {
            retval = 1;
        }
    }
    uart2_receiver_clear(receive_count);
    return retval;
}
/**
  * @brief          The esp8266 reset
  * @param[in]      none
  * @retval         The return 0 is reset successfully, and the return 1 is reset failed
  */
uint8_t esp8266_reset(void)
{
	uint8_t retval =0;
	uint16_t count = 0;
	
	HAL_UART_Transmit(&huart2, (unsigned char *)"AT+RST\r\n",8, 1000);
	while((receive_start == 0)&&(count<2000))	
	{
		count++;
		HAL_Delay(1);
	}
	if(count >= 2000)	
	{
		retval = 1;	
	}
	else	
	{
		HAL_Delay(5000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
		}
    else
    {
      retval = 1;	
    }
	}
  uart2_receiver_clear(receive_count);	
	return retval;
}
/**
  * @brief          The esp8266 sends data
  * @param[in]      none
  * @retval         Return 0 successfully sending data, return 1 failed to send data
  */
uint8_t esp8266_send_msg(void)	
{
	uint8_t retval =0;	
  uint16_t count = 0;			
	static uint8_t error_count=0;
	unsigned char msg_buf[256];
  
  sprintf((char *)msg_buf,"AT+MQTTPUB=0,\""PUB_TOPIC"\",\""JSON_FORMAT"\",0,0\r\n",temp_value,humi_value);
	HAL_UART_Transmit(&huart2, (unsigned char *)msg_buf,strlen((const char *)msg_buf), 1000);	
  HAL_UART_Transmit(&huart1, (unsigned char *)msg_buf,strlen((const char *)msg_buf), 1000);	
	while((receive_start == 0)&&(count<500))	
	{
		count++;
		HAL_Delay(1);
	}
	if(count >= 500)	
	{
		retval = 1;	
	}
	else	
	{
		HAL_Delay(50);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
			error_count=0;
		}
		else 
		{
			error_count++;
			if(error_count==5)
			{
				error_count=0;
        printf("RECONNECT MQTT BROKER!!!\r\n");
				esp8266_init();
			}
		}
	}
  uart2_receiver_clear(receive_count);	
	return retval;
}
/**
  * @brief          The esp8266 receives data
  * @param[in]      none
  * @retval         Return 0 The received data is normal. Return 1 The received data is abnormal or no data is available
  */
uint8_t esp8266_receive_msg(void)	
{
  uint8_t retval =0;
	int msg_len=0;
	uint8_t msg_body[128] = {0};
  
	if(receive_start == 1)	
	{
		do
    {
			receive_finish++;
			HAL_Delay(1);
		}
    while(receive_finish < 5);	
		
		if(strstr((const char*)receive_buf,"+MQTTSUBRECV:"))
		{
			sscanf((const char *)receive_buf,"+MQTTSUBRECV:0,\""SUB_TOPIC"\",%d,%s",&msg_len,msg_body);
      printf("len:%d,msg:%s\r\n",msg_len,msg_body);
			if(strlen((const char*)msg_body)== msg_len)
			{
        retval = parse_json_msg(msg_body,msg_len);
			}
      else
      {
        retval = 1;
      }
		}
    else 
    {
      retval = 1;
    }
	}
  else
  {
    retval = 1;
  }
  uart2_receiver_clear(receive_count);	
  return retval;
}
/**
  * @brief          esp8266初始化
  * @param[in]      none
  * @retval         none
  */
void esp8266_init(void){
    
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);                                                       //Open the serial port. 2 Receiving is interrupted
    printf("1.SETTING STATION MODE\r\n");  OLED_printf(0,0,"1.SETTING STATION MODE                   ");
    while(esp8266_send_cmd((uint8_t *)"AT+CWMODE=1\r\n",strlen("AT+CWMODE=1\r\n"),"OK")!=0)
    {
        HAL_Delay(1000);
    }
    printf("2.CLOSE ESP8266 ECHO\r\n");  OLED_printf(0,0,"2.CLOSE ESP8266 ECHO                     ");
    while(esp8266_send_cmd((uint8_t *)"ATE0\r\n",strlen("ATE0\r\n"),"OK")!=0)
    {
        HAL_Delay(1000);
    }
    printf("3.NO AUTO CONNECT WIFI\r\n");   OLED_printf(0,0,"3.NO AUTO CONNECT WIFI                   ");
    while(esp8266_send_cmd((uint8_t *)"AT+CWAUTOCONN=0\r\n",strlen("AT+CWAUTOCONN=0\r\n"),"OK")!=0)
    {
        HAL_Delay(1000);
    }
    printf("4.RESET ESP8266\r\n");  OLED_printf(0,0,"4.RESET ESP8266                          ");
    while(esp8266_reset() != 0)
    {
        HAL_Delay(5000);
    }
    printf("5.CONFIG WIFI NETWORK\r\n");  OLED_printf(0,0,"5.CONFIG WIFI NETWORK                    ");
    while(esp8266_config_network() != 0)
    {
        HAL_Delay(8000);
    }
    printf("6.MQTT USER CONFIG\r\n");  OLED_printf(0,0,"6.MQTT USER CONFIG                       ");
    while(esp8266_send_cmd((uint8_t *)"AT+MQTTUSERCFG=0,1,\"NULL\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n",
                            strlen("AT+MQTTUSERCFG=0,1,\"NULL\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n"),"OK")!=0)
    {
        HAL_Delay(2000);
    }      
    OLED_printf(0,0,"7.MQTT CLIENTID                       ");
    while(esp8266_send_cmd((uint8_t *)MQTT_CLIENT_ID,strlen(MQTT_CLIENT_ID),"OK")!=0)
    {
        HAL_Delay(2000);
    }
    printf("8.CONNECT MQTT BROKER\r\n");  OLED_printf(0,0,"8.CONNECT MQTT BROKER                    ");
    while(esp8266_connect_server() != 0)
    {
        HAL_Delay(5000);
    }
    printf("9.SUBSCRIBE TOPIC\r\n");  OLED_printf(0,0,"9.SUBSCRIBE TOPIC                ");
    while(esp8266_send_cmd((uint8_t *)"AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n",strlen("AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n"),"OK")!=0)
    {
        HAL_Delay(2000);
    }
    printf("0.ESP8266 INIT OK!!!\r\n");  OLED_printf(0,0,"0.ESP8266 INIT OK!!!                ");
}






























