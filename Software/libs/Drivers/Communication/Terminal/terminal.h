#ifndef DRIVERS_COMMUNICATION_TERMINAL_TERMINAL_H_
#define DRIVERS_COMMUNICATION_TERMINAL_TERMINAL_H_

#include <stdint.h>

#define TERMINAL_BUFFER_SIZE 128

extern uint8_t terminal_buffer[TERMINAL_BUFFER_SIZE];
extern uint8_t terminal_buffer_rx_size;

void terminal_loop_back(char* buf);

#endif /* DRIVERS_COMMUNICATION_TERMINAL_TERMINAL_H_ */
