#include "esp32at.h"
#include "hardwareGlobal.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

/* MQTT Connection Example with web based client
 * https://mqttx.app/web-client#/recent_connections/8dc002fe-dc48-4926-8760-bde4bbb4c859
 * https://www.emqx.com/en/blog/mqtt-client-tools
 * */

// AT+MQTTUSERCFG=0,7,"sc36ClientID","emqx","public",0,0,"mqtt" (uses WSS scheme 7: MQTT over WebSocket Secure (based on TLS, no certificate verify).
// AT+MQTTCONN=0,"broker.emqx.io",8084,1
// AT+MQTTSUB=0,"topic007",1
// AT+MQTTPUB=0,"topic008","hallo broker",1,0

/* another MQTT Connection Example with web based client */
// AT+MQTTUSERCFG=0,1,"sc36ClientID","emqx","public",0,0,"" (uses TCP sechme 1: MQTT over TCP)
// AT+MQTTCONN=0,"broker.emqx.io",1883,1
// AT+MQTTSUB=0,"topic007",1
// AT+MQTTPUB=0,"topic008","hallo broker",1,0



/* Constants to be used in ESP32 MQTT State Machine */
#define String char*
String ESP32_RESET					= "AT+RST\r\n";
String ESP32_QUERY_WIFI_STATE		= "AT+CWSTATE?\r\n";
String ESP32_QUERY_WIFI_CONNECTION	= "AT+CWJAP?\r\n";

String ESP32_SET_AP_CONNECTION 		= "AT+CWJAP=\"hot-spot\",\"JKp8636785\"\r\n";

String ES32_QUERY_IP 				= "AT+CIFSR\r\n";
String ESP32_PING 					= "AT+PING=\"www.google.de\"\r\n";

String ESP32_SET_SNTP_TIME 			= "AT+CIPSNTPCFG=1,0,\"zeit.fu-berlin.de\"\r\n";
String ESP32_QUERY_TIMESTAMP 		= "AT+SYSTIMESTAMP?\r\n";
String ESP32_QUERY_SNTP_TIME 		= "AT+CIPSNTPTIME?\r\n";

String ESP32_SET_MQTT_CONFIG 		= "AT+MQTTUSERCFG=0,1,\"sc36ClientID\",\"emqx\",\"public\",0,0,\"\"\r\n";
String ESP32_QUERY_MQTT_CONNECTION 	= "AT+MQTTCONN?\r\n";
String ESP32_SET_MQTT_CONNECTION 	= "AT+MQTTCONN=0,\"broker.emqx.io\",1883,1\r\n";
String ESP32_CLOSE_MQTT_CONNECTION 	= "AT+MQTTCLEAN=0\r\n";

String ESP32_PUB_MQTT 				= "AT+MQTTPUB=0,\"jiKEBRupldriwltofPUB/trigonometry\",\"hallo broker new fsm\",1,0\r\n";
String ESP32_PUB_RAW_MQTT 			= "AT+MQTTPUBRAW=0,\"jiKEBRupldriwltofPUB/trigonometry\",40,1,0\r\n";
//String ESP32_PUB_RAW_MQTT_PAYLOAD	= "das ist eine telemtry packet dummy\r\n";
char ESP32_PUB_RAW_MQTT_PAYLOAD[]	= {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

String ESP32_QUERY_SUB_MQTT 		= "AT+MQTTSUB?\r\n";
String ESP32_SET_SUB_MQTT 			= "AT+MQTTSUB=0,\"jiKEBRupldriwltofSUB/trigonometry\",1\r\n";
String ESP32_SUB_REC_MQTT			= "+MQTTSUBRECV:";
#define TOTAL_TOPICS_SUBSCRIBE_TO 5
String sub_topics[TOTAL_TOPICS_SUBSCRIBE_TO] = {
		"jiKEBRupldriwltofSUB1/button", "jiKEBRupldriwltofSUB2/slider",
		"jiKEBRupldriwltofSUB3/raw", "jiKEBRupldriwltofSUB4/bin", "jiKEBRupldriwltofSUB5/string"};

static char at_working_buffer[1024];

void esp32_reset(){
	HAL_GPIO_WritePin(ESP32_RST_GPIO_Port, ESP32_RST_Pin, 0);
	DELAY(200);
	HAL_GPIO_WritePin(ESP32_RST_GPIO_Port, ESP32_RST_Pin, 1);
	// alternative send RST AT-Command
	//UART_INTERNET_SEND( (uint8_t*)ESP32_RESET, strlen(ESP32_RESET));
}

void esp32_check_wifi_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_QUERY_WIFI_CONNECTION, strlen(ESP32_QUERY_WIFI_CONNECTION) );
}

void esp32_connect_to_wifi(mqtt_client_t *client){
	uint16_t size = sprintf(at_working_buffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", client->wifi_ssid, client->wifi_password);
	UART_INTERNET_SEND( (uint8_t*)at_working_buffer, size);
}

void esp32_connect_to_sntp(void){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_SNTP_TIME, strlen(ESP32_SET_SNTP_TIME));
}

void esp32_query_epochtime(void){
	UART_INTERNET_SEND( (uint8_t*)ESP32_QUERY_TIMESTAMP, strlen(ESP32_QUERY_TIMESTAMP));
}

void esp32_query_sntp_time(void){
	UART_INTERNET_SEND( (uint8_t*)ESP32_QUERY_SNTP_TIME, strlen(ESP32_QUERY_SNTP_TIME));
}

void esp32_synch_host_rtc(void){
	char* unix_time = strstr(at_working_buffer, "+CIPSNTPTIME:");
	printf("Date / Time is: %s", unix_time+13);
	struct tm time_date;
	time_t epoch = cvt_asctime( (unix_time+13), &time_date);
	epoch = epoch;
	set_gmtime_stm32(&time_date);
}

void esp32_config_mqtt_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONFIG, strlen(ESP32_SET_MQTT_CONFIG));
}

void esp32_connect_to_mqtt_broker(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONNECTION, strlen(ESP32_SET_MQTT_CONNECTION));
}

void esp32_subscribe_to_topic(uint8_t topic_in_list){
	uint16_t size = sprintf(at_working_buffer, "AT+MQTTSUB=0,\"%s\",1\r\n", sub_topics[topic_in_list]);
	UART_INTERNET_SEND( (uint8_t*)at_working_buffer, size);
}

mqtt_client_exit_code_t esp32_check_for_sub_receive(mqtt_client_t *client) {
	// check if needle was found
	char *ret = strstr((char*) client->at_fifo.buffer, ESP32_SUB_REC_MQTT);

	if (ret != NULL) {
		// debug messages
		printf("\t[debug] - hit needle\n");
		// echo to terminal
		UART_TERMINAL_SEND(client->at_fifo.buffer, client->at_fifo.head);
		return esp32_ok;
	}
	return esp32_idle;
}

void esp32_publish_to_topic(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_PUB_MQTT, strlen(ESP32_PUB_MQTT));
}

void esp32_publish_raw_to_topic(mqtt_client_t* client){
	uint16_t size = sprintf(at_working_buffer,
			"AT+MQTTPUBRAW=0,\"jiKEBRupldriwltofPUB/trigonometry\",%d,1,0\r\n",
			client->at_pub_payload_size);
	UART_INTERNET_SEND( (uint8_t*)at_working_buffer, size);
	DELAY(100);
	UART_INTERNET_SEND( (uint8_t*)client->at_pub_payload, client->at_pub_payload_size);
}

void esp32_set_needle_for_response(mqtt_client_t* client, String at_response, uint32_t timeout){
	memcpy(client->at_response_to_be, at_response, strlen(at_response));
	client->timeout = timeout;
	client->timeout_start = GET_TICK();
}

void esp32_clear_needle_for_response(mqtt_client_t* client){
	memset(client->at_response_to_be, '\0', 128);
}

void esp32_preserve_response(mqtt_client_t* client){
	uint16_t size = client->at_fifo.head;
	memcpy(at_working_buffer, &client->at_fifo.buffer, size);
	// don't forget null terminator at end of string
	at_working_buffer[size] = '\0';
}

mqtt_client_exit_code_t esp32_wait_for_response(mqtt_client_t* client){
	uint32_t tic = GET_TICK();

	// time out must be reworked
	if( (tic - client->timeout_start) >= client->timeout){
		printf("\t[debug] - timeout\n");
		// echo to terminal what has been received so far
		UART_TERMINAL_SEND(client->at_fifo.buffer, client->at_fifo.head);

		esp32_clear_needle_for_response(client);

		// clear at-command rx-buffer
		fifo_clear(&client->at_fifo);
		return esp32_timeout;
	}

	// check if needle was found
	char *ret = strstr((char*)client->at_fifo.buffer, client->at_response_to_be);
	if( ret !=  NULL ){
		// debug messages
		printf("\t[debug] - hit needle\n");
		// echo to terminal
		UART_TERMINAL_SEND(client->at_fifo.buffer, client->at_fifo.head);

		esp32_clear_needle_for_response(client);
		esp32_preserve_response(client);

		// clear at-command rx-buffer
		fifo_clear(&client->at_fifo);
		return esp32_ok;
	}

	// if AT command "ERROR" was found
	ret = strstr((char*)client->at_fifo.buffer, "ERROR\r\n");
	if( ret != NULL){
		printf("\t[debug] - hit error\n");
		// echo to terminal
		UART_TERMINAL_SEND(client->at_fifo.buffer, client->at_fifo.head);

		esp32_clear_needle_for_response(client);

		// clear at-command rx-buffer
		fifo_clear(&client->at_fifo);
		return esp32_error;
	}

	// else wait for next call
	return esp32_idle;
}

static fifo_t fsm_job_fifo = {.head = 0, .tail = 0};

void fsm_job_queue_put(mqtt_client_state_t job_to_do){
	fifo_put_byte(&fsm_job_fifo, (uint8_t) job_to_do);
}

mqtt_client_state_t fsm_job_queue_pop(void){
	uint8_t byte = fifo_pop_byte(&fsm_job_fifo);
	return (mqtt_client_state_t) byte;
}

static mqtt_client_state_t state = boot_up;

mqtt_client_state_t get_mqtt_client_state(mqtt_client_t *client){
	return state;
}

void set_mqtt_client_state(mqtt_client_t *client, mqtt_client_state_t new_state){
	state = new_state;
}

void mqtt_client_fsm(mqtt_client_t *client) {
	mqtt_client_state_t new_state = standby;
	static bool fsm_is_online = false;

	switch (state) {

	case boot_up:
		printf("Reseting ESP32\n");
		fsm_is_online = false;
		esp32_reset();
		client->next_state = connect_to_wifi;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "ready\r\n", 0x3000);
		break;

	case connect_to_wifi:
		printf("connecting to Wifi\n");
		esp32_connect_to_wifi(client);
		client->next_state = connect_sntp;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "WIFI GOT IP\r\n\r\nOK\r\n", 0x3000);
		break;

	case connect_sntp:
		printf("connecting to SNTP Server\n");
		esp32_connect_to_sntp();
		client->next_state = request_sntp_time;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n+TIME_UPDATED\r\n", 3000);
		break;

	case request_sntp_time:
		printf("request SNTP time\n");
		esp32_query_sntp_time();
		esp32_set_needle_for_response(client, "OK\r\n", 3000);
		new_state = wait_for_response;
		client->next_state = synch_rtc;
		break;

	case synch_rtc:
		esp32_synch_host_rtc();
		if(fsm_is_online == false){
			// in regular boot phase use this path
			client->next_state	= config_connection_to_broker;
			new_state 			= config_connection_to_broker;
		}else{
			// on runtime when need to resynch RTC with atom-clock the mqtt broker
			// is already set up and the FSM can jump to online
			client->next_state	= online;
			new_state 			= online;
		}
		break;

	case config_connection_to_broker:
		printf("Config MQTT Connection\n");
		esp32_config_mqtt_connection();
		client->next_state = connect_to_mqtt_broker;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case connect_to_mqtt_broker:
		printf("Connect to MQTT Broker\n");
		esp32_connect_to_mqtt_broker();
		client->next_state = subscribe_to_mqtt_msg;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case subscribe_to_mqtt_msg:
		printf("Subscribe to topic\n");
		static uint8_t subscribes_done = 0;
		esp32_subscribe_to_topic(subscribes_done);
		subscribes_done += 1;
		if(subscribes_done < TOTAL_TOPICS_SUBSCRIBE_TO){
			client->next_state = subscribe_to_mqtt_msg;
		}else{
			client->next_state = online;
			subscribes_done = 0;
		}
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case online:
		printf("Online \n");
		fsm_is_online = true;
		mqtt_client_exit_code_t did_recieve_something = esp32_check_for_sub_receive(client);
		mqtt_client_state_t job = fsm_job_queue_pop();

		if (did_recieve_something == esp32_ok) {
			// todo
			// process mqtt received message

			// clear at-command rx-buffer
			fifo_clear(&client->at_fifo);

			// Continue in state machine
			client->next_state = online;
			new_state = online;
		} else if (job != 0) {
			//set_mqtt_client_state(client, job);
			client->next_state = job;
			new_state = job;
		} else {
			// stay online
			client->next_state = online;
			new_state = online;
		}
		break;

	case publish_mqtt_msg:
		printf("Publishing\n");
		esp32_publish_to_topic();
		client->next_state = online;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0x3000);
		break;

	case publish_raw_mqtt_msg:
		printf("Publishing Raw\n");
		esp32_publish_raw_to_topic(client);
		client->next_state = online;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "+MQTTPUB:OK\r\n", 0x3000);
		break;

	case wait_for_response:
		mqtt_client_exit_code_t code = esp32_wait_for_response(client);
		if( code == esp32_ok){
			new_state = client->next_state;
		}else if ( code == esp32_timeout) {
			new_state = boot_up;
		}else if( code == esp32_error ){
			new_state = boot_up;
		}else{
			new_state = state;
		}
		break;

	default:
		printf("hit default switch-case: new state = boot_up \n");
		new_state = boot_up;
		break;
	}

	if (new_state != state) {
		state = new_state;
	}
}




