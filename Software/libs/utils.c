#include "utils.h"


/* override _write, is used by puts and printf */
int _write(int file, char *ptr, int len)
{
    UART_SEND_TERMINAL((uint8_t*) ptr, (uint16_t)len);
	return len;
}
