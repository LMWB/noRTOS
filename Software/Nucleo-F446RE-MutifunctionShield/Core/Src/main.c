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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "noRTOS.h"
#include "BSP/multifunc.h"
static uint8_t global_button_state = 0;
static bool global_button_event = false;
static uint32_t global_adc_value = 0;
static bool global_adc_event = false;
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

void noRTOS_setup(void) {
	mf_reset_display();
	HAL_ADC_Start_DMA(&hadc1, &global_adc_value, 1);
	mf_write_char_to_display_segment(0, '1');
	mf_write_char_to_display_segment(1, '2');
	mf_write_char_to_display_segment(2, '3');
	mf_write_char_to_display_segment(3, '4');
}

/**
 * definition of callback functions
 */
void read_adc_values(void) {
	if (global_adc_event) {
		global_adc_event = false;
		uint32_t adc_in_percent = global_adc_value * 100 / 4000;
		mf_write_gauge_to_display_segment(3, adc_in_percent);
		HAL_ADC_Start_DMA(&hadc1, &global_adc_value, 1);
	}
}

void read_button_state(void) {
	uint8_t button = 0;
	button |= MF_BUTTON_1_READ() << 0;
	button |= MF_BUTTON_2_READ() << 1;
	button |= MF_BUTTON_3_READ() << 2;

	/* invert button state and mask to 3 bits since buttons are low active */
	button = ~button & 0x07;
	if (button != global_button_state) {
		global_button_state = button;
		global_button_event = true;
	}
}

void inter_task_communication(void) {
	if (global_button_event) {
		global_button_event = false;
		if (global_button_state != 0) {
			printf("Button got pressed\n");
		} else if (global_button_state == 0) {
			printf("Button got released\n");
		}
	}

	if (global_button_state & 0x01) {
		MF_BEEP_ON();
	} else {
		MF_BEEP_OFF();
	}
}

void test_callback1(void) {
	printf("Button state %d\n\n", global_button_state);
}

void test_callback2(void) {
	printf("\tHello World Task 2\n");
}

void demo_seven_segment(void) {
	static uint8_t cnt;
	mf_write_char_to_display_segment(0, mf_uint_to_char(cnt));
	mf_write_char_to_display_segment(1, mf_uint_to_char(cnt));
	mf_write_char_to_display_segment(2, mf_uint_to_char(cnt));
	//mf_write_char_to_display_segment(3, mf_uint_to_char(cnt));
	cnt += 1;
	if (cnt >= 16) {
		cnt = 0;
	}
}

void refresh_gui(void) {
	mf_refresh_display();
}

void demo_led(void) {
	static uint8_t state = 0;
	static uint8_t shift = 1;
	if (state == 0) {
		shift = shift << 1;
		if (shift == 8)
			state = 1;
	} else {
		shift = shift >> 1;
		if (shift == 1)
			state = 0;
	}
	switch (shift) {
	case 1:
		MF_LED1_turn_on();
		MF_LED2_turn_off();
		MF_LED3_turn_off();
		MF_LED4_turn_off();
		break;
	case 2:
		MF_LED1_turn_off();
		MF_LED2_turn_on();
		MF_LED3_turn_off();
		MF_LED4_turn_off();
		break;
	case 4:
		MF_LED1_turn_off();
		MF_LED2_turn_off();
		MF_LED3_turn_on();
		MF_LED4_turn_off();
		break;
	case 8:
		MF_LED1_turn_off();
		MF_LED2_turn_off();
		MF_LED3_turn_off();
		MF_LED4_turn_on();
		break;
	default:
		break;
	}
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
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();
	MX_TIM6_Init();
	/* USER CODE BEGIN 2 */

	noRTOS_task_t refresh_gui_t = { .delay = eNORTOS_PERIODE_6ms, .task_callback = refresh_gui };
	noRTOS_add_task_to_scheduler(&refresh_gui_t);

	noRTOS_task_t read_button_state_t = { .delay = eNORTOS_PERIODE_10ms, .task_callback = read_button_state };
	noRTOS_add_task_to_scheduler(&read_button_state_t);

	noRTOS_task_t inter_task_communication_t = { .delay = eNORTOS_PERIODE_100ms, .task_callback = inter_task_communication };
	noRTOS_add_task_to_scheduler(&inter_task_communication_t);

	noRTOS_task_t demo_led_t = { .delay = eNORTOS_PERIODE_500ms, .task_callback = demo_led };
	noRTOS_add_task_to_scheduler(&demo_led_t);

	noRTOS_task_t adc_t = { .delay = eNORTOS_PERIODE_500ms, .task_callback = read_adc_values };
	noRTOS_add_task_to_scheduler(&adc_t);

	noRTOS_task_t demo_seven_segment_t = { .delay = eNORTOS_PERIODE_1s, .task_callback = demo_seven_segment };
	noRTOS_add_task_to_scheduler(&demo_seven_segment_t);

	noRTOS_task_t test_task1 = { .delay = eNORTOS_PERIODE_1s, .task_callback = test_callback1 };
	noRTOS_add_task_to_scheduler(&test_task1);

	noRTOS_task_t test_task2 = { .delay = eNORTOS_PERIODE_5s, .task_callback = test_callback2 };
	noRTOS_add_task_to_scheduler(&test_task2);

	/* this runs for ever */
	noRTOS_run_scheduler();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		printf("hello multi-function shield\n");
		MF_LED1_toggle();
		MF_LED2_toggle();
		MF_LED3_toggle();
		MF_LED4_toggle();
		DELAY(200);
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
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if (hadc->Instance == ADC1) {
		global_adc_event = true;
	}
}
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
