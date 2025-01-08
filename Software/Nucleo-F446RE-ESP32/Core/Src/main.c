/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "noRTOS.h"
#include "hardwareGlobal.h"

#define __1KByte__ 1024
#define uart_rx_buffer_size (__1KByte__)
static uint8_t uart_rx_buffer_internet[uart_rx_buffer_size];
volatile uint16_t head = 0;
volatile uint16_t tail = 0;
static uint8_t uart_rx_buffer_terminal[uart_rx_buffer_size];

typedef struct __WiFi_Client__{
	const char* wifi_ssid;
	const char* wifi_password;
	const char* mqtt_broker_endpoint;
	const char* mqtt_username;
	const char* mqtt_password;
	const char* mqtt_port;
}WiFi_Client_t;

WiFi_Client_t esp32Client;

/* Constants to be used in ESP32 MQTT State Machine */
#define String char*
String ESP32_RESET					= "AT+RST\r\n";
String ESP32_QUERY_WIFI_STATE		= "AT+CWSTATE?\r\n";
String ESP32_QUERY_WIFI_CONNECTION	= "AT+CWJAP?\r\n";
String ESP32_SET_AP_CONNECTION 		= "AT+CWJAP=\"VodafoneMobileWiFi-6B18C9\",\"wGkH536785\"\r\n";
String ES32_QUERY_IP 				= "AT+CIFSR\r\n";

String ESP32_PING 					= "AT+PING=\"www.google.de\"\r\n";

String ESP32_SET_SNTP_TIME 			= "AT+CIPSNTPCFG=1,0,\"zeit.fu-berlin.de\"\r\n";
String ESP32_QUERY_TIMESTAMP 		= "AT+SYSTIMESTAMP?\r\n";
String ESP32_QUERY_SNTP_TIME 		= "AT+CIPSNTPTIME?\r\n";

String ESP32_SET_MQTT_CONFIG 		= "AT+MQTTUSERCFG=0,1,\"sc36ClientID\",\"emqx\",\"public\",0,0,\"\"\r\n";
String ESP32_QUERY_MQTT_CONNECTION 	= "AT+MQTTCONN?\r\n";
String ESP32_SET_MQTT_CONNECTION 	= "AT+MQTTCONN=0,\"broker.emqx.io\",1883,1\r\n";
String ESP32_CLOSE_MQTT_CONNECTION 	= "AT+MQTTCLEAN=0\r\n";

String ESP32_PUB_MQTT 				= "AT+MQTTPUB=0,\"topic008\",\"hallo broker\",1,0\r\n";
String ESP32_PUB_RAW_MQTT 			= "AT+MQTTPUBRAW=<LinkID>,<\"topic\">,<length>,<qos>,<retain>\r\n";
String ESP32_QUERY_SUB_MQTT 		= "AT+MQTTSUB?\r\n";
String ESP32_SET_SUB_MQTT 			= "AT+MQTTSUB=0,\"topic007\",1\r\n";

/* MQTT Connection Example with web based client
 * https://mqttx.app/web-client#/recent_connections/8dc002fe-dc48-4926-8760-bde4bbb4c859
 * https://www.emqx.com/en/blog/mqtt-client-tools
 * */

// AT+MQTTUSERCFG=0,7,"sc36ClientID","emqx","public",0,0,"mqtt" (uses WSS scheme 7: MQTT over WebSocket Secure (based on TLS, no certificate verify).
// AT+MQTTCONN=0,"broker.emqx.io",8084,1
// AT+MQTTSUB=0,"topic007",1
// AT+MQTTPUB=0,"topic008","hallo broker",1,0

/* another MQTT Connection Example with web based client */
// AT+MQTTUSERCFG=0,1,"sc36ClientID","emqx","public",0,0,"" (uses TCP sechme 1: MQTT over TCP)
// AT+MQTTCONN=0,"broker.emqx.io",1883,1
// AT+MQTTSUB=0,"topic007",1
// AT+MQTTPUB=0,"topic008","hallo broker",1,0

void esp32_reset(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_RESET, strlen(ESP32_RESET));
}

void esp32_check_wifi_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_QUERY_WIFI_CONNECTION, strlen(ESP32_QUERY_WIFI_CONNECTION) );
}

void esp32_connect_to_wifi(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_AP_CONNECTION, strlen(ESP32_SET_AP_CONNECTION));
}

void esp32_connect_to_sntp(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_SNTP_TIME, strlen(ESP32_SET_SNTP_TIME));
}

void esp32_config_mqtt_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONFIG, strlen(ESP32_SET_MQTT_CONFIG));
}

void esp32_connect_to_mqtt_broker(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONNECTION, strlen(ESP32_SET_MQTT_CONNECTION));
}

void esp32_subscribe_to_topic(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_SUB_MQTT, strlen(ESP32_SET_SUB_MQTT));
}
void esp32_publish_to_topic(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_PUB_MQTT, strlen(ESP32_PUB_MQTT));
}

typedef enum{
	 esp32_error = 0,
	 esp32_ok,
	 esp32_timeout,
	 esp32_idle,
}esp32_exit_code_t;

typedef enum{
	undefined = 0,
	standby,					// 1
	boot_up,					// 2
	connect_to_wifi,			// 3
	config_connection_to_broker,// 4
	connect_to_mqtt_broker,		// 5
	subscribe_to_mqtt_msg,		// 6
	online,						// 7
	publish_mqtt_msg,			// 8
	wait_for_response,			// 9
}MQTT_client_t;

typedef struct {
	/* public attributes */
	uint32_t timeout;

	/* private attributes */
	char at_response_buffer[512];
	char at_response_to_be[128];
	uint32_t timeout_start;
	MQTT_client_t next_state;	// next state aiming to after at_response success (nothing to do with state machine new_state)
}client_fsm_t;

void esp32_set_needle_for_response(client_fsm_t* client, String at_response, uint32_t timeout){
	memcpy(client->at_response_to_be, at_response, strlen(at_response));
	client->timeout = timeout;
}

void esp32_save_at_response_to_client(client_fsm_t* client ){
	client->timeout_start = GET_TICK();
	uint16_t size = head - tail;
	// todo some how this needs to be kind of fifo buffer as well, since some ESP32 responses have multiple line which will be get overwritten
	memcpy(client->at_response_buffer, &uart_rx_buffer_internet[tail], size);
}

esp32_exit_code_t esp32_wait_for_response(client_fsm_t* client){
	uint32_t tic = GET_TICK();

	// if timeout got hit
	if( (tic - client->timeout_start) >= client->timeout){
		printf("\t[debug] - timeout\n");
		return esp32_timeout;
	}

	// check if needle was found
	char *ret = strstr(client->at_response_buffer, client->at_response_to_be);
	if( ret !=  NULL ){
		memset(client->at_response_to_be, '\0', 128);
		memset(client->at_response_buffer, '\0', 512);
		printf("\t[debug] - hit needle\n");
		return esp32_ok;
	}

	// if at phrase ERROR was found
	ret = strstr(client->at_response_buffer, "ERROR\r\n");
	if( ret != NULL){
		memset(client->at_response_to_be, '\0', 128);
		memset(client->at_response_buffer, '\0', 512);
		printf("\t[debug] - hit error\n");
		return esp32_error;
	}

	// else wait for next call
	return esp32_idle;
}


void esp32_mqtt_fsm(client_fsm_t *client) {
	static MQTT_client_t state = boot_up;
	MQTT_client_t new_state = standby;

	switch (state) {
	case undefined:
		new_state = boot_up;
		break;

	case standby:
		new_state = standby;
		break;

	case boot_up:
		esp32_reset();
		client->next_state = connect_to_wifi;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case connect_to_wifi:
		esp32_connect_to_wifi();
		client->next_state = config_connection_to_broker;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "WIFI GOT IP\r\n\r\nOK\r\n", 0xFFFF);
		break;

	case config_connection_to_broker:
		esp32_config_mqtt_connection();
		client->next_state = connect_to_mqtt_broker;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case connect_to_mqtt_broker:
		esp32_connect_to_mqtt_broker();
		client->next_state = subscribe_to_mqtt_msg;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case subscribe_to_mqtt_msg:
		esp32_subscribe_to_topic();
		client->next_state = online;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 5000);
		break;

	case online:
		client->next_state = online;
		new_state = online;
		break;

	case publish_mqtt_msg:
		esp32_publish_to_topic();
		new_state = online;
		break;

	case wait_for_response:
		esp32_exit_code_t code = esp32_wait_for_response(client);
		if( code == esp32_ok){
			new_state = client->next_state;
		}else if ( code == esp32_timeout) {
			new_state = boot_up;
		}else if( code == esp32_error ){
			new_state = state;
		}else{
			new_state = state;
		}
		break;

	default:
		break;
	}

	if (new_state != state) {
		state = new_state;
	}
}

client_fsm_t esp32_mqtt_client;

void test_fsm(void){
	esp32_mqtt_fsm(&esp32_mqtt_client);
}


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void uart_increment_pointer(void){
	head += 1;
	if(head >= uart_rx_buffer_size){
		head = 0;
	}
}

void uart_clear_fifo(void){
	uint16_t size = head - tail;
	esp32_save_at_response_to_client(&esp32_mqtt_client);
	memset(&uart_rx_buffer_internet[tail], '\0', size);
	head = 0;
	tail = 0;
	UART_INTERNET_ABORT_IRQ();
	UART_INTERNET_READ_BYTE_IRQ( &uart_rx_buffer_internet[head] );
}

void uart_rx_complete_callback(void){
	uint16_t size = head - tail;
	if( (size > 3) && (uart_rx_buffer_internet[head-1] == '\n') && (uart_rx_buffer_internet[head-2] == '\r') ){
		UART_TERMINAL_SEND( &uart_rx_buffer_internet[tail], size);
		uart_clear_fifo();
	}
}

void noRTOS_setup(void) {

	esp32Client.wifi_ssid 		= "VodafoneMobileWiFi-6B18C9";
	esp32Client.wifi_password 	= "wGkH536785";
	esp32Client.mqtt_broker_endpoint 	= "broker.emqx.io";  // EMQX broker endpoint
	esp32Client.mqtt_username 			= "emqx";  			// MQTT username for authentication
	esp32Client.mqtt_password 			= "public";  		// MQTT password for authentication
	esp32Client.mqtt_port 				= "1883";  			// MQTT port (TCP)

	// enable uart RX byte-wise interrupt for ESP32 AT-Command Communication
	UART_INTERNET_READ_BYTE_IRQ( &uart_rx_buffer_internet[head] );

	// enable uart with DMA interupt for COM port (terminal)
	UART_TERMINAL_READ_LINE_IRQ( uart_rx_buffer_terminal, uart_rx_buffer_size);
	printf("ESP32 Demo\n");
}

/* -------------------------------------------------------------------------------------------------------- */

/* STM32 HAL Based UART Bridge with RX-byte (char) interrupt */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART_TERMINAL_INSTANCE){
	}

	if (huart->Instance == UART_INTERNET_INSTANCE){
		// put every byte to uart_rx_buffer and start interrupt again
		uart_increment_pointer();
		UART_INTERNET_READ_BYTE_IRQ( &uart_rx_buffer_internet[head] );
	}
}

/* STM32 HAL Based UART Bridge with RX-line interrupt */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if (huart->Instance == UART_TERMINAL_INSTANCE){
		// transmit to esp32 what has been received from terminal
		UART_INTERNET_SEND(uart_rx_buffer_terminal, Size);
		UART_TERMINAL_READ_LINE_IRQ( uart_rx_buffer_terminal, uart_rx_buffer_size);
	}

	if (huart->Instance == UART_INTERNET_INSTANCE){
		// transmit to terminal what has been received from esp32
		UART_TERMINAL_SEND(uart_rx_buffer_internet, Size);
		UART_INTERNET_READ_LINE_IRQ( uart_rx_buffer_internet, uart_rx_buffer_size );
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  noRTOS_task_t uart_calback_t = {.delay = eNORTOS_PERIODE_100ms, .task_callback = uart_rx_complete_callback};
  noRTOS_add_task_to_scheduler(&uart_calback_t);

  noRTOS_task_t check_connection_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = test_fsm};
  noRTOS_add_task_to_scheduler(&check_connection_t);

  noRTOS_run_scheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  printf("You should not see this message!\n");
	  DELAY(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
