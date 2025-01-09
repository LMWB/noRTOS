#include "utils.h"


/* override _write, is used by puts and printf */
int _write(int file, char *ptr, int len)
{
    UART_TERMINAL_SEND((uint8_t*) ptr, (uint16_t)len);
	return len;
}


uint32_t raw_buffer_to_hex_string(const uint8_t *buffer, size_t buffer_size, char *hex_string) {
	uint32_t bytes_written = 0;
	for (uint_fast32_t i = 0; i < buffer_size; i++) {
		bytes_written += sprintf(hex_string + 2 * i, "%02X", buffer[i]);
	}
	hex_string[2 * buffer_size] = '\0';

	return bytes_written + 1;
}


void myprintf(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int len = strlen(buffer);
	UART_TERMINAL_SEND((uint8_t *) buffer, len);
}

void scan_i2c_sensors(void) {
	myprintf("Scanning I2C bus...\n");

	DEVICE_STATUS_DEFINITION res;
	for (uint16_t i = 0; i < 128; i++) {
		res = IS_I2C_DEVICE_READY(i << 1);
		if( i%32 == 0){
			myprintf("\n");
		}
		if (res == DEVICE_OK) {
			myprintf("0x%02X", i);
		} else {
			myprintf(".");
		}
		DELAY(20);
	}

	myprintf("\r\n");
	DELAY(1000);
}
