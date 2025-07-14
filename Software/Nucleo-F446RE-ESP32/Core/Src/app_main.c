#include "app_main.h"

#include "noRTOS.h"
#include "hardwareGlobal.h"
#include "Drivers/Communication/FIFO/fifo.h"
#include "math.h"

// Declare a global instance of mqtt_client
mqtt_client_t esp32_mqtt_client = {0};

// ---------- platform specific (STM32) interrupt management -------------------------------

// local, very short, buffer since we reading byte wise
uint8_t uart_internet_interrupt_buffer[8] = {0};

// local, little larger buffer for uart-bridge
#define __1KByte__ 1024
#define uart_rx_buffer_size (__1KByte__)
uint8_t uart_rx_buffer_terminal[uart_rx_buffer_size] = {0};

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

// ---------------- noRTOS initial setup (runs once on boot) -------------------

void noRTOS_setup(void) {

	// start mqtt client by giving credentials
	esp32_mqtt_client.wifi_ssid 			= "hot-spot";
	esp32_mqtt_client.wifi_password 		= "JKp8636785";
//	esp32_mqtt_client.wifi_ssid 			= "iPhone11";
//	esp32_mqtt_client.wifi_password 		= "abc123456";

	esp32_mqtt_client.mqtt_broker_endpoint 	= "broker.emqx.io"; // EMQX broker endpoint
	esp32_mqtt_client.mqtt_username 		= "emqx";  			// MQTT username for authentication
	esp32_mqtt_client.mqtt_password 		= "public";  		// MQTT password for authentication
	esp32_mqtt_client.mqtt_port 			= "1883";  			// MQTT port (TCP)

	// enable uart RX byte-wise interrupt for ESP32 AT-Command Communication
	UART_INTERNET_READ_BYTE_IRQ( uart_internet_interrupt_buffer );

	// enable uart with DMA interupt for COM port (terminal)
	UART_TERMINAL_READ_LINE_IRQ( uart_rx_buffer_terminal, uart_rx_buffer_size);
	printf("ESP32 Demo\n");
}

// ---------------- noRTOS callback (runs periodic called by scheduler)-------------------

void internet_fsm(void){
	mqtt_client_fsm(&esp32_mqtt_client);
}

void led_snake(void){
	static uint8_t step = 0;

	switch (step) {
		case 0:
			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET);
			break;
		case 1:
			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_SET);
			break;
		case 2:
			HAL_GPIO_WritePin(	USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_SET);
			break;
		case 3:
			HAL_GPIO_WritePin(	USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_SET);
			break;
		case 4:
			HAL_GPIO_WritePin(	USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_RESET);
			break;
		case 5:
			HAL_GPIO_WritePin(	USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_RESET);
			break;
		case 6:
			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_RESET);
			break;
		case 7:
			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_RESET);
			break;
		default:
			break;
	}
	step ++;
	if(step > 7) step = 0;
}

void heart_beat(void){
	NUCLEO_LED_toggle();
}

// just a heave task to keep the cpu busy and test "real time"

// just a structure for handle some dummy data in the application code
typedef struct{
	float sin;
	float cos;
	float tan;
	uint32_t time;
	uint32_t date;
}application_output;
application_output app = {0};

void control_algorithm(void){
	struct tm* now = get_gmtime_stm32();
	float now_minutes = now->tm_hour*60.0 + (float)now->tm_min;

	float amp = 10.0;
	float off = 5;
	float f = 1/(24.0*60.0); // T = 24h
	float max = 70.0;
	float min = -10.0;

	float sin = sinf( 2*M_PI*f * (float)(now_minutes)) * amp;
	float cos = cosf( 2*M_PI*f * (float)(now_minutes)) * amp * (-1.0) + off;
	float tan = tanf( 2*M_PI*f * (float)(now_minutes)) * amp;

	/* catch min/max boundaries */
	if(sin > max) sin = max;
	if(sin < min) sin = min;
	if(cos > max) cos = max;
	if(cos < min) cos = min;
	if(tan > max) tan = max;
	if(tan < min) tan = min;

	app.sin = sin;
	app.cos = cos;
	app.tan = tan;

	printf(" -- Hello World -- \n");
}

void print_curretn_time(void) {
	// print epoch time fetched from stm32 internal real time clock (RTC)
	uint32_t epochtime = get_epoch_time();

	// print timestamp from stm32 internal RTC
	struct tm *timestamp = get_gmtime_stm32();

	// print them all nice to log in terminal
	printf("Time-Date-Logging:epoch,%ld,ts_hh,%d,ts_mm,%d,ts_ss,%d\r\n",
	epochtime, timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec);
}

void resynch_time_date(void){
	printf("Resynch RTC with SNTP time\n");
	fsm_job_queue_put(request_sntp_time);
}

void pub_telemetry(void){
	char payload[128];
	uint16_t size = 0;
	uint32_t timedate = get_epoch_time();
	size = sprintf(payload, "{\"timestamp\":%ld,\"sin\":%.2f,\"cos\":%.2f,\"tan\":%.2f}", timedate, app.sin, app.cos, app.tan);
	printf("Telemetry [%d bytes] Payload: %s\n", size, payload);

	// todo build a nice to read wrapper function that copys payload to client pub buffer
	memcpy(esp32_mqtt_client.at_pub_payload, payload, size);
	esp32_mqtt_client.at_pub_payload_size = size;

	fsm_job_queue_put(publish_raw_mqtt_msg);
}

void live_data(void){

}

// ----------------------- this is the super-loop() ----------------------------------------
void app_main() {
	noRTOS_task_t led_snake_t = { .delay = eNORTOS_PERIODE_200ms, .task_callback = led_snake };
	noRTOS_add_task_to_scheduler(&led_snake_t);

	noRTOS_task_t internet_fsm_t = { .delay = eNORTOS_PERIODE_500ms, .task_callback = internet_fsm };
	noRTOS_add_task_to_scheduler(&internet_fsm_t);

	noRTOS_task_t heartbeat_t = { .delay = eNORTOS_PERIODE_1s, .task_callback = heart_beat };
	noRTOS_add_task_to_scheduler(&heartbeat_t);

	noRTOS_task_t application_t = { .delay = eNORTOS_PERIODE_10s, .task_callback = control_algorithm };
	noRTOS_add_task_to_scheduler(&application_t);

	noRTOS_task_t pub_telemetry_t = { .delay = eNORTOS_PERIODE_30s, .task_callback = pub_telemetry };
	noRTOS_add_task_to_scheduler(&pub_telemetry_t);

	noRTOS_task_t time_date_t = { .delay = eNORTOS_PERIODE_1min, .task_callback = print_curretn_time };
	noRTOS_add_task_to_scheduler(&time_date_t);

	noRTOS_task_t resynch_time_date_t = { .delay = eNORTOS_PERIODE_1h, .task_callback = resynch_time_date };
	noRTOS_add_task_to_scheduler(&resynch_time_date_t);

	noRTOS_run_scheduler();
}

