#ifndef INC_APP_DEMO_H_
#define INC_APP_DEMO_H_
#include <stdbool.h>

#define UART_BUFFER_SIZE 128

extern uint8_t uart2_buffer[UART_BUFFER_SIZE];
extern uint8_t usb_RX_buffer[512];

extern uint8_t uart2_buffer_rx_size;

void app_demo_main(void);

// todo: where to move ?
/* *************** USB VCOM Buffer *************** */
int vcom_write(char *ptr, int len);

#endif /* INC_APP_DEMO_H_ */
