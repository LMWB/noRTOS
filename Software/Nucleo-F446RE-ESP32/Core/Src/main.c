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
  *
  *
  *
  *
  * Hardware:
  * smart-hub-100 PCB with Nucleo-F446RE and soldered ESP32
  *
  *
  * Todo
  * - add several sub topics
  * - send 'real' telemetry, currently static string
  *
  *
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
#include "Drivers/Communication/ESP32AT/esp32at.h"
#include "Drivers/Communication/ESP32AT/fifo.h"
#include "math.h"



client_fsm_t esp32_mqtt_client;
WiFi_Client_t esp32Client;     // todo not in use yet, merge with 'client_fsm_t esp32_mqtt_client'
fifo_t esp32_at_cammand;

typedef struct{
	float sin;
	float cos;
	float tan;
	uint32_t time;
	uint32_t date;
}application_output;

application_output app = {0};

void internet_fsm(void){
	esp32_mqtt_fsm(&esp32_mqtt_client);
}

void led_snake(void){
	static uint8_t step = 0;

	switch (step) {
		case 0:
			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET);
			break;
		case 1:
			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_SET);
			break;
		case 2:
			HAL_GPIO_WritePin(	USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_SET);
			break;
		case 3:
			HAL_GPIO_WritePin(	USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_SET);
			break;
		case 4:
			HAL_GPIO_WritePin(	USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_RESET);
			break;
		case 5:
			HAL_GPIO_WritePin(	USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_RESET);
			break;
		case 6:
			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_RESET);
			break;
		case 7:
			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_RESET);
			break;
		default:
			break;
	}
	step ++;
	if(step > 7) step = 0;
}

void heart_beat(void){
	NUCLEO_LED_toggle();
}

void pub_telemetry(void){

	char payload[128];
	uint16_t size = 0;
	size = sprintf(payload, "{\"sin\": %.2f, \"cos\": %.2f, \"tan\": %.2f}", app.sin, app.cos, app.tan);
	printf("Telemetry [%d bytes] Payload: %s\n", size, payload);
	memcpy(esp32_mqtt_client.at_pub_payload, payload, size);
	esp32_mqtt_client.at_pub_payload_size = size;

	if(get_fsm_state(&esp32_mqtt_client) == online){
		set_fsm_state(&esp32_mqtt_client, publish_raw_mqtt_msg);
	}

}

/* just a heave task to keep the cpu busy and test "real time" */
void control_algorithm(void){
	uint32_t now_sec = GET_TICK() / 1000.0;
	float amp = 30.0;
	float f = 1/3600.0; // f = 1/T, T = 1h = 3600sec = 3.600.000 ms
	float max = 70.0;
	float min = -10.0;
	float sin = sinf( 2*M_PI*f * (float)(now_sec)) * amp;
	float cos = cosf( 2*M_PI*f * (float)(now_sec)) * amp;
	float tan = tanf( 2*M_PI*f * (float)(now_sec)) * amp;

	/* catch min/max boundaries */
	if(sin > max) sin = max;
	if(sin < min) sin = min;
	if(cos > max) cos = max;
	if(cos < min) cos = min;
	if(tan > max) tan = max;
	if(tan < min) tan = min;

	app.sin = sin;
	app.cos = cos;
	app.tan = tan;

	printf(" -- Hello World -- \n");
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


// very short buffer since we reading byte wise
uint8_t uart_internet_interrupt_buffer[8];

/* STM32 HAL Based UART Bridge with RX-byte (char) interrupt */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART_INTERNET_INSTANCE){
		// put every byte to uart_rx_buffer and start interrupt again
		fifo_put_byte(&esp32_mqtt_client.at_fifo, uart_internet_interrupt_buffer[0]);

		//uart_increment_pointer();
		UART_INTERNET_READ_BYTE_IRQ( uart_internet_interrupt_buffer );
	}
}

/* STM32 HAL Based UART Bridge with RX-line interrupt */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if (huart->Instance == UART_TERMINAL_INSTANCE){
		// transmit to esp32 what has been received from terminal
		UART_INTERNET_SEND(uart_rx_buffer_terminal, Size);
		UART_TERMINAL_READ_LINE_IRQ( uart_rx_buffer_terminal, uart_rx_buffer_size);
	}
}

void noRTOS_setup(void) {

	esp32Client.wifi_ssid 				= "ssid";
	esp32Client.wifi_password 			= "password";
	esp32Client.mqtt_broker_endpoint 	= "broker.emqx.io";  // EMQX broker endpoint
	esp32Client.mqtt_username 			= "emqx";  			// MQTT username for authentication
	esp32Client.mqtt_password 			= "public";  		// MQTT password for authentication
	esp32Client.mqtt_port 				= "1883";  			// MQTT port (TCP)

	fifo_init(&esp32_at_cammand);
	fifo_clear(&esp32_at_cammand);

	// enable uart RX byte-wise interrupt for ESP32 AT-Command Communication
	//UART_INTERNET_READ_BYTE_IRQ( &uart_rx_buffer_internet[head] );
	UART_INTERNET_READ_BYTE_IRQ( uart_internet_interrupt_buffer );

	// enable uart with DMA interupt for COM port (terminal)
	UART_TERMINAL_READ_LINE_IRQ( uart_rx_buffer_terminal, uart_rx_buffer_size);
	printf("ESP32 Demo\n");
}

/* -------------------------------------------------------------------------------------------------------- */


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

  noRTOS_task_t led_snake_t = {.delay = eNORTOS_PERIODE_200ms, .task_callback = led_snake};
  noRTOS_add_task_to_scheduler(&led_snake_t);

  noRTOS_task_t internet_fsm_t = {.delay = eNORTOS_PERIODE_500ms, .task_callback = internet_fsm};
  noRTOS_add_task_to_scheduler(&internet_fsm_t);

  noRTOS_task_t heartbeat_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = heart_beat};
  noRTOS_add_task_to_scheduler(&heartbeat_t);

  noRTOS_task_t application_t = {.delay = eNORTOS_PERIODE_10s, .task_callback = control_algorithm};
  noRTOS_add_task_to_scheduler(&application_t);

  noRTOS_task_t pub_telemetry_t = {.delay = eNORTOS_PERIODE_30s, .task_callback = pub_telemetry};
  noRTOS_add_task_to_scheduler(&pub_telemetry_t);

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
