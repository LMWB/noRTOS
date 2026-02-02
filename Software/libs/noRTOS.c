#include "noRTOS.h"
#include "platformGlue.h"
#include <string.h>
/**
 * *************** Task Schedular ***************
 */

noRTOS_task_t *global_list_of_tasks[NORTOS_NO_OF_MAX_TASK];
static uint32_t number_of_active_task = 0;

static uint8_t noRTOS_interrupt_flag = 0;

#define BIT_MASK_RESET_ALL		0b00000000 /* 0 */
#define BIT_MASK_DI_INTERRUPT	0b00000001 /* 1 */
#define BIT_MASK_UART_INTERRUPT	0b00000010 /* 2 */
#define BIT_MASK_CAN_INTERRUPT	0b00000100 /* 3 */
#define BIT_MASK_RESERVED1		0b00001000 /* 4 */
#define BIT_MASK_RESERVED2		0b00010000 /* 5 */
#define BIT_MASK_RESERVED3		0b00100000 /* 6 */
#define BIT_MASK_RESERVED4		0b01000000 /* 7 */
#define BIT_MASK_RESERVED5		0b10000000 /* 8 */

static void set_bit_in_byte(const uint8_t bit, uint8_t* byte){
	*byte |= bit;
}

static void clear_bit_in_byte(const uint8_t bit, uint8_t* byte){
	*byte &= ~bit;
}

static void clear_all_bits_in_byte(uint8_t* byte){
	*byte = 0;
}

/* blue print  for more callbacks of this kind */
bool noRTOS_wait_for_eventX(uint8_t event){
	if( noRTOS_interrupt_flag & event){
		clear_bit_in_byte(event, &noRTOS_interrupt_flag);
		return true;
	}else{
		return false;
	}
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

/* override this with your implementation */
__weak void noRTOS_setup(void){
	;
}

/* override this with your implementation */
__weak void noRTOS_run_always(void){
	;
}

void noRTOS_run_scheduler(void) {

	noRTOS_print_version();
	noRTOS_print_active_task_running();
	noRTOS_setup();

	while(1){
		uint32_t now = NORTOS_SCHEDULAR_GET_TICK();

		for(uint_fast32_t i = 0; i < number_of_active_task; i++){
			/* task that executes always */
			noRTOS_run_always();

			/* task that execute with specific timings */
			if( (now - global_list_of_tasks[i]->tick) >= global_list_of_tasks[i]->delay){
				global_list_of_tasks[i]->tick = now;
				global_list_of_tasks[i]->task_callback();
			}
		}
	}
}

/**
 * *************** temporary UART 2 Handler ***************
 */

uint8_t uart2_buffer[UART_BUFFER_SIZE] =  {0};
static uint16_t rx_size = 0;
static bool uart2_read_line_complete = false;

bool noRTOS_is_UART2_read_line_complete(void){
	return uart2_read_line_complete;
}

void noRTOS_UART2_read_byte_with_interrupt(void){
	HAL_UART_Receive_IT(&huart2, &uart2_buffer[rx_size], 1);
}

void noRTOS_UART2_clear_rx_buffer(void){
	rx_size = 0;
	uart2_read_line_complete = false;
	memset(uart2_buffer, 0, UART_BUFFER_SIZE);
	noRTOS_UART2_read_byte_with_interrupt();
}

void noRTOS_UART2_echo_whats_been_received(void)
{
	UART_TERMINAL_SEND( uart2_buffer, rx_size);
}

void noRTOS_UART2_receive_byte_callback(void){
	/* check if read line complete */
	if(rx_size >= 3 && uart2_buffer[rx_size-1] == '\r' && uart2_buffer[rx_size] == '\n'){
		rx_size++;
		uart2_read_line_complete = true;
	}else{
		rx_size++;
		noRTOS_UART2_read_byte_with_interrupt();
	}
}
