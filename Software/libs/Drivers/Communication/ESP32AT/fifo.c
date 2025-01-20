#include "fifo.h"
#include <string.h>


void fifo_init(fifo_t* fifo){

}

void fifo_clear(fifo_t* fifo){
	fifo->head = 0;
	fifo->tail = 0;
	memset((char*) fifo->buffer, '\0', FIFO_BUFFER_SIZE);
}

void fifo_put_byte(fifo_t* fifo, uint8_t byte){
	fifo->buffer[fifo->head] = byte;
	fifo->head++;
}

uint8_t fifo_pop_byte(fifo_t* fifo){
	;;;
}
