#ifndef DRIVERS_COMMUNICATION_ESP32AT_FIFO_H_
#define DRIVERS_COMMUNICATION_ESP32AT_FIFO_H_
#include <stdint.h>

#define __1KByte__ 1024
#define FIFO_BUFFER_SIZE (__1KByte__)

typedef struct _fifo{
	uint8_t buffer[FIFO_BUFFER_SIZE];
	uint16_t head;
	uint16_t tail;
}fifo_t;


void fifo_init(fifo_t* fifo);
void fifo_clear(fifo_t* fifo);

void fifo_put_byte(fifo_t* fifo, uint8_t byte);
uint8_t fifo_pop_byte(fifo_t* fifo);

void fifo_put_string(fifo_t* fifo, uint8_t* string);
uint8_t* fifo_pop_string(fifo_t* fifo);


#endif /* DRIVERS_COMMUNICATION_ESP32AT_FIFO_H_ */
