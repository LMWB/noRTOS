#include "terminal.h"


uint8_t terminal_buffer[TERMINAL_BUFFER_SIZE] = {0};
uint8_t terminal_buffer_rx_size = 0;

void terminal_loop_back(char* buf){
	printf("loopback %s", buf);
}
