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
 * Testing DRV8908 Digital Out Driver from TI
 * Setup:
 * Nucleo-F446RE
 * SPI on PA10, PB4, PB5, PB6
 *
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* Control Register */
#define IC_STAT		0x01
#define CONFIG_CTRL 0x07
#define OP_CTRL_1 	0x08
#define OP_CTRL_2 	0x09

/* Bitmask */
#define HB1_LS_EN (1<<0)
#define HB1_HS_EN (1<<1)
#define HB2_LS_EN (1<<2)
#define HB2_HS_EN (1<<3)
#define HB3_LS_EN (1<<4)
#define HB3_HS_EN (1<<5)
#define HB4_LS_EN (1<<6)
#define HB4_HS_EN (1<<7)

#define HB5_LS_EN (1<<0)
#define HB5_HS_EN (1<<1)
#define HB6_LS_EN (1<<2)
#define HB6_HS_EN (1<<3)
#define HB7_LS_EN (1<<4)
#define HB7_HS_EN (1<<5)
#define HB8_LS_EN (1<<6)
#define HB8_HS_EN (1<<7)

// Helper macros for CS control
#define DRV_CS_LOW()   HAL_GPIO_WritePin(SPI1_nCS_GPIO_Port, SPI1_nCS_Pin, 0)
#define DRV_CS_HIGH()  HAL_GPIO_WritePin(SPI1_nCS_GPIO_Port, SPI1_nCS_Pin, 1)

void drv89xxq1_enable_chip_select(void) {
	HAL_GPIO_WritePin(SPI1_nCS_GPIO_Port, SPI1_nCS_Pin, 0);
}

void drv89xxq1_disable_chip_select(void) {
	HAL_GPIO_WritePin(SPI1_nCS_GPIO_Port, SPI1_nCS_Pin, 1);
}

void drv89xxq1_write_control_register(uint8_t register_address, uint8_t data, uint8_t *result) {
	/* compose the write address byte */
	uint16_t address = 0x00;// For a write command (W0 = 0), the response word on the SDO pin is the data currently in the register being written to.
	address |= register_address;
	uint8_t spi_tx_data_frame[2] = { address, data };
	HAL_SPI_TransmitReceive(&hspi1, spi_tx_data_frame, result, 2, 0xffff);
}

void drv89xxq1_read_control_register(uint8_t register_address, uint8_t *result) {
	/* compose the write address byte */
	uint16_t address = 0x40;// For a read command (W0 = 1), the response word is the data currently in the register being read.
	address |= register_address;
	uint8_t spi_tx_data_frame[2] = { address, 0xff};
	HAL_SPI_TransmitReceive(&hspi1, spi_tx_data_frame, result, 2, 0xffff);
}

void drv89xxq1_set_outputs(uint8_t bank, uint8_t output_state_bitmask, uint8_t *result) {
	drv89xxq1_enable_chip_select();
	drv89xxq1_write_control_register(bank, output_state_bitmask, result);
	drv89xxq1_disable_chip_select();
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  uint8_t result[2] = {0};
  drv89xxq1_enable_chip_select();
  drv89xxq1_read_control_register(CONFIG_CTRL, result);
  drv89xxq1_disable_chip_select();

  drv89xxq1_enable_chip_select();
  drv89xxq1_write_control_register(CONFIG_CTRL, 0, result);
  drv89xxq1_disable_chip_select();

  HAL_Delay(500);
  drv89xxq1_enable_chip_select();
  drv89xxq1_read_control_register(IC_STAT, result);
  drv89xxq1_enable_chip_select();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		for (uint16_t i = 0; i < 8; ++i) {
			uint8_t output = (1 << i);
			drv89xxq1_set_outputs(OP_CTRL_1, output, result);
			HAL_Delay(500);
		}

		for (uint16_t i = 1; i < 8; ++i) {
			uint8_t output = (1 << i);
			drv89xxq1_set_outputs(OP_CTRL_2, output, result);
			HAL_Delay(500);
		}
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
	while (1) {
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
