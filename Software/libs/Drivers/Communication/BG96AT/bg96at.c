#include "bg96at.h"
#include "hardwareGlobal.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
/* braucht man die auch?
AT+CREG?
AT+CGATT?
AT+QCCID
AT+QMTDISC=0
*/

#define String char*
String BG96_RESET						= "AT+CFUN=1,1\r\n";
String BG96_QUERY_LTE_STATE				= "AT+COPS?\r\n";
String BG96_QUERY_LTE_SIGNAL_QUALITY 	= "AT+CSQ\r\n";
String BG96_QUERY_LTE_CONNECTION		= "AT+CEREG?\r\n";




String BG96_QUERY_SIM_STATE				= "AT+CPIN?\r\n";
String BG96_QUERY_SIM_ID				= "AT+QCCID\r\n";

String BG96_SET_API 				= "AT+QICSGP=1,\"IP\",\"data641003\"";
String BG96_QUERY_API 				= "AT+QICSGP=?";

String BG96_QUERY_IP 				= "AT+QIACT?\r\n";
String BG96_PING 					= "AT+QPING=1,\"8.8.8.8\",1,1\r\n";

String BG96_SET_SNTP_TIME 			= "AT+QNTP=1,\"zeit.fu-berlin.de\"\r\n";
String BG96_QUERY_TIMESTAMP 		= "AT+CCLK?\r\n";
String BG96_QUERY_SNTP_TIME 		= "";

String BG96_SET_MQTT_CONFIG1 		= "AT+QMTCFG=\"version\",0,4\r\n";
String BG96_SET_MQTT_CONFIG2 		= "AT+QMTCFG=\"ssl\",0,0\r\n";
String BG96_SET_MQTT_CONFIG3 		= "AT+QMTCFG=\"keepalive\",0,0\r\n";

String BG96_QUERY_MQTT_CONNECTION 	= "AT+QMTCONN?\r\n";
String BG96_SET_MQTT_CONNECTION 	= "AT+QMTOPEN=0,\"broker.hivemq.com\",1883\r\n";
String BG96_MQTT_CONNECT			= "AT+QMTCONN=0,\"testID2\"";
String BG96_CLOSE_MQTT_CONNECTION 	= "AT+QMTDISC=0\r\n";

#define TOTAL_TOPICS_SUBSCRIBE_TO 1
String BG96_SET_SUB_MQTT 			= "AT+QMTSUB=0,1,\"hello/world/room1/actor2\",1\r\n";
String BG96_SUB_REC_MQTT			= "+QMTRECV:";
String BG96_PUB_RAW_MQTT_PAYLOAD	= "das ist eine telemtry packet dummy\r\n";
//char BG96_PUB_RAW_MQTT_PAYLOAD[]	= {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

String BG96_PUB_MQTT 				= "AT+QMTPUBEX=0,0,0,0,\"hello/world/room1/actor1\",\"hallowelt\"\r\n";

void bg96_reset(){
	// toggle PERST-Pin
	//HAL_GPIO_WritePin(ESP32_RST_GPIO_Port, ESP32_RST_Pin, 0);
	//DELAY(200);
	//HAL_GPIO_WritePin(ESP32_RST_GPIO_Port, ESP32_RST_Pin, 1);
	// alternative send RST AT-Command
	//
}

void bg96_set_needle_for_response(bg96_mqtt_client_t* client, String at_response, uint32_t timeout){
	memcpy(client->at_response_to_be, at_response, strlen(at_response));
	client->timeout = timeout;
	client->timeout_start = GET_TICK();
}

void bg_connect_to_lte(bg96_mqtt_client_t* client){
	//BG96_QUERY_LTE_CONNECTION
}

void bg96_config_mqtt_connection_1(){
	UART_INTERNET_SEND( (uint8_t*)BG96_SET_MQTT_CONFIG1, strlen(BG96_SET_MQTT_CONFIG1));
}

void bg96_config_mqtt_connection_2(){
	UART_INTERNET_SEND( (uint8_t*)BG96_SET_MQTT_CONFIG2, strlen(BG96_SET_MQTT_CONFIG2));
}

void bg96_config_mqtt_connection_3(){
	UART_INTERNET_SEND( (uint8_t*)BG96_SET_MQTT_CONFIG3, strlen(BG96_SET_MQTT_CONFIG3));
}

void bg96_config_mqtt_connection_4(){
	UART_INTERNET_SEND( (uint8_t*)BG96_SET_MQTT_CONNECTION, strlen(BG96_SET_MQTT_CONNECTION));
}

void bg96_connect_to_mqtt_broker(){
	UART_INTERNET_SEND( (uint8_t*)BG96_MQTT_CONNECT, strlen(BG96_MQTT_CONNECT));
}

static bg96_mqtt_client_state_t state = bg96_boot_up;

bg96_mqtt_client_state_t bg96_get_mqtt_client_state(bg96_mqtt_client_t *client){
	return state;
}

void bg96_set_mqtt_client_state(bg96_mqtt_client_t *client, bg96_mqtt_client_state_t new_state){
	state = new_state;
}

void bg96_mqtt_client_fsm(bg96_mqtt_client_t *client) {
	bg96_mqtt_client_state_t new_state = bg96_standby;
	static bool fsm_is_online = false;

	switch (state) {

	case bg96_boot_up:
		printf("Reseting ESP32\n");
		fsm_is_online = false;
		bg96_reset();
		client->next_state = bg96_connect_to_lte;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "APP RDY\r\n", 0x3000);
		break;

	case bg96_connect_to_lte:
		// check connection first

		printf("connecting to LTE network\n");
		bg_connect_to_lte(client);
		client->next_state = bg96_connect_sntp;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "+CREG: 0,5\r\n\r\nOK\r\n", 0x3000);
		break;

	case bg96_connect_sntp:
		// check sntp state first

		printf("connecting to SNTP Server\n");
		esp32_connect_to_sntp();
		client->next_state = bg96_request_sntp_time;
		new_state = bg96_wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n+TIME_UPDATED\r\n", 3000);
		break;

	case bg96_request_sntp_time:
		printf("request SNTP time\n");
		esp32_query_sntp_time();
		esp32_set_needle_for_response(client, "OK\r\n", 3000);
		new_state = bg96_wait_for_response;
		client->next_state = bg96_synch_rtc;
		break;

	case bg96_synch_rtc:
		esp32_synch_host_rtc();
		if(fsm_is_online == false){
			// in regular boot phase use this path
			client->next_state	= bg96_config_mqtt_broker1;
			new_state 			= bg96_config_mqtt_broker1;
		}else{
			// on runtime when need to resynch RTC with atom-clock the mqtt broker
			// is already set up and the FSM can jump to online
			client->next_state	= bg96_online;
			new_state 			= bg96_online;
		}
		break;

	case bg96_config_mqtt_broker1:
		printf("Config MQTT Connection\n");
		bg96_config_mqtt_connection_1();
		client->next_state = bg96_config_mqtt_broker2;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case bg96_config_mqtt_broker2:
		printf("Config MQTT Connection\n");
		bg96_config_mqtt_connection_2();
		client->next_state = bg96_config_mqtt_broker3;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case bg96_config_mqtt_broker3:
		printf("Config MQTT Connection\n");
		bg96_config_mqtt_connection_3();
		client->next_state = bg96_config_mqtt_broker4;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case bg96_config_mqtt_broker4:
		printf("Config MQTT Connection\n");
		bg96_config_mqtt_connection_4();
		client->next_state = bg96_connect_mqtt_broker;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "OK\r\n\r\n+QMTOPEN: 0,0\r\n", 0x3000);
		break;


	case bg96_connect_mqtt_broker:
		printf("Connect to MQTT Broker\n");
		bg96_connect_to_mqtt_broker();
		client->next_state = bg96_subscribe_to_mqtt_msg;
		new_state = bg96_wait_for_response;
		bg96_set_needle_for_response(client, "OK\r\n\r\n+QMTCONN: 0,0,0\r\n", 0x3000);
		break;

	case bg96_subscribe_to_mqtt_msg:
		printf("Subscribe to topic\n");
		static uint8_t subscribes_done = 0;
		esp32_subscribe_to_topic(subscribes_done);
		subscribes_done += 1;
		if(subscribes_done < TOTAL_TOPICS_SUBSCRIBE_TO){
			client->next_state = bg96_subscribe_to_mqtt_msg;
		}else{
			client->next_state = bg96_online;
			subscribes_done = 0;
		}
		new_state = bg96_wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case bg96_online:
		printf("Online \n");
		fsm_is_online = true;
		bg96_mqtt_client_exit_code_t did_recieve_something = esp32_check_for_sub_receive(client);
		bg96_mqtt_client_state_t job = fsm_job_queue_pop();

		if (did_recieve_something == bg96_ok) {
			// todo
			// process mqtt received message

			// clear at-command rx-buffer
			fifo_clear(&client->at_fifo);

			// Continue in state machine
			client->next_state = bg96_online;
			new_state = bg96_online;
		} else if (job != 0) {
			//set_mqtt_client_state(client, job);
			client->next_state = job;
			new_state = job;
		} else {
			// stay online
			client->next_state = bg96_online;
			new_state = bg96_online;
		}
		break;

	case bg96_publish_mqtt_msg:
		printf("Publishing\n");
		esp32_publish_to_topic();
		client->next_state = bg96_online;
		new_state = bg96_wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case bg96_publish_raw_mqtt_msg:
		printf("Publishing Raw\n");
		esp32_publish_raw_to_topic(client);
		client->next_state = bg96_online;
		new_state = bg96_wait_for_response;
		esp32_set_needle_for_response(client, "+MQTTPUB:OK\r\n", 0x3000);
		break;

	case bg96_wait_for_response:
		bg96_mqtt_client_exit_code_t code = esp32_wait_for_response(client);
		if( code == bg96_ok){
			new_state = client->next_state;
		}else if ( code == bg96_timeout) {
			new_state = bg96_boot_up;
		}else if( code == bg96_error ){
			new_state = bg96_boot_up;
		}else{
			new_state = state;
		}
		break;

	default:
		printf("hit default switch-case: new state = boot_up \n");
		new_state = bg96_boot_up;
		break;
	}

	if (new_state != state) {
		state = new_state;
	}
}
