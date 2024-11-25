#ifndef SIMPLEOS_H_INCLUDED
#define SIMPLEOS_H_INCLUDED

#include <main.h>
#include <tim.h>

#define SIMPLE_OS_START_HW_TIMER()	HAL_TIM_Base_Start(&htim6) 			// start timer 6 for executing measurement
#define SIMPLE_OS_STOP_HW_TIMER()	HAL_TIM_Base_Stop(&htim6)
#define SIMPLE_OS_RESET_HW_TIMER()	__HAL_TIM_SET_COUNTER(&htim6, 0)	// reset hardware timer timer (with 1MHz time base) to 0
#define SIMPLE_OS_GET_HW_TIMER()	__HAL_TIM_GET_COUNTER(&htim6) 	// get hardware timer timer (with 1MHz time base) counter value

//#define NULL     ((void*)0)

#define TRUE   (0==0)
#define FALSE  (0==1)

typedef enum __periodic_task_number
{
	pTask1_nmb = 0,
	pTask2_nmb,
	pTask3_nmb,
	pTask4_nmb,
	pTask5_nmb,
	pTask6_nmb,
	pTask7_nmb,
	pTask8_nmb,
	pTask9_nmb,
	pTask10_nmb,
	pTask11_nmb,
	pTask12_nmb,
	pTask13_nmb,
	pTask14_nmb,
	pTask15_nmb,
	pTask16_nmb,
	periodic_task_number
} periodic_task_number_e;

typedef enum __non_periodic_task_number
{
	npTask1_nmb = 0,
	npTask2_nmb,
	npTask3_nmb,
	npTask4_nmb,
	npTask5_nmb,
	npTask6_nmb,
	npTask7_nmb,
	npTask8_nmb,
	non_periodic_task_number
} non_periodic_task_number_e;

/* Prototype for periodic task and non periodic task function pointer */
typedef void(*func_ptr)(void);

typedef struct periodic_task_struct
{
	uint8_t flag;
	func_ptr task_ptr;
	uint32_t cycle_time_us;
	uint32_t execution_time_us;
} periodic_task_s;

typedef struct nonperiodic_task_struct
{
	uint8_t flag;
	func_ptr task_ptr;
} nonperiodic_task_s;


/* Task Activation/Deactivation */
#define PERIODIC_TASK_ACTIVATION(	 fnc_ptr, nmb)  do{periodicTaskTable[nmb].task_ptr 	  = fnc_ptr; periodicTaskTable[nmb].flag 	= TRUE;}while(0)
#define NON_PERIODIC_TASK_ACTIVATION(fnc_ptr, nmb)  do{nonPeriodicTaskTable[nmb].task_ptr = fnc_ptr; nonPeriodicTaskTable[nmb].flag = TRUE;}while(0)

uint32_t periodic_task_set_cyc_time_us(uint32_t time_us, uint8_t task_nmb);
uint32_t periodic_task_get_cyc_time_us(uint8_t task_nmb);

uint32_t periodic_task_set_exe_time_us(uint32_t time_us, uint8_t task_nmb);
uint32_t periodic_task_get_exe_time_us(uint8_t task_nmb);

void simpleOS_demo(void);
void simpleOS_init(void);

void simpleOS_measure_exe_time_tic(periodic_task_number_e task);
void simpleOS_measure_exe_time_toc(periodic_task_number_e task);

void simpleOS_callback_01(void);
void simpleOS_callback_02(void);
void simpleOS_callback_03(void);
void simpleOS_callback_04(void);
void simpleOS_callback_05(void);
void simpleOS_callback_06(void);
void simpleOS_callback_07(void);
void simpleOS_callback_08(void);
void simpleOS_callback_09(void);
void simpleOS_callback_10(void);
void simpleOS_callback_11(void);
void simpleOS_callback_12(void);
void simpleOS_callback_13(void);
void simpleOS_callback_14(void);
void simpleOS_callback_15(void);
void simpleOS_callback_16(void);
void simpleOS_callback_dummy(void);



#endif // SIMPLEOS_H_INCLUDED
