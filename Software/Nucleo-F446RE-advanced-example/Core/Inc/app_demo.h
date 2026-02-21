#ifndef INC_APP_DEMO_H_
#define INC_APP_DEMO_H_
#include <stdbool.h>

#define UART_BUFFER_SIZE 128

extern uint8_t uart2_buffer[UART_BUFFER_SIZE];
extern uint8_t uart4_buffer[UART_BUFFER_SIZE];
extern uint8_t usb_RX_buffer[512];

extern uint8_t uart2_buffer_rx_size;
extern uint8_t uart4_buffer_rx_size;

void app_demo_main(void);

// todo: where to move ?
/* *************** USB VCOM Buffer *************** */
int vcom_write(char *ptr, int len);

/* *************** UART Byte Wise Interrupt Receiver *************** */
bool noRTOS_is_UART2_read_line_complete(void);
void noRTOS_UART2_read_byte_with_interrupt(void);
void noRTOS_UART2_clear_rx_buffer(void);
void noRTOS_UART2_echo_whats_been_received(void);
void noRTOS_UART2_receive_byte_callback(void);

#endif /* INC_APP_DEMO_H_ */
