#ifndef DRIVERS_COMMUNICATION_BG96AT_BG96AT_H_
#define DRIVERS_COMMUNICATION_BG96AT_BG96AT_H_

#include "Drivers/Communication/FIFO/fifo.h"

typedef enum{
	bg96_error = 0,
	bg96_ok,
	bg96_timeout,
	bg96_idle,
}bg96_mqtt_client_exit_code_t;

typedef enum{
	bg96_undefined = 0,
	bg96_standby,					// 1
	bg96_boot_up,					// 2
	bg96_connect_to_lte,			// 3
	bg96_connect_sntp,				// 4
	bg96_config_mqtt_broker1,		// 5
	bg96_config_mqtt_broker2,		// 6
	bg96_config_mqtt_broker3,		// 7
	bg96_config_mqtt_broker4,		// 8


	bg96_connect_mqtt_broker,	//
	bg96_subscribe_to_mqtt_msg,		//
	bg96_online,					//

	bg96_publish_mqtt_msg,			//
	bg96_publish_raw_mqtt_msg,		//
	bg96_request_sntp_time,			//
	bg96_synch_rtc,					//
	bg96_send_live_data,			//
	bg96_send_logs,					//
	bg96_receive_update,			//  (maybe not needed since it is a mqtt-sub)

	bg96_wait_for_response,			//
}bg96_mqtt_client_state_t;

typedef struct {
	/* public attributes */
	const char* sim_apn;
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
	bg96_mqtt_client_state_t next_state;	// next state aiming to after at_response success (nothing to do with state machine new_state)
}bg96_mqtt_client_t;

#endif /* DRIVERS_COMMUNICATION_BG96AT_BG96AT_H_ */
