#include "noRTOS.h"
#include "platformGlue.h"
#include <string.h>



/* override this with your implementation */
__weak void noRTOS_setup(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_DIGITAL_INPUT_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_UART_RX_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_RS485_RX_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_CAN_RX_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_ADC_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_USB_IRQ(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_RESERVED_IRQ(void){
	;
}


/**
 * *************** Task Schedular ***************
 */

noRTOS_task_t *global_list_of_tasks[NORTOS_NO_OF_MAX_TASK];
static uint32_t number_of_active_task = 0;
static uint8_t noRTOS_interrupt_flag = 0;
static uint8_t noRTOS_event_flag = 0;

/* (private) local getter / setter */
static void set_bit_in_byte(const uint8_t bit, uint8_t* byte){
	*byte |= bit;
}

static void clear_bit_in_byte(const uint8_t bit, uint8_t* byte){
	*byte &= ~bit;
}

// preparation in case we need it one time
//static void clear_all_bits_in_byte(uint8_t* byte){
//	*byte = 0;
//}

/* public */
void noRTOS_set_interrupt_received_flag(interrupt_bit_mask event_type){
	set_bit_in_byte(event_type, &noRTOS_interrupt_flag);
}

void noRTOS_set_event_received_flag(interrupt_bit_mask event_type){
	set_bit_in_byte(event_type, &noRTOS_event_flag);
}

/* blue print  for more callbacks of this kind */
bool noRTOS_wait_for_event(interrupt_bit_mask event_type) {
	if (noRTOS_event_flag & event_type) {
		clear_bit_in_byte(event_type, &noRTOS_event_flag);
		return true;
	}
	return false;
}


void noRTOS_print_version(void){
	printf("booting from noRTOS Version: %d.%d\n", NORTOS_VERSION_MAYOR, NORTOS_VERSION_MINOR);
}

void noRTOS_print_active_task_running(void){
	printf("starting noRTOS scheduler with %ld task in queue\n", number_of_active_task);
}

bool noRTOS_add_task_to_scheduler(noRTOS_task_t *task){
	if(number_of_active_task < NORTOS_NO_OF_MAX_TASK){
		global_list_of_tasks[number_of_active_task] = task;
		number_of_active_task++;
		return true;
	}
	return false;
}


/*
 * this methods operates the INCOMMING interrupt callback routines
 * it been called from noRTOS scheduler
 * */
static void noRTOS_run_always(void) {
	// check for ANY bit is set
	if (noRTOS_interrupt_flag != 0) {
		// check for WHICH bit is set

		if (noRTOS_interrupt_flag & eBIT_MASK_DI_INTERRUPT) {
			// do something
			noRTOS_DIGITAL_INPUT_IRQ();
			clear_bit_in_byte(eBIT_MASK_DI_INTERRUPT, &noRTOS_interrupt_flag);
		}

		if (noRTOS_interrupt_flag & eBIT_MASK_UART_INTERRUPT) {
			// do something
			noRTOS_UART_RX_IRQ();
			clear_bit_in_byte(eBIT_MASK_UART_INTERRUPT, &noRTOS_interrupt_flag);
		}

		if (noRTOS_interrupt_flag & eBIT_MASK_CAN_INTERRUPT) {
			// do something
			noRTOS_CAN_RX_IRQ();
			clear_bit_in_byte(eBIT_MASK_CAN_INTERRUPT, &noRTOS_interrupt_flag);
		}

		if (noRTOS_interrupt_flag & eBIT_MASK_ADC_INTERRUPT) {
			// do something
			noRTOS_ADC_IRQ();
			clear_bit_in_byte(eBIT_MASK_ADC_INTERRUPT, &noRTOS_interrupt_flag);
		}

		if (noRTOS_interrupt_flag & eBIT_MASK_USB_INTERRUPT) {
			// do something
			noRTOS_USB_IRQ();
			clear_bit_in_byte(eBIT_MASK_USB_INTERRUPT, &noRTOS_interrupt_flag);
		}

		if (noRTOS_interrupt_flag & eBIT_MASK_RS485_INTERRUPT) {
			// do something
			noRTOS_RS485_RX_IRQ();
			clear_bit_in_byte(eBIT_MASK_RS485_INTERRUPT, &noRTOS_interrupt_flag);
		}
	}
}

void noRTOS_run_scheduler(void) {

	noRTOS_print_version();
	noRTOS_print_active_task_running();
	noRTOS_setup();

	while(1){
		uint32_t now = NORTOS_SCHEDULAR_GET_TICK();
		/* task that executes always */
		noRTOS_run_always();

		for(uint_fast32_t i = 0; i < number_of_active_task; i++){
			/* task that executes always also in between regular timed tasks */
			noRTOS_run_always();

			/* task that execute with specific timings */
			if( (now - global_list_of_tasks[i]->tick) >= global_list_of_tasks[i]->delay){
				global_list_of_tasks[i]->tick = now;
				global_list_of_tasks[i]->task_callback();
			}
		}
	}
}
