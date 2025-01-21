#ifndef DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#define DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#include <stdint.h>
#include "fifo.h"

/* Temporaer */
#define __1KByte__ 1024
#define uart_rx_buffer_size (__1KByte__)

extern uint8_t uart_rx_buffer_internet[uart_rx_buffer_size];
extern uint16_t head;
extern uint16_t tail;
extern uint8_t uart_rx_buffer_terminal[uart_rx_buffer_size];
/* Temporaer */






typedef enum{
	 esp32_error = 0,
	 esp32_ok,
	 esp32_timeout,
	 esp32_idle,
}esp32_exit_code_t;

typedef struct __WiFi_Client__{
	const char* wifi_ssid;
	const char* wifi_password;
	const char* mqtt_broker_endpoint;
	const char* mqtt_username;
	const char* mqtt_password;
	const char* mqtt_port;
}WiFi_Client_t;

typedef enum{
	undefined = 0,
	standby,					// 1
	boot_up,					// 2
	connect_to_wifi,			// 3
	config_connection_to_broker,// 4
	connect_to_mqtt_broker,		// 5
	subscribe_to_mqtt_msg,		// 6
	online,						// 7
	publish_mqtt_msg,			// 8
	publish_raw_mqtt_msg,		// 9
	wait_for_response,			// 10
}MQTT_client_t;

typedef struct {
	/* public attributes */
	uint32_t timeout;
	fifo_t at_fifo;

	/* private attributes */
	char at_response_to_be[128];
	uint32_t timeout_start;
	MQTT_client_t next_state;	// next state aiming to after at_response success (nothing to do with state machine new_state)
}client_fsm_t;


void esp32_mqtt_fsm(client_fsm_t *client);

MQTT_client_t get_fsm_state(client_fsm_t *client);
void set_fsm_state(client_fsm_t *client, MQTT_client_t new_state);


#endif /* DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_ */
