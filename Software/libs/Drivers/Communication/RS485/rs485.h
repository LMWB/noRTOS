#ifndef DRIVERS_COMMUNICATION_RS485_RS485_H_
#define DRIVERS_COMMUNICATION_RS485_RS485_H_

#include <stdbool.h>
#include <stdint.h>

#define RS485_BUFFER_SIZE 128

extern uint8_t rs485_buffer[RS485_BUFFER_SIZE];
extern uint8_t rs485_buffer_rx_size;

/* *************** UART Byte Wise Interrupt Receiver *************** */
bool noRTOS_is_UART2_read_line_complete(void);

void noRTOS_UART2_read_byte_with_interrupt(void);

void noRTOS_UART2_clear_rx_buffer(void);

void noRTOS_UART2_echo_whats_been_received(void);

void noRTOS_UART2_receive_byte_callback(void);


#endif /* DRIVERS_COMMUNICATION_RS485_RS485_H_ */
