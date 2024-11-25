/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app.h"
#include "simpleOS.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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
  MX_ADC1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

#define DELAY_TIME_MS 2

  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET); // LED off
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); // LED off
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // LED off
  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET); // LED off

  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET); // Sound off


  /* Reset 7 Segment Display */
  mf_reset_segment();
  HAL_Delay(DELAY_TIME_MS);
  //mf_demo_segment();

  simpleOS_init();

//  BEEP_ON();
//  HAL_Delay(200);
//  BEEP_OFF();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  simpleOS_demo();
	  HAL_Delay(DELAY_TIME_MS);

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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint8_t cnt = 0;

	switch (GPIO_Pin) {

	case S3_Pin:
		//HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		break;

	case S1_Pin:
		//HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		mf_write_to_display_memory(1, mf_uint_to_hex(cnt));
		cnt+=1;
		if(cnt>=16)
		{
			cnt = 0;
		}
		break;

	case S2_Pin:
		//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		break;

	case B1_Pin:
		break;

	default:
		break;
	}
}

void simpleOS_callback_01(void) {
//	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET); // LED on
//	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
//	HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	//LED1_TOGGLE();
	mf_refresh_segment();
}

void simpleOS_callback_05(void) {

	// reset timer 6 to 0
	__HAL_TIM_SET_COUNTER(&htim6, 0);

	// start timer 6 for executing measurement
	HAL_TIM_Base_Start(&htim6);

	// execute task
	app_FSM_SEGMENT_1_counter();
	for (uint_fast32_t i=0; i<1e3; i++) __NOP();

	// stop timer 6 for executing measurement
	HAL_TIM_Base_Stop(&htim6);

	periodic_task_set_exe_time_us(__HAL_TIM_GET_COUNTER(&htim6), pTask5_nmb);
}

void simpleOS_callback_06(void)
{
	simpleOS_measure_exe_time_tic(pTask6_nmb);

	app_FSM_SEGMENT_2_counter();

	simpleOS_measure_exe_time_toc(pTask6_nmb);
}

void simpleOS_callback_07(void) {
	//app_FSM_LED_bin_cunter();
	app_FSM_LED_snake();
	//app_FSM_LED_knightrider();
}

static volatile float var1 = 0.0;
static volatile float var2 = 0.0;
static volatile float var3 = 0.0;
static volatile float var4 = 0.0;

void simpleOS_callback_08(void) {
	// do some heavy calculation for testing

	simpleOS_measure_exe_time_tic(pTask8_nmb);

	const float a = -7.3496;
	const float b = 20.86;
	static float count = 0.000;
	float x = count;

	var1 = powf(3.14159, count);
	var2 = powf(2.0, count);
	var3 = a*x + b;
	count = count + 0.001;

	simpleOS_measure_exe_time_toc(pTask8_nmb);
}

void simpleOS_callback_09(void) {

	// report infor to console

	char buf[64];
	uint16_t sz = strlen(buf);

	sz = sprintf(buf, "task 1 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask1_nmb), periodic_task_get_cyc_time_us(pTask1_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "task 5 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask5_nmb), periodic_task_get_cyc_time_us(pTask5_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "task 6 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask6_nmb), periodic_task_get_cyc_time_us(pTask6_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "task 7 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask7_nmb), periodic_task_get_cyc_time_us(pTask7_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "task 8 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask8_nmb), periodic_task_get_cyc_time_us(pTask8_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "task 9 exe time: %ld us cyc time: %ld us\r\n", periodic_task_get_exe_time_us(pTask9_nmb), periodic_task_get_cyc_time_us(pTask9_nmb));
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	sz = sprintf(buf, "var1: %6.3f var2: %6.3f var3 %6.3f\r\n", var1, var2, var3);
	HAL_UART_Transmit(&huart2, (uint8_t*)buf, sz, 10);

	HAL_UART_Transmit(&huart2, (uint8_t*)"----------\r\n", 12, 10);
}

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
