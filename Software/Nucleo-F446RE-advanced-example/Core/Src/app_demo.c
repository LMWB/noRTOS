#include "main.h"

#include "app_demo.h"
#include "platformGlue.h"
#include "noRTOS.h"
#include "utils.h"
#include "dsp.h"
#include "Drivers/DRV8908/drv8908.h"
#include "Drivers/SE95/se95.h"
#include "Drivers/AHT21/aht21.h"
#include "Drivers/BME280/bme280.h"

#include <stdio.h>

uint8_t uart2_buffer[UART_BUFFER_SIZE] = {0};
uint8_t uart4_buffer[UART_BUFFER_SIZE] = {0};

static uint32_t gTotal_CPU_Ticks = 0;
static uint32_t gButton_states = 0x80000000;
static uint16_t gADC_raw_data[ADC_NO_OF_CHANNELS];
static int16_t gTemperature = 0;
static int16_t gHumidity = 0;
static uint32_t gPressure = 0;
static int16_t gAltitude = 0;

static IIR_State filter1;
static IIR_State filter2;
static IIR_State filter3;
static IIR_State filter4;

/* override */
void noRTOS_setup(void) {
	/* read the STM32 CPU unique ID, could be handy to have for various
	 * things in software since this is is a singular number world wide */
	cpu_uid_t cpu_id;
	READ_CPU_UID(cpu_id);

	printf("STM32 UUID (int): %ld-%ld-%ld \r\n", cpu_id.uid[0], cpu_id.uid[1], cpu_id.uid[2]);
	printf("STM32 UUID (hex): 0x%08lX-0x%08lX-0x%08lX \r\n", cpu_id.uid[0], cpu_id.uid[1], cpu_id.uid[2]);
	printf("\r\n");

	drv8908_Init();

	UART_TERMINAL_READ_LINE_IRQ(uart2_buffer, UART_BUFFER_SIZE);

	ADC_START_DMA(&gADC_raw_data);

	IIR_Low_Pass_init(&filter1);
	IIR_Low_Pass_init(&filter2);
	IIR_Low_Pass_init(&filter3);
	IIR_Low_Pass_init(&filter4);

	TIMER_START_PWM_CH1();
	TIMER_START_PWM_CH2();

	DWT_Init();
}

void uart_loop_back(char* buf){
	printf("loopback %s", buf);
}

/* -------- Asynchronous Tasks ----------------- */
/* override */
void noRTOS_UART_RX_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */
	printf("received UART interrupt\n");
	uart_loop_back((char*)uart2_buffer);

	/* now since the IRQ callback has been executed, we can restart the IRQ trigger */
	UART_TERMINAL_READ_LINE_IRQ(uart2_buffer, UART_BUFFER_SIZE);
}

void noRTOS_DIGITAL_INPUT_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */
	printf("Digital Input interrupt\n");
}

void noRTOS_ADC_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */

	/* restart ADC sampling */
	//ADC_START_DMA(&gADC_raw_data);
}

/* -------- Synchronous Tasks ----------------- */
void blink_LED(void) {
	LED_GREEN_TOGGLE();
}

uint8_t read_gpio(uint8_t slot) {
	uint8_t return_value = 0;
	switch (slot) {
	case 0:
		return_value = READ_PIN(DI1_GPIO_Port, DI1_Pin);
		break;
	case 1:
		return_value = READ_PIN(DI2_GPIO_Port, DI2_Pin);
		break;
	case 2:
		return_value = READ_PIN(DI3_GPIO_Port, DI3_Pin);
		break;
	case 3:
		return_value = READ_PIN(DI4_GPIO_Port, DI4_Pin);
		break;
	default:
		break;
	}
	return return_value;
}

/* this is a kind of software interrupt */
void read_button_states(void){
	uint32_t button_states = 0;

	for (uint_fast8_t i = 0;  i < 4; ++ i) {
		button_states |= read_gpio(i) << i;
	}

	/* because of inverted logic (pull ups), flip all bits and limit to 18 digits*/
	button_states = ~button_states;
	button_states = (button_states & 0xF); // 4 buttons max in ths example

	if (button_states != gButton_states) {
		/* save the new flag globally */
		gButton_states = button_states;

		/* do stuff when button state has changed*/
		printf("Digital Input software interrupt\n");
	}
}

void process_analog_readings(void){
	uint32_t end;
	uint32_t start = GET_CPU_TICKS();
	if( noRTOS_wait_for_event(eBIT_MASK_ADC_INTERRUPT) ){
		/* do stuff like digital filtering etc. */
		IIR_Low_Pass_update(&filter1, gADC_raw_data[0]);
		IIR_Low_Pass_update(&filter2, gADC_raw_data[1]);
		IIR_Low_Pass_update(&filter3, gADC_raw_data[2]);
		IIR_Low_Pass_update(&filter4, gADC_raw_data[3]);

		/* finally restart adc sampling */
		ADC_START_DMA(&gADC_raw_data);
	}
	end = GET_CPU_TICKS();
	gTotal_CPU_Ticks = end - start;
}

void drv8908_state_machine(void) {
	static uint16_t z = 0;

	if (z == 0x1FF) {
		// if 8 bits full clear to 0
		z = 0;
	}

	DRV8908_write_bulk_output_register(z);

	// adding zeros from right (LSB)
	z = z << 1;
	z = z | 1;
}

void se95_state_machine(void) {
	static uint8_t z = 0;
	switch (z) {
	case 0:
		if (SE95_Init() == DEVICE_OK) {
			z = 1;
		}
		break;
	case 1:
		gTemperature = SE95_read_temperature();
		if (gTemperature == SE95_ERROR_CODE) {
			z = 0;
		}
		break;
	default:
		z = 0;
		break;
	}
}

void ath21_state_machine(void) {
	static uint8_t z = 0;
	switch (z) {
	case 0:
		if (AHT21_Init() == DEVICE_OK) {
			z = 1;
		}
		break;
	case 1:
		if (AHT21_start_convertion() == DEVICE_OK) {
			z = 2;
		} else {
			z = 0;
		}
		break;
	case 2:
		if (AHT21_read_data(&gTemperature, &gHumidity) == DEVICE_OK) {
			z = 1;
		} else {
			z = 0;
		}
		break;
	default:
		z = 0;
		break;
	}
}

void bme280_state_machine(void) {
	static uint8_t z = 0;
	switch (z) {
	case 0:
		if (BME280_Init() == DEVICE_OK) {
			z = 1;
		}
		break;
	case 1:
		BME280_read(&gTemperature, &gHumidity, &gPressure);

		// 101325 Pa ist der Standardwert, falls du kein lokales QNH hast
		gAltitude = BME280_get_altitude(gPressure, 100700);
		break;
	default:
		z = 0;
		break;
	}
}

/*
 *   htim17.Instance = TIM17;
  htim17.Init.Prescaler = 480-1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 1024-1;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
 */

void fading_heartbeat(void) {
	static int8_t counter = 0;
	static uint8_t direction = 0; /* 0 => up, else => down */
	/* https://www.mikrocontroller.net/articles/LED-Fading */
//	const uint16_t pwmtable_8D[32] =
//	{
//	    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
//	    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
//	};
	const uint16_t pwmtable_10[64] = { 0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5,
			5, 6, 6, 7, 8, 9, 10, 11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32,
			36, 40, 44, 49, 55, 61, 68, 76, 85, 94, 105, 117, 131, 146, 162,
			181, 202, 225, 250, 279, 311, 346, 386, 430, 479, 534, 595, 663,
			739, 824, 918, 1023 };

	//__HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, pwmtable_10[counter]);
	//TIMER_SET_PWM_DUTY_CYCLE_CH1(pwmtable_10[counter]);

	switch (direction) {
	case 0: /* counting up */
		counter++;
		if (counter >= 64 - 1) {
			direction = 1;
		}
		break;
	case 1: /* counting down */
		counter--;
		if (counter <= 0) {
			direction = 0;
		}
	default:
		break;
	}
}

void app_demo_main(void){
	noRTOS_task_t buttons = { .delay = eNORTOS_PERIODE_100ms, .task_callback = read_button_states };
	noRTOS_add_task_to_scheduler(&buttons);

	noRTOS_task_t analog = { .delay = eNORTOS_PERIODE_100ms, .task_callback = process_analog_readings };
	noRTOS_add_task_to_scheduler(&analog);

	noRTOS_task_t blinky = { .delay = eNORTOS_PERIODE_500ms, .task_callback = blink_LED };
	noRTOS_add_task_to_scheduler(&blinky);

	noRTOS_task_t snake = { .delay = eNORTOS_PERIODE_500ms, .task_callback = drv8908_state_machine };
	noRTOS_add_task_to_scheduler(&snake);

	noRTOS_task_t temperature = { .delay = eNORTOS_PERIODE_1s, .task_callback = bme280_state_machine };
	noRTOS_add_task_to_scheduler(&temperature);

	noRTOS_run_scheduler();
}

/* *************** STM32 HAL Based UART Bridge with RX-byte (char) interrupt *************** */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == UART_TERMINAL_INSTANCE){
		//uart_increment_pointer();
		UART_TERMINAL_READ_BYTE_IRQ( uart2_buffer );
	}
}

/* *************** STM32 HAL Based UART Bridge with RX-line interrupt *************** */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if (huart->Instance == UART_TERMINAL_INSTANCE){
		// add null terminator to override what was received before
		uart2_buffer[Size+1] = '\0';
		noRTOS_set_interrupt_received_flag(eBIT_MASK_UART_INTERRUPT);
	}

	if (huart->Instance == UART_RS485_INSTANCE){
		// add null terminator to override what was received before
		uart2_buffer[Size+1] = '\0';
		noRTOS_set_interrupt_received_flag(eBIT_MASK_UART_INTERRUPT);
	}
}

/* *************** STM32 HAL Based GPIO input interrupt *************** */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == B1_Pin){
		noRTOS_set_interrupt_received_flag(eBIT_MASK_DI_INTERRUPT);
	}
}

/* *************** STM32 HAL Based ADC input interrupt *************** */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	if(hadc->Instance == ADC_INSTANCE){
		noRTOS_set_interrupt_received_flag(eBIT_MASK_ADC_INTERRUPT);
		noRTOS_set_event_received_flag(eBIT_MASK_ADC_INTERRUPT);
	}
}


// todo: where to move ?
/* *************** UART Byte Wise Interrupt Receiver *************** */

static uint16_t rx_size = 0;
static bool uart2_read_line_complete = false;

bool noRTOS_is_UART2_read_line_complete(void){
	return uart2_read_line_complete;
}

void noRTOS_UART2_read_byte_with_interrupt(void){
	HAL_UART_Receive_IT(&huart2, &uart2_buffer[rx_size], 1);
}

void noRTOS_UART2_clear_rx_buffer(void){
	rx_size = 0;
	uart2_read_line_complete = false;
	memset(uart2_buffer, 0, UART_BUFFER_SIZE);
	noRTOS_UART2_read_byte_with_interrupt();
}

void noRTOS_UART2_echo_whats_been_received(void)
{
	UART_TERMINAL_SEND( uart2_buffer, rx_size);
}

void noRTOS_UART2_receive_byte_callback(void){
	/* check if read line complete */
	if(rx_size >= 3 && uart2_buffer[rx_size-1] == '\r' && uart2_buffer[rx_size] == '\n'){
		rx_size++;
		uart2_read_line_complete = true;
	}else{
		rx_size++;
		noRTOS_UART2_read_byte_with_interrupt();
	}
}

