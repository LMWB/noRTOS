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
#include "Drivers/Communication/ESP32AT/esp32at.h"
#include "Drivers/Communication/ESP32AT/fifo.h"




client_fsm_t esp32_mqtt_client;
WiFi_Client_t esp32Client;
fifo_t esp32_at_cammand;

char* AT_Pointer;

void internet_fsm(void){
	esp32_mqtt_fsm(&esp32_mqtt_client);
}

void heart_beat(void){
	NUCLEO_LED_toggle();
}

void pub_telemetry(void){

	if(get_fsm_state(&esp32_mqtt_client) == online){
		set_fsm_state(&esp32_mqtt_client, publish_mqtt_msg);
	}

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

	// testing for debugging to read buffer content, but still not that great
	AT_Pointer = esp32_at_cammand.buffer;

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

  noRTOS_task_t internet_fsm_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = internet_fsm};
  noRTOS_add_task_to_scheduler(&internet_fsm_t);

  noRTOS_task_t heartbeat_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = heart_beat};
  noRTOS_add_task_to_scheduler(&heartbeat_t);

  noRTOS_task_t pub_telemetry_t = {.delay = eNORTOS_PERIODE_10s, .task_callback = pub_telemetry};
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
