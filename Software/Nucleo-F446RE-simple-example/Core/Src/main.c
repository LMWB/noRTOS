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
 * This is a simple project on how to use the noRTOS Api. It requires a STM32 Nucleo Board in default configuration.
 * Any Nucleo Board will work since there is no external hardware  involved (shields, extensions etc.).
 * The only components used in this demo are the onboard green LED, blue push button and UART2 as a serial terminal (printf).
 *
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "noRTOS.h"
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
static uint8_t global_button_state = 0;
static bool global_button_event = false;

/* create some callback function you like to run
 * consider each as individual non blocking main()-loops*/
	void read_button_state(void){
		uint8_t button = NUCLEO_BUTTON_READ();
			if (button != global_button_state) {
				global_button_state = button;
				global_button_event = true;
			}
	}

	void blinky(void) {
		NUCLEO_LED_turn_toggle();
	}

	void inter_task_communication(void) {
		if (global_button_event) {
			global_button_event = false;
			if(global_button_state == 1){
				printf("Button got released\n");
			}else if (global_button_state == 0) {
				printf("Button got pressed\n");
			}
		}
	}

	void timing(void) {
		static uint32_t time_stamp_last_call = 0;
		uint32_t now = NORTOS_SCHEDULAR_GET_TICK();
		printf("differnce from now to previous call is %ld ms\n",
				(now - time_stamp_last_call));
		time_stamp_last_call = now;
	}

	void test_callback1(void) {
		printf("Testing printf with _write() override\n\n");
	}

	void test_callback2(void) {
		printf("\tHello World Task 2\n");
	}

	void test_callback3(void) {
		printf("\t\tHello World Task 3\n");
	}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	/* USER CODE BEGIN 2 */


	/* now I create some tasks and add them to the scheduler */
	noRTOS_task_t button = { .delay = eNORTOS_PERIODE_10ms, .task_callback = read_button_state };
	noRTOS_add_task_to_scheduler(&button);

	noRTOS_task_t task_communication = { .delay = eNORTOS_PERIODE_100ms, .task_callback = inter_task_communication };
	noRTOS_add_task_to_scheduler(&task_communication);

	noRTOS_task_t blinky_t = { .delay = eNORTOS_PERIODE_1s, .task_callback = blinky };
	noRTOS_add_task_to_scheduler(&blinky_t);

	noRTOS_task_t task_timing = { .delay = eNORTOS_PERIODE_500ms, .task_callback = timing };
	noRTOS_add_task_to_scheduler(&task_timing);

	noRTOS_task_t test_task1 = { .delay = eNORTOS_PERIODE_1s, .task_callback = test_callback1 };
	noRTOS_add_task_to_scheduler(&test_task1);

	noRTOS_task_t test_task2 = { .delay = eNORTOS_PERIODE_5s, .task_callback = test_callback2 };
	noRTOS_add_task_to_scheduler(&test_task2);

	noRTOS_task_t test_task3 = { .delay = eNORTOS_PERIODE_30s, .task_callback = test_callback3 };
	noRTOS_add_task_to_scheduler(&test_task3);

	/* this runs for ever */
	noRTOS_run_scheduler();

	/* never get here! */

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
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
