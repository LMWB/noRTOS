#include "rs485.h"
#include "platformGlue.h"
#include <string.h>


/* *************** UART Byte Wise Interrupt Receiver *************** */
uint8_t rs485_buffer[RS485_BUFFER_SIZE] = {0};
uint8_t rs485_buffer_rx_size = 0;

static uint16_t rx_size = 0;
static bool uart2_read_line_complete = false;

bool noRTOS_is_UART2_read_line_complete(void){
	return uart2_read_line_complete;
}

// todo remove &uart2
void noRTOS_UART2_read_byte_with_interrupt(void){
	HAL_UART_Receive_IT(&huart2, &rs485_buffer[rx_size], 1);
}

void noRTOS_UART2_clear_rx_buffer(void){
	rx_size = 0;
	uart2_read_line_complete = false;
	memset(rs485_buffer, 0, RS485_BUFFER_SIZE);
	noRTOS_UART2_read_byte_with_interrupt();
}

void noRTOS_UART2_echo_whats_been_received(void)
{
	UART_TERMINAL_SEND( rs485_buffer, rx_size);
}

void noRTOS_UART2_receive_byte_callback(void){
	/* check if read line complete */
	if(rx_size >= 3 && rs485_buffer[rx_size-1] == '\r' && rs485_buffer[rx_size] == '\n'){
		rx_size++;
		uart2_read_line_complete = true;
	}else{
		rx_size++;
		noRTOS_UART2_read_byte_with_interrupt();
	}
}
