#include "esp32at.h"
#include "hardwareGlobal.h"
#include <stdio.h>
#include <string.h>

/* Temporaer */
uint8_t uart_rx_buffer_internet[uart_rx_buffer_size] = {0};
uint16_t head = 0;
uint16_t tail = 0;
uint8_t uart_rx_buffer_terminal[uart_rx_buffer_size] = {0};
/* Temporaer */


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
//String ESP32_SET_AP_CONNECTION 		= "AT+CWJAP=\"VodafoneMobileWiFi-6B18C9\",\"wGkH536785\"\r\n";
//String ESP32_SET_AP_CONNECTION 		= "AT+CWJAP=\"hot-spot\",\"JKp8636785\"\r\n";
String ESP32_SET_AP_CONNECTION 		= "AT+CWJAP=\"iPhone11\",\"abc123456\"\r\n";
String ES32_QUERY_IP 				= "AT+CIFSR\r\n";

String ESP32_PING 					= "AT+PING=\"www.google.de\"\r\n";

String ESP32_SET_SNTP_TIME 			= "AT+CIPSNTPCFG=1,0,\"zeit.fu-berlin.de\"\r\n";
String ESP32_QUERY_TIMESTAMP 		= "AT+SYSTIMESTAMP?\r\n";
String ESP32_QUERY_SNTP_TIME 		= "AT+CIPSNTPTIME?\r\n";

String ESP32_SET_MQTT_CONFIG 		= "AT+MQTTUSERCFG=0,1,\"sc36ClientID\",\"emqx\",\"public\",0,0,\"\"\r\n";
String ESP32_QUERY_MQTT_CONNECTION 	= "AT+MQTTCONN?\r\n";
String ESP32_SET_MQTT_CONNECTION 	= "AT+MQTTCONN=0,\"broker.emqx.io\",1883,1\r\n";
String ESP32_CLOSE_MQTT_CONNECTION 	= "AT+MQTTCLEAN=0\r\n";

String ESP32_PUB_MQTT 				= "AT+MQTTPUB=0,\"topic008\",\"hallo broker new fsm\",1,0\r\n";
String ESP32_PUB_RAW_MQTT 			= "AT+MQTTPUBRAW=<LinkID>,<\"topic\">,<length>,<qos>,<retain>\r\n";
String ESP32_QUERY_SUB_MQTT 		= "AT+MQTTSUB?\r\n";
String ESP32_SET_SUB_MQTT 			= "AT+MQTTSUB=0,\"topic007\",1\r\n";



void esp32_reset(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_RESET, strlen(ESP32_RESET));
}

void esp32_check_wifi_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_QUERY_WIFI_CONNECTION, strlen(ESP32_QUERY_WIFI_CONNECTION) );
}

void esp32_connect_to_wifi(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_AP_CONNECTION, strlen(ESP32_SET_AP_CONNECTION));
}

void esp32_connect_to_sntp(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_SNTP_TIME, strlen(ESP32_SET_SNTP_TIME));
}

void esp32_config_mqtt_connection(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONFIG, strlen(ESP32_SET_MQTT_CONFIG));
}

void esp32_connect_to_mqtt_broker(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_MQTT_CONNECTION, strlen(ESP32_SET_MQTT_CONNECTION));
}

void esp32_subscribe_to_topic(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_SET_SUB_MQTT, strlen(ESP32_SET_SUB_MQTT));
}
void esp32_publish_to_topic(){
	UART_INTERNET_SEND( (uint8_t*)ESP32_PUB_MQTT, strlen(ESP32_PUB_MQTT));
}




void esp32_set_needle_for_response(client_fsm_t* client, String at_response, uint32_t timeout){
	memcpy(client->at_response_to_be, at_response, strlen(at_response));
	client->timeout = timeout;
}

esp32_exit_code_t esp32_wait_for_response(client_fsm_t* client){
	uint32_t tic = GET_TICK();

	// if timeout got hit
	if( (tic - client->timeout_start) >= client->timeout){
		printf("\t[debug] - timeout\n");
		return esp32_timeout;
	}

	// check if needle was found
	char *ret = strstr((char*)client->at_fifo.buffer, client->at_response_to_be);
	if( ret !=  NULL ){
		memset(client->at_response_to_be, '\0', 128);
		fifo_clear(&client->at_fifo);
		printf("\t[debug] - hit needle\n");
		return esp32_ok;
	}

	// if AT command "ERROR" was found
	ret = strstr((char*)client->at_fifo.buffer, "ERROR\r\n");

	if( ret != NULL){
		memset(client->at_response_to_be, '\0', 128);
		fifo_clear(&client->at_fifo);
		printf("\t[debug] - hit error\n");
		return esp32_error;
	}

	// else wait for next call
	return esp32_idle;
}

MQTT_client_t get_fsm_state(client_fsm_t *client){
	return client->next_state;
}

void set_fsm_state(client_fsm_t *client, MQTT_client_t new_state){
	client->next_state = new_state;
}

void esp32_mqtt_fsm(client_fsm_t *client) {
	static MQTT_client_t state = boot_up;
	MQTT_client_t new_state = standby;

	switch (state) {
	case undefined:
		new_state = boot_up;
		break;

	case standby:
		new_state = standby;
		break;

	case boot_up:
		printf("Reseting ESP32\n");
		esp32_reset();
		client->next_state = connect_to_wifi;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "ready\r\n", 0xFFFF);
		break;

	case connect_to_wifi:
		printf("connecting to Wifi\n");
		esp32_connect_to_wifi();
		client->next_state = config_connection_to_broker;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "WIFI GOT IP\r\n\r\nOK\r\n", 0xFFFF);
		break;

	case config_connection_to_broker:
		printf("Config MQTT Connection\n");
		esp32_config_mqtt_connection();
		client->next_state = connect_to_mqtt_broker;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case connect_to_mqtt_broker:
		printf("Connect to MQTT Broker\n");
		esp32_connect_to_mqtt_broker();
		client->next_state = subscribe_to_mqtt_msg;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case subscribe_to_mqtt_msg:
		printf("Subscribe to topic\n");
		esp32_subscribe_to_topic();
		client->next_state = online;
		new_state = wait_for_response;
		esp32_set_needle_for_response(client, "OK\r\n", 0xFFFF);
		break;

	case online:
		printf("Online \n");
		client->next_state = online;
		new_state = online;
		break;

	case publish_mqtt_msg:
		printf("Publishing\n");
		esp32_publish_to_topic();
		new_state = online;
		break;

	case wait_for_response:
		esp32_exit_code_t code = esp32_wait_for_response(client);
		if( code == esp32_ok){
			new_state = client->next_state;
		}else if ( code == esp32_timeout) {
			new_state = boot_up;
		}else if( code == esp32_error ){
			new_state = state;
		}else{
			new_state = state;
		}
		break;

	default:
		break;
	}

	if (new_state != state) {
		state = new_state;
	}
}




