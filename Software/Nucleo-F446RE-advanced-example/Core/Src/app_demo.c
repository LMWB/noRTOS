#include "main.h"

#include "app_demo.h"
#include "platformGlue.h"
#include "noRTOS.h"
#include "Drivers/DRV8908/drv8908.h"
#include "Drivers/SE95/se95.h"

#include <stdio.h>

static uint32_t gButton_states = 0x80000000;
static uint16_t gADC_raw_data[ADC_NO_OF_CHANNELS];
static int16_t gTemperature = 0;

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
	if( noRTOS_wait_for_event(eBIT_MASK_ADC_INTERRUPT) ){
		/* do stuff like digital filtering etc. */
		__NOP();

		/* finally restart adc sampling */
		ADC_START_DMA(&gADC_raw_data);
	}
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

void se95_state_machine(void){
	static uint8_t z = 0;
	switch (z) {
		case 0:
			if(SE95_Init() == DEVICE_OK){
				z = 1;
			}
			break;
		case 1:
			gTemperature = SE95_ReadTemperature();
			if(gTemperature == SE95_ERROR_CODE){
				z = 0;
			}
			break;
		default:
			z = 0;
			break;
	}
}

void app_demo_main(void){
	noRTOS_task_t buttons = { .delay = eNORTOS_PERIODE_100ms, .task_callback = read_button_states };
	noRTOS_add_task_to_scheduler(&buttons);

	noRTOS_task_t analog = { .delay = eNORTOS_PERIODE_200ms, .task_callback = process_analog_readings };
	noRTOS_add_task_to_scheduler(&analog);

	noRTOS_task_t blinky = { .delay = eNORTOS_PERIODE_500ms, .task_callback = blink_LED };
	noRTOS_add_task_to_scheduler(&blinky);

	noRTOS_task_t snake = { .delay = eNORTOS_PERIODE_500ms, .task_callback = drv8908_state_machine };
	noRTOS_add_task_to_scheduler(&snake);

	noRTOS_task_t temperature = { .delay = eNORTOS_PERIODE_1s, .task_callback = se95_state_machine };
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
		// add null terminator to overide what was received befor
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
