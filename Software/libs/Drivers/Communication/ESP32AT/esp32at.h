#ifndef DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#define DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#include <stdint.h>
#include "fifo.h"

/* Temporaer todo remove this */
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
}mqtt_client_exit_code_t;

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
}mqtt_client_state_t;

typedef struct {
	/* public attributes */
	const char* wifi_ssid;
	const char* wifi_password;
	const char* mqtt_broker_endpoint;
	const char* mqtt_username;
	const char* mqtt_password;
	const char* mqtt_port;

	/* private attributes */
	fifo_t at_fifo;
	char at_response_to_be[128];
	char at_pub_payload[128];
	uint16_t at_pub_payload_size;
	uint32_t timeout;
	uint32_t timeout_start;
	mqtt_client_state_t next_state;	// next state aiming to after at_response success (nothing to do with state machine new_state)
}mqtt_client_t;


void mqtt_client_fsm(mqtt_client_t *client);

mqtt_client_state_t get_mqtt_client_state(mqtt_client_t *client);
void set_mqtt_client_state(mqtt_client_t *client, mqtt_client_state_t new_state);


#endif /* DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_ */
