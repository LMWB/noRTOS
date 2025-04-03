#include "fifo.h"
#include <string.h>


void fifo_init(fifo_t* fifo){

}

void fifo_clear(fifo_t* fifo){
	fifo->head = 0;
	fifo->tail = 0;
	memset((char*) fifo->buffer, '\0', FIFO_BUFFER_SIZE);
}

static uint16_t get_fill_level(fifo_t* fifo){
	if(fifo->head == fifo->tail){
		return 0;
	}else if(fifo->head > fifo->tail){
		return fifo->head-fifo->tail;
	}else{
		return (FIFO_BUFFER_SIZE + fifo->head - fifo->tail);
	}
}

static void increment_head(fifo_t* fifo){
	fifo->head += 1;
	if(fifo->head > (FIFO_BUFFER_SIZE-1)){
		fifo->head = 0;
	}
}

static void increment_tail(fifo_t *fifo) {
	fifo->tail += 1;
	if (fifo->tail > (FIFO_BUFFER_SIZE-1)) {
		fifo->tail = 0;
	}
}

void fifo_put_byte(fifo_t* fifo, uint8_t byte){
	fifo->buffer[fifo->head] = byte;
	increment_head(fifo);
}

uint8_t fifo_pop_byte(fifo_t* fifo){
	if(get_fill_level(fifo) != 0){
		uint8_t byte = fifo->buffer[fifo->tail];
		fifo->buffer[fifo->tail] = '\0';
		increment_tail(fifo);
		return byte;
	}
	return 0;
}


void fifo_put_string(fifo_t* fifo, uint8_t* string){
	;
}

uint8_t* fifo_pop_string(fifo_t* fifo){
	return NULL;
}
