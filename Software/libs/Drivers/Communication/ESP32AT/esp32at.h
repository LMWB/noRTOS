#ifndef DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#define DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_
#include <stdint.h>
#include "fifo.h"

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
	connect_sntp,				// 4
	config_connection_to_broker,// 5
	connect_to_mqtt_broker,		// 6
	subscribe_to_mqtt_msg,		// 7
	online,						// 8

	publish_mqtt_msg,			// 9
	publish_raw_mqtt_msg,		// 10
	request_sntp_time,			// 11
	synch_rtc,					// 12
	send_live_data,				// 13
	send_logs,					// 14
	receive_update,				// 15 (maybe not needed since it is a mqtt-sub)

	wait_for_response,			// 16
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

/* deprecated do not use */
mqtt_client_state_t get_mqtt_client_state(mqtt_client_t *client);
void set_mqtt_client_state(mqtt_client_t *client, mqtt_client_state_t new_state);

/* use this to control state machine from outside */
void fsm_job_queue_put(mqtt_client_state_t job_to_do);
mqtt_client_state_t fsm_job_queue_pop(void);


#endif /* DRIVERS_COMMUNICATION_ESP32AT_ESP32AT_H_ */
