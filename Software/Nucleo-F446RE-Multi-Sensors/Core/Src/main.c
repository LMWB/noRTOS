/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  *
  *
  * Comparing 3 CO2 Sensors interfacing to I2C
  * Sesirion 	@ 0x62
  * CCS811 		@ 0x5A
  * ENS160		@ 0x52
  * AHT21		@ 0x38
  *
  *
  *
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "noRTOS.h"
#include "hardwareGlobal.h"
#include "Drivers/Sensors/CO2/ENS160.h"
#include "Drivers/Sensors/CO2/CCS811.h"
#include "Drivers/Sensors/CO2/SCD4x.h"
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

ccs811_data_t myCCS811 = {};

void blinky(void){
	NUCLEO_LED_toggle();
}

void sample_temp_hum(void){
	printf("Sample temperature and humidity and transmit to ens160 and ccs811\n");
	// do stuff
	printf("done\n");
}

void sample_ens160(void){
	uint16_t eCO2 = 0;
	uint16_t temperature = 0;
	uint16_t rleative_humidity = 0;
	uint8_t status = 0;
	ens160_i2c_read_register(ENS160_DATA_STATUS, &status, 1);
	ens160_i2c_read_register(ENS160_DATA_ECO2, (uint8_t*) &eCO2, 2);
	ens160_i2c_read_register(ENS160_DATA_T, (uint8_t*) &temperature, 2);
	ens160_i2c_read_register(ENS160_DATA_RH, (uint8_t*) &rleative_humidity, 2);

	ens160_translate_status_byte(status);
	printf("ENS160 CO2: %d\n", eCO2);
	printf("ENS160 Temperature: %d\n", temperature);
	printf("ENS160 Humidity: %d\n", rleative_humidity);
}

void sample_ccs811(void){

	ccs811_sample_data(&myCCS811);

}

void sample_scd4x(void){
	bool data_ready_flag = false;
	scd4x_get_data_ready_flag(&data_ready_flag);
	if (!data_ready_flag) {
		return;
	}else{
		uint16_t co2;
		int32_t temperature;
		int32_t humidity;
		scd4x_read_measurement(&co2, &temperature, &humidity);
		printf("SCD4x-CO2: %u\n", co2);
		printf("SCD4x-Temperature: %ld milli Grad C\n", temperature);
		printf("SCD4x-Humidity: %ld mRH\n", humidity);
	}
}

void noRTOS_setup(void) {
	NUCLEO_LED_turn_on();
	scan_i2c_sensors();

	ens160_init();
	ccs811_init();
	sdc4x_init();

	NUCLEO_LED_turn_off();
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
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  noRTOS_task_t blinky_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = blinky};
  noRTOS_add_task_to_scheduler(&blinky_t);

  noRTOS_task_t ens160_t = {.delay = eNORTOS_PERIODE_10s, .task_callback = sample_ens160};
  noRTOS_add_task_to_scheduler(&ens160_t);

  noRTOS_task_t ccs811_t = {.delay = eNORTOS_PERIODE_10s, .task_callback = sample_ccs811};
  noRTOS_add_task_to_scheduler(&ccs811_t);

  noRTOS_task_t scd4x_t = {.delay = eNORTOS_PERIODE_10s, .task_callback = sample_scd4x};
  noRTOS_add_task_to_scheduler(&scd4x_t);

  /* ens160 and ccs811 sensors need to get temperature and humidity informations from external sensor once in a while */
  noRTOS_task_t temp_hum_t = {.delay = eNORTOS_PERIODE_10min, .task_callback = sample_temp_hum};
  noRTOS_add_task_to_scheduler(&temp_hum_t);

  noRTOS_run_scheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
