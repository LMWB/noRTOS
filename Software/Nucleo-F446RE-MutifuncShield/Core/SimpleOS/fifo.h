#ifndef FIFO_H_INCLUDED
#define FIFO_H_INCLUDED

#include <main.h>
#include "simpleOS.h"


// Opaque circular buffer structure
typedef struct __circular_buf_t
{
	uint8_t head;
	uint8_t tail;
	uint8_t size;
	uint8_t full;

	// das muss noch verbessert werden auf universal typ (void pointer)
	nonperiodic_task_s *buffer;       /* data structure which is to be manages by the circular ring buffer
                                        signature (typedef or datatype) has to be adjusted to the buffer hans the payload */
}circular_buf_t;

// Handle type, the way users interact with the API
// bracht man vllt. gar nicht so in dieser art und weise
typedef circular_buf_t* cbuf_handle_t;

// Pass in a storage buffer and size
void circular_buf_init(cbuf_handle_t me, nonperiodic_task_s* buffer, uint8_t size);

// Free a circular buffer structure.
// Does not free data buffer; owner is responsible for that
void circular_buf_free(cbuf_handle_t me);

// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t me);

// Put version 1 continues to add data if the buffer is full
// Old data is overwritten
void circular_buf_put(cbuf_handle_t me, nonperiodic_task_s new_element);

// Put Version 2 rejects new data if the buffer is full
// Returns 0 on success, -1 if buffer is full
int8_t circular_buf_put2(cbuf_handle_t me, func_ptr new_function);

// Retrieve a value from the buffer
// Returns 0 on success, -1 if the buffer is empty
int8_t circular_buf_get(cbuf_handle_t me, nonperiodic_task_s *return_element);
//func_ptr circular_buf_get(cbuf_handle_t me);

// Returns true if the buffer is empty
uint8_t circular_buf_empty(cbuf_handle_t me);

// Returns true if the buffer is full
uint8_t circular_buf_full(cbuf_handle_t me);

// Returns the maximum capacity of the buffer
uint8_t circular_buf_capacity(cbuf_handle_t me);

// Returns the current number of elements in the buffer
uint8_t circular_buf_size(cbuf_handle_t me);

#endif // FIFO_H_INCLUDED
