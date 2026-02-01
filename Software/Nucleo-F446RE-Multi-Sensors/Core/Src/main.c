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
#include "platformGlue.h"

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
#define ENS160_I2C_ADDRESS	0x52
#define ENS160_OPMODE 		0x10	// Operating mode:0x00: DEEP SLEEP mode (low power standby); 0x01: IDLE mode (low-power); 0x02: STANDARD Gas Sensing Modes

#define ENS160_TEMP_IN 		0x13	// 2 Bytes Length
#define ENS160_RH_IN 		0x15	// 2 Bytes Length
#define ENS160_DATA_STATUS	0x20
#define ENS160_DATA_ECO2	0x24 	// 2 Bytes Length
#define ENS160_DATA_T 		0x30	// 2 Bytes Length
#define ENS160_DATA_RH 		0x32	// 2 Bytes Length


void ens160_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Read(&I2C_HANDLER, ENS160_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

void ens160_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Write(&I2C_HANDLER, ENS160_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

void ens160_translate_status_byte(uint8_t status_byte){

	uint8_t NEWGPR;
	uint8_t NEWDAT;
	uint8_t VALIDITY_FLAG;
	uint8_t STATER;
	uint8_t STATAS;
	NEWGPR 			= (status_byte & 0x01);		 // High indicates that a new data is available in the GPR_READx registers. Cleared automatically at first GPR_READx read.
	NEWDAT 			= (status_byte & 0x02) >> 1; // High indicates that a new data is available in the DATA_x registers. Cleared automatically at first DATA_x read.
	VALIDITY_FLAG 	= (status_byte & 0x0C) >> 2; // 0: Normal operation; 1: Warm-Up phase; 2: Initial Start-Up phase; 3: Invalid output
	STATER 			= (status_byte & 0x40) >> 6; // High indicates that an error is detected. E.g. Invali Operating Mode has been selected.
	STATAS 			= (status_byte & 0x80) >> 7; // High indicates that an OPMODE is running
	printf("ENS160 Status: ");
	if(NEWGPR) printf("GPR data available, ");
	if(NEWDAT) printf("Sensor data available, ");
	if(VALIDITY_FLAG) printf("Operation: %d, ", VALIDITY_FLAG);
	if(STATER) printf("Error is present, ");
	if(STATAS) printf("OPMODE is running");
	printf("\n");
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void blinky(void){
	NUCLEO_LED_toggle();
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

void noRTOS_setup(void) {
	NUCLEO_LED_turn_on();
	scan_i2c_sensors();

	// set ENS160 to OPERATIONAL MODE 0x01
	uint8_t mode = 0x01;
	ens160_i2c_write_register(ENS160_OPMODE, &mode, 1);

	uint16_t temp_in = 0x488A; // (17+273.15)*64 = 18569.6 = 0x488A; For 25°C the input value is calculated as follows: (25 + 273.15) * 64 = 0x4A8A.
	uint8_t temp_in_lsb = 0x8A;
	uint8_t temp_in_msb = 0x48;
	ens160_i2c_write_register(ENS160_TEMP_IN, &temp_in_lsb, 1);
	ens160_i2c_write_register(ENS160_TEMP_IN, &temp_in_msb, 1);

	// set ENS160 to OPERATIONAL MODE 0x02
	mode = 0x02;
	ens160_i2c_write_register(ENS160_OPMODE, &mode, 1);
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

  noRTOS_task_t ens160_t = {.delay = eNORTOS_PERIODE_1s, .task_callback = sample_ens160};
  noRTOS_add_task_to_scheduler(&ens160_t);

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
