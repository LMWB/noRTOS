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
#include "Drivers/SSD1306/ssd1306.h"
#include "Drivers/PCF8574/pcf8574.h"

#include "Drivers/Communication/CAN/can_config.h"

#include <stdio.h>

uint8_t uart2_buffer[UART_BUFFER_SIZE] = {0};
uint8_t uart4_buffer[UART_BUFFER_SIZE] = {0};

uint8_t uart2_buffer_rx_size = 0;
uint8_t uart4_buffer_rx_size = 0;

static uint32_t gTotal_CPU_Ticks = 0;
static uint32_t gButton_states = 0x80000000;
static uint16_t gADC_raw_data[ADC_NO_OF_CHANNELS];
static int16_t gTemperature = 0;
static int16_t gHumidity = 0;
static uint32_t gPressure = 0;
static int16_t gAltitude = 0;
static uint8_t gDisplayState = 0;

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

	// override CAN-ID with CPU-UID
	canID = cpu_id.uid[0];

	// set CPU ID as CAN ID
	configure_can_tx_identifier(canID);

	// set CAN filter mask
	set_can_extended_ID_filter(canID);

	// set CAN filter mask
	set_can_extended_ID_filter(canID);
	printf("CAN filter has been set to 0x%lX\r\n", canID);
	printf("Device will only react to CAN messages with this ID\r\n");

	drv8908_Init();

	UART_TERMINAL_READ_LINE_IRQ(	uart2_buffer, UART_BUFFER_SIZE);
	UART_RS485_READ_LINE_IRQ(		uart4_buffer, UART_BUFFER_SIZE);

	ADC_START_DMA(&gADC_raw_data);

	IIR_Low_Pass_init(&filter1);
	IIR_Low_Pass_init(&filter2);
	IIR_Low_Pass_init(&filter3);
	IIR_Low_Pass_init(&filter4);

	TIMER_START_PWM_CH1();
	TIMER_START_PWM_CH2();

	char t[] = "Fri Feb 19 17:47:56 2026";
	struct tm tm;
	cvt_asctime(t, &tm);
	set_gmtime_stm32(&tm);

	/* put RS485 in receiver mode */
	WRITE_PIN(RS485_Enable_GPIO_Port, RS485_Enable_Pin, 0);

	/* cpu load counter */
	DWT_Init();
}

void uart_loop_back(char* buf){
	printf("loopback %s", buf);
}

void rs485_loop_back(char* buf, uint8_t length){
	/* put RS485 in transmit mode */
	WRITE_PIN(RS485_Enable_GPIO_Port, RS485_Enable_Pin, 1);
	UART_RS485_SEND( (uint8_t*)buf, length);
	/* put RS485 in receiver mode */
	WRITE_PIN(RS485_Enable_GPIO_Port, RS485_Enable_Pin, 0);
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

/* override */
void noRTOS_RS485_RX_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */
	printf("received RS45 interrupt\n");
	rs485_loop_back( (char*)uart4_buffer, uart4_buffer_rx_size);

	/* now since the IRQ callback has been executed, we can restart the IRQ trigger */
	UART_RS485_READ_LINE_IRQ(uart4_buffer, UART_BUFFER_SIZE);
}

void noRTOS_DIGITAL_INPUT_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */
	printf("Digital Input interrupt\n");
}

void noRTOS_USB_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */
	printf("USB RX interrupt\n");
}

void noRTOS_ADC_IRQ(void){
	/* do some stuff you like to do when IRQ has triggered */

	/* restart ADC sampling */
	//ADC_START_DMA(&gADC_raw_data);
}

void noRTOS_CAN_RX_IRQ(void){
	printf("CAN RX interrupt\n");
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

		/* do stuff when button state has changed*/
		printf("Digital Input software interrupt\n");

		// check for rising edge on bit 0
		if( ((button_states & 0x01) == 1) && ((gButton_states & 0x01) == 0) ){
			printf("Digital Input Rising Edge Detected\n");
			// increment display state counter, this is a comprehensive task action
			gDisplayState +=1;
		}

		/* save the new flag globally */
		gButton_states = button_states;
	}
}

void process_analog_readings(void){

	if( noRTOS_wait_for_event(eBIT_MASK_ADC_INTERRUPT) ){
		/* do stuff like digital filtering etc. */
		IIR_Low_Pass_update(&filter1, gADC_raw_data[0]);
		IIR_Low_Pass_update(&filter2, gADC_raw_data[1]);
		IIR_Low_Pass_update(&filter3, gADC_raw_data[2]);
		IIR_Low_Pass_update(&filter4, gADC_raw_data[3]);

		/* finally restart adc sampling */
		ADC_START_DMA(&gADC_raw_data);
	}
}

void pfc8574_state_machine(void) {
	static uint8_t z = 0;
	switch (z) {
	case 0:
		// 0xF0 -> 1111 0000 (Pins 4-7 auf High für Input, Pins 0-3 auf Low für LEDs aus)
		if ( PCF8574_Init( 0xF0 ) == HAL_OK) {
			z = 1;
		}
		break;
	case 1:
		PCF8574_Write( 0x0F );
		z = 2;
		break;
	case 2:
		PCF8574_Write( 0xF0 );
		z = 1;
		break;
	default:
		z = 0;
		break;
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
 * htim17.Instance = TIM17;
 * htim17.Init.Prescaler = 480-1;
 * htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
 * htim17.Init.Period = 1024-1;
 * htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
 * htim17.Init.RepetitionCounter = 0;
 * htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
 */

void fading_heartbeat(void) {
	static int8_t counter = 0;
	/* 0 => up, else => down */
	static uint8_t direction = 0;
	const uint8_t START = 24;
	const uint8_t STOP = 64;

	/* https://www.mikrocontroller.net/articles/LED-Fading */
//	const uint16_t pwmtable_8D[32] =
//	{
//	    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
//	    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
//	};

	const uint16_t pwmtable_10[64] = {
			0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6,
			6, 7, 8, 9, 10, 11, 12, 13, 15, 17, 19, 21, 23,
			26, 29, 32, 36, 40, 44, 49, 55, 61, 68, 76, 85,
			94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
			279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023 };

	TIMER_SET_PWM_DUTY_CYCLE_CH1(pwmtable_10[counter]);

	switch (direction) {
	case 0: /* counting up */
		counter++;
		if (counter >= STOP) {
			direction = 1;
		}
		break;
	case 1: /* counting down */
		counter--;
		if (counter <= START) {
			direction = 0;
		}
		break;
	default:
		break;
	}
}

void build_display_content(void) {
	const uint8_t line_hight = 16;
	const uint8_t line_width = 18;
	const uint8_t barchart_scale = 255; // 4095 (ADC full scale) / 255 = 16 (Characters for bar graf)

	char msg[64];
	/* function gets called every 200ms, so it needs to be called 5 times for 1 full second */
	static uint32_t up_time_sec = 0;

	switch (gDisplayState) {
	case 0:
		uint32_t adc_ref_voltage = 	ADC_CALC_REF_VOLTAGE(gADC_raw_data[5]);
		uint16_t adc_voltage = 		ADC_CALC_DATA_TO_MILLI_VOLT(adc_ref_voltage, filter4.y1);
		ssd1306_Fill(Black);

		ssd1306_SetCursor(1, 0);
		sprintf(msg, "Run Time: %ld s", up_time_sec / 5);
		ssd1306_WriteString(msg, Font_7x10, White);
		ssd1306_SetCursor(1, 1 * line_hight);

		// build horizontal bar chart gauge
		msg[0] = 0;
		for (uint_fast8_t i = 0; i < (up_time_sec % line_width); i++) {
			msg[i] = '#';
			msg[i + 1] = 0;
		}
		ssd1306_WriteString(msg, Font_7x10, White);

		ssd1306_SetCursor(1, 2 * line_hight);
		sprintf(msg, "ADC 1: %ld mV", adc_voltage);
		ssd1306_WriteString(msg, Font_7x10, White);

		ssd1306_SetCursor(1, 3 * line_hight);
		sprintf(msg, "noRTOS V.%d.%d", NORTOS_VERSION_MAYOR, NORTOS_VERSION_MINOR);
		ssd1306_WriteString(msg, Font_7x10, White);
		break;
	case 1:
		ssd1306_Fill(Black);

		// Zeile 1
		ssd1306_SetCursor(1, 0);
		sprintf(msg, "x:");
		// build horizontal bar chart gauge
		uint8_t max = filter1.y1 / barchart_scale;
		for (uint_fast8_t i = 0; i < (max); i++) {
			msg[i+2] = '#';
			msg[i+3] = 0;
		}
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 2
		ssd1306_SetCursor(1, 1 * line_hight);
		sprintf(msg, "y:");
		// build horizontal bar chart gauge
		max = filter2.y1 / barchart_scale;
		for (uint_fast8_t i = 0; i < (max); i++) {
			msg[i+2] = '#';
			msg[i+3] = 0;
		}
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 3
		ssd1306_SetCursor(1, 2 * line_hight);
		sprintf(msg, "z:");
		// build horizontal bar chart gauge
		max = filter3.y1 / barchart_scale;
		for (uint_fast8_t i = 0; i < (max); i++) {
			msg[i+2] = '#';
			msg[i+3] = 0;
		}
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 4
		ssd1306_SetCursor(1, 3 * line_hight);
		sprintf(msg, "CPU Temp.: %d", (int)filter4.y1);
		ssd1306_WriteString(msg, Font_7x10, White);
		break;
	case 2:
		ssd1306_Fill(Black);
		// Zeile 1
		ssd1306_SetCursor(1, 0);
		sprintf(msg, "P: %ld", gPressure);
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 2
		ssd1306_SetCursor(1, 1 * line_hight);
		sprintf(msg, "T: %d", gTemperature);
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 3
		ssd1306_SetCursor(1, 2 * line_hight);
		sprintf(msg, "H: %d", gHumidity);
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 4
		ssd1306_SetCursor(1, 3 * line_hight);
		sprintf(msg, "CPU Load: %ld", gTotal_CPU_Ticks);
		ssd1306_WriteString(msg, Font_7x10, White);
		break;

	case 3:
		struct tm *ptm;
		ptm = get_gmtime_stm32();
		ssd1306_Fill(Black);
		// Zeile 1
		ssd1306_SetCursor(1, 0);
		sprintf(msg, "Time: %02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 2
		ssd1306_SetCursor(1, 1 * line_hight);
		sprintf(msg, "Date: %02d:%02d:%02d",ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year-100);
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 3
		ssd1306_SetCursor(1, 2 * line_hight);
		sprintf(msg, "---");
		ssd1306_WriteString(msg, Font_7x10, White);

		// Zeile 4
		ssd1306_SetCursor(1, 3 * line_hight);
		sprintf(msg, "CPU Load: %ld", gTotal_CPU_Ticks);
		ssd1306_WriteString(msg, Font_7x10, White);
		break;
	case 4:
		gDisplayState = 0;
	default:
		gDisplayState = 0;
		break;
	}
	ssd1306_UpdateScreen();
	up_time_sec++;
}

void refresh_display(void) {
	uint32_t end;
	uint32_t start = GET_CPU_TICKS();
	static uint8_t z = 0;

	switch (z) {
	case 0:
		if ( I2C_IS_DEVICE_READY(SSD1306_I2C_ADDR) == DEVICE_OK) {
			ssd1306_Init();
			z = 1;
		} else {
			z = z;
		}
		break;
	case 1:
		build_display_content();
		z = z;
		break;
	default:
		z = 0;
		break;
	}
	end = GET_CPU_TICKS();
	gTotal_CPU_Ticks = end - start;
}

void testing_watchdog_restart(void){
	printf("So far the application ha been run for about 2 minutes. \n");
	printf("The watchdog will restart the system in ...\n");
	int count_down = 31;
	while(1){
		myprintf("%d, ", count_down);
		DELAY(1000);
		count_down--;
	}
}

void refresh_watchdog(void){
	WATCHDOG_REFRESH();
}

void can_dummy_message(void) {
	static uint8_t z = 0;
	uint8_t m[4];
	uint32_t messageID = canID;
	uint32_t const offset = 0x1000;
	uint32_t const p = 0x01;
	uint32_t const t = 0x02;
	uint32_t const h = 0x03;
	uint32_t const adc = 0x04;


	switch (z) {
	case 0:
		m[3] = gPressure & 0xff;
		m[2] = (gPressure & 0xff00) >> 8;
		m[1] = (gPressure & 0xff0000) >> 16;
		m[0] = (gPressure & 0xff000000) >> 24;
		messageID = messageID + offset + p;
		z=1;
		break;
	case 1:
		m[3] = gTemperature & 0xff;
		m[2] = (gTemperature & 0xff00) >> 8;
		m[1] = (gTemperature & 0xff0000) >> 16;
		m[0] = (gTemperature & 0xff000000) >> 24;
		messageID = messageID + offset + t;
		z=2;
		break;
	case 2:
		m[3] = gHumidity & 0xff;
		m[2] = (gHumidity & 0xff00) >> 8;
		m[1] = (gHumidity & 0xff0000) >> 16;
		m[0] = (gHumidity & 0xff000000) >> 24;
		messageID = messageID + offset + h;
		z=3;
		break;
	case 3:
		m[3] = filter4.y1 & 0xff;
		m[2] = (filter4.y1 & 0xff00) >> 8;
		m[1] = (filter4.y1 & 0xff0000) >> 16;
		m[0] = (filter4.y1 & 0xff000000) >> 24;
		messageID = messageID + offset + adc;
		z=0;
		break;
	default:
		break;
	}
	send_can_message(messageID, m, 4);
}

void rs485_dummy_message(void){
	/* put RS485 in transmit mode */
	WRITE_PIN(RS485_Enable_GPIO_Port, RS485_Enable_Pin, 1);
	UART_RS485_SEND((uint8_t*)"Hello World\r\n", 13);
	/* put RS485 in receiver mode */
	WRITE_PIN(RS485_Enable_GPIO_Port, RS485_Enable_Pin, 0);
}

void usb_dummy_message(void){
	char usb_msg[] = "Hello World from USB\r\n";

	vcom_write(usb_msg, strlen(usb_msg));
}

void app_demo_main(void){
	noRTOS_task_t buttons = { .delay = eNORTOS_PERIODE_100ms, .task_callback = read_button_states };
	noRTOS_add_task_to_scheduler(&buttons);

	noRTOS_task_t analog = { .delay = eNORTOS_PERIODE_100ms, .task_callback = process_analog_readings };
	noRTOS_add_task_to_scheduler(&analog);

	noRTOS_task_t heartbeat = { .delay = eNORTOS_PERIODE_100ms, .task_callback = fading_heartbeat};
	noRTOS_add_task_to_scheduler(&heartbeat);

	noRTOS_task_t display = { .delay = eNORTOS_PERIODE_200ms, .task_callback = refresh_display};
	noRTOS_add_task_to_scheduler(&display);

	noRTOS_task_t blinky = { .delay = eNORTOS_PERIODE_500ms, .task_callback = blink_LED };
	noRTOS_add_task_to_scheduler(&blinky);

	noRTOS_task_t blinky2 = { .delay = eNORTOS_PERIODE_500ms, .task_callback = pfc8574_state_machine };
	noRTOS_add_task_to_scheduler(&blinky2);

	noRTOS_task_t snake = { .delay = eNORTOS_PERIODE_500ms, .task_callback = drv8908_state_machine };
	noRTOS_add_task_to_scheduler(&snake);

	noRTOS_task_t canTX = { .delay = eNORTOS_PERIODE_500ms, .task_callback = can_dummy_message };
	noRTOS_add_task_to_scheduler(&canTX);

	noRTOS_task_t rs485TX = { .delay = eNORTOS_PERIODE_1s, .task_callback = rs485_dummy_message };
	noRTOS_add_task_to_scheduler(&rs485TX);

	noRTOS_task_t usb_vcom = { .delay = eNORTOS_PERIODE_1s, .task_callback = usb_dummy_message };
	noRTOS_add_task_to_scheduler(&usb_vcom);

	noRTOS_task_t temperature = { .delay = eNORTOS_PERIODE_1s, .task_callback = bme280_state_machine };
	noRTOS_add_task_to_scheduler(&temperature);

	noRTOS_task_t watchdog = { .delay = eNORTOS_PERIODE_30s, .task_callback = refresh_watchdog };
	noRTOS_add_task_to_scheduler(&watchdog);

	noRTOS_task_t test_watchdog = { .delay = eNORTOS_PERIODE_2min, .task_callback = testing_watchdog_restart };
	/* leaf task commented out unless you like to test the watchdog */
	//noRTOS_add_task_to_scheduler(&test_watchdog);

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
		uart2_buffer_rx_size = Size;
		uart2_buffer[uart2_buffer_rx_size+1] = '\0';
		noRTOS_set_interrupt_received_flag(eBIT_MASK_UART_INTERRUPT);
	}

	if (huart->Instance == UART_RS485_INSTANCE){
		// add null terminator to override what was received before
		uart4_buffer_rx_size = Size;
		uart4_buffer[uart4_buffer_rx_size+1] = '\0';
		noRTOS_set_interrupt_received_flag(eBIT_MASK_RS485_INTERRUPT);
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

/* *************** STM32 HAL Based CAN RX interrupt *************** */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	uint32_t fill_level = HAL_CAN_GetRxFifoFillLevel(hcan, CAN_FILTER_FIFO0);
	/* fill_level not used yet */
	fill_level = fill_level;

	/* pick up incoming data and release the CAN Peripheral */
	if( HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canRxHeader, canRxData) == HAL_OK){
		set_can_message_pending();

		/* depending on what you like to do when receiving CAN message */

		/* use this if you like to react immediately (asynchronous) */
		noRTOS_set_interrupt_received_flag(eBIT_MASK_CAN_INTERRUPT);
		/* use this if you like to wait for a CAN message (synchronous) */
		noRTOS_set_event_received_flag(eBIT_MASK_CAN_INTERRUPT);
	}
}


// todo: where to move ?
/* *************** USB VCOM Buffer *************** */
uint8_t usb_RX_buffer[512] = {0}; // used at usbd_csc_if.c line 266
uint32_t msg_cnt = 0;

int vcom_write(char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*) ptr, (uint16_t)len);
	return len;
}

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

