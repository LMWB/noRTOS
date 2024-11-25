#include "fifo.h"

/*
* https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/
*/


void circular_buf_init(cbuf_handle_t me, nonperiodic_task_s* buffer, uint8_t size)
{
    circular_buf_reset(me);

    me->buffer = buffer;
    me->size = size;
}


void circular_buf_reset(cbuf_handle_t me)
{
    me->head = 0;
    me->tail = 0;
    me->full = 0;
}


void circular_buf_free(cbuf_handle_t me)
{
    // to do when we know how circular_buf_init looks like
    ;;;
}

uint8_t circular_buf_full(cbuf_handle_t me)
{
	return me->full;
}


uint8_t circular_buf_empty(cbuf_handle_t me)
{

	return (!me->full && (me->head == me->tail));
}

uint8_t circular_buf_capacity(cbuf_handle_t me)
{
	return me->size;
}

uint8_t circular_buf_size(cbuf_handle_t me)
{
	uint8_t size = me->size;

	if(!me->full)
	{
		if(me->head >= me->tail)
		{
			size = (me->head - me->tail);
		}
		else
		{
			size = (me->size + me->head - me->tail);
		}
	}

	return size;
}


static void increment_pointer(cbuf_handle_t me)
{
	if(me->full)
   	{
		if (++(me->tail) == me->size)
		{
			me->tail = 0;
		}
	}

	if (++(me->head) == me->size)
	{
		me->head = 0;
	}
	me->full = (me->head == me->tail);
}

static void decrement_pointer(cbuf_handle_t me)
{

	me->full = 0;
	if (++(me->tail) == me->size)
	{
		me->tail = 0;
	}
}

// If the buffer is full, the oldest value will be overwritten. This is the standard use case for a circular buffer
void circular_buf_put(cbuf_handle_t me, nonperiodic_task_s new_element)
{

    me->buffer[me->head] = new_element;

    /* temp. pointer for better reading */
    //nonperiodic_task_s *p = me->buffer;

    /* pointer arithmetics */
    //(p+me->head)->flag = 1;
    //(p+me->head)->task_ptr = new_function;


    increment_pointer(me);
}


// returns an error if the buffer is full. This is provided for demonstration purposes, but we do not use this variant in our systems.
int8_t circular_buf_put2(cbuf_handle_t me, func_ptr new_function)
{
    int r = -1;

    if(!circular_buf_full(me))
    {
        //me->buffer[me->head] = data;
        increment_pointer(me);
        r = 0;
    }

    return r;
}


int8_t circular_buf_get(cbuf_handle_t me, nonperiodic_task_s *return_element)
//func_ptr circular_buf_get(cbuf_handle_t me)
{
    int r = -1;
    //func_ptr foo = NULL;

    if(!circular_buf_empty(me))
    {
        *return_element = me->buffer[me->tail];

        /* temp. pointer for better reading */
        //nonperiodic_task_s *p = me->buffer;

        /* pointer arithmetics return *fp */
        //foo = (p+me->tail)->task_ptr;

        /* dierekt ausfuehren funktioniert, aber wieso funktioniert die rueckbage nicht */
        //(p+me->tail)->task_ptr();

        /* remove function from buffer, not really necessary */
        //(p+me->tail)->flag = 0;
        //(p+me->tail)->task_ptr = NULL;
        nonperiodic_task_s temp = {0, NULL};
        me->buffer[me->tail] = temp;

        decrement_pointer(me);

        r = 0;
    }

    return r;
    //return foo;
}

