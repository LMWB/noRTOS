#ifndef NORTOS_H_
#define NORTOS_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "utils.h"


#define NORTOS_VERSION_MAYOR 2
#define NORTOS_VERSION_MINOR 1

#define NORTOS_SCHEDULAR_GET_TICK()	GET_TICK() // returns tick in milliseconds
#define NORTOS_NO_OF_MAX_TASK		128

/* definition of general function pointer */
typedef void(*VoidFunctionPointer)(void);

/* definition of interrupt mask
 * typedef enum does NOT cost Flash or RAM compared to #define. enums for clarity and correctness, not macros.
 * */

typedef enum{
	eBIT_MASK_RESET_ALL 		= 0b00000000,
	eBIT_MASK_DI_INTERRUPT 		= 0b00000001,
	eBIT_MASK_UART_INTERRUPT 	= 0b00000010,
	eBIT_MASK_CAN_INTERRUPT 	= 0b00000100,
	eBIT_MASK_ADC_INTERRUPT		= 0b00001000,
	eBIT_MASK_USB_INTERRUPT		= 0b00010000,
	eBIT_MASK_RS485_INTERRUPT	= 0b00100000,
	eBIT_MASK_RESERVED4 		= 0b01000000,
	eBIT_MASK_RESERVED5 		= 0b10000000,
} interrupt_bit_mask;

typedef enum {
	eNORTOS_PERIODE_1ms 	= 1,
	eNORTOS_PERIODE_2ms 	= 2,
	eNORTOS_PERIODE_3ms 	= 3,
	eNORTOS_PERIODE_4ms 	= 4,
	eNORTOS_PERIODE_5ms 	= 5,
	eNORTOS_PERIODE_6ms 	= 6,
	eNORTOS_PERIODE_7ms 	= 7,
	eNORTOS_PERIODE_8ms 	= 8,
	eNORTOS_PERIODE_9ms 	= 9,
	eNORTOS_PERIODE_10ms 	= 10,
	eNORTOS_PERIODE_100ms 	= 100,
	eNORTOS_PERIODE_200ms 	= 200,
	eNORTOS_PERIODE_500ms 	= 500,

	eNORTOS_PERIODE_1s	= 1 * 1000,
	eNORTOS_PERIODE_5s 	= 5 * 1000,
	eNORTOS_PERIODE_10s	= 10 * 1000,
	eNORTOS_PERIODE_30s = 30 * 1000,

	eNORTOS_PERIODE_1min	= 60 * 1000,
	eNORTOS_PERIODE_2min	= eNORTOS_PERIODE_1min * 2,
	eNORTOS_PERIODE_10min	= eNORTOS_PERIODE_1min * 10,

	eNORTOS_PERIODE_1h		= 60 * 60 * 1000,
} task_delay_e;

typedef struct {
	uint32_t tick;
	task_delay_e delay;
	VoidFunctionPointer task_callback;
} noRTOS_task_t;

extern noRTOS_task_t* global_list_of_tasks[NORTOS_NO_OF_MAX_TASK];

/* adding callbacks to schedular */
bool noRTOS_add_task_to_scheduler(noRTOS_task_t *task);

/* runs once and once only just before scheduler */
void noRTOS_setup(void);

/* override this with your implementation */
void noRTOS_DIGITAL_INPUT_IRQ(void);

/* override this with your implementation */
void noRTOS_UART_RX_IRQ(void);

/* override this with your implementation */
void noRTOS_CAN_RX_IRQ(void);

/* override this with your implementation */
void noRTOS_ADC_IRQ(void);

void noRTOS_set_interrupt_received_flag(interrupt_bit_mask event_type);

void noRTOS_set_event_received_flag(interrupt_bit_mask event_type);

bool noRTOS_wait_for_event(			interrupt_bit_mask event_type);


/* runs forever and fires calbacks on a regular basis which have been
 * added using the noRTOS_add_task_to_scheduler() API */
void noRTOS_run_scheduler(void);

/* */
void noRTOS_print_version(void);

/* */
void noRTOS_print_active_task_running(void);

#endif /* NORTOS_H_ */
