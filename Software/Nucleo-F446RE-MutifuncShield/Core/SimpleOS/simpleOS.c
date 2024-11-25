#include "simpleOS.h"
#include "fifo.h"

static uint32_t os_counter = 0;

/* Periodic Task Table */
//func_ptr periodicTaskTable[periodic_task_number];

/* Nonperiodic Task Table */
static nonperiodic_task_s nonPeriodicTaskTable[non_periodic_task_number];

/* Periodic Task Table */
static periodic_task_s periodicTaskTable[periodic_task_number];

/* fifo buffer handler */
static circular_buf_t fifo;


// Obviously the Task_n has a time slot: 2**n * TimeTick
void IncrementCounterAndStartPeriodicTasks(void)
{
    os_counter++;
    for(int mask=0; mask<periodic_task_number; mask++)
    {
        if( 0 != (os_counter & (1u<<mask)) )
        {
            //printf("Hit Periodic Task at %d\n", (1u<<mask));
        	periodicTaskTable[mask].task_ptr();
            break;
        }
    }
    //printf("Counter: %d -> Inc+1\n\n", os_counter);

}


void StartNonPeriodicTasks(void)
{
    static uint8_t pos_in_queue = 0;

    /* work through nonPeriodicTaskTable[] an find some task that should be exceuted */
    for(uint8_t i=pos_in_queue; i<non_periodic_task_number; i++)
    {
        /* The nonperiodic task is in the queue */
        if(TRUE == nonPeriodicTaskTable[i].flag )
        {
            //printf("Hit NON-Periodic Task at %d\n", i);
            /* Start the nonperiodic task */
            nonPeriodicTaskTable[i].task_ptr();

            /* Clear the queue */
            nonPeriodicTaskTable[i].flag = FALSE;
            nonPeriodicTaskTable[i].task_ptr = NULL;

            /* remember where the hit was situated, make sure running through the hole thing befor starting from top */
            pos_in_queue = i;
            /* exit the search for non periodic task in the queue for now */
            return;
        }
    }
    pos_in_queue = 0;
}



void hello(void)
{
    //printf("--> Hello...\n");
}

void world(void)
{
    //printf("--> World!\n");
}

void StartNonPeriodicTasks_fifo(void)
{
    //printf("Hit NON-Periodic Task, call fifo getter() \n");

    /* 1st attempt returning functionpointer */
    //func_ptr foo = NULL;
    //foo = circular_buf_get(&fifo);

    /* 2nd attempt: returning hole fifo buffer element and run it */
    nonperiodic_task_s temp = {0, NULL};
    uint8_t r = circular_buf_get(&fifo, &temp);

    /* execute only if no error while getting element from fifo */
    if(0==r)
    {
        temp.task_ptr();
    }
}


void simpleOS_init(void)
{
    /* direct way */
    //nonPeriodicTaskTable[0].task_ptr = hello;
    //nonPeriodicTaskTable[0].flag = TRUE;

    //nonPeriodicTaskTable[3].task_ptr = hello;
    //nonPeriodicTaskTable[3].flag = TRUE;

    /* indirect way with pointer exemple */
    //nonperiodic_task_s *p = NULL;
    //p = &nonPeriodicTaskTable[4];

    //(p+1)->flag = 1;
    //(p+1)->task_ptr = hello;

    /* das funktioniert, wieso geht es nicht bei funktionsaufruf 'circular_buf_get(&fifo, foo)' */
//    func_ptr function_to_execute;
//    function_to_execute = hello;
//    function_to_execute();
//
//    /* fifo circular buffer way */
//    circular_buf_init(&fifo, nonPeriodicTaskTable, non_periodic_task_number);

    /* 1st attempt returning functionpointer */
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//    circular_buf_put(&fifo, hello);
//
//    /* testing overrun of circular buffer */
//    circular_buf_put(&fifo, world);
//    circular_buf_put(&fifo, world);

    /* 2nd attempt: returning hole fifo buffer element and run it */

//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, hello});
//
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, world});
//    circular_buf_put(&fifo, (nonperiodic_task_s){1, world});


	/* 01.02.2023 Smart Cube */

	// fill up the periodic task table with default task to prevent the OS from crash
	for (uint_fast8_t i = 0; i < periodic_task_number; i++)
	{
		PERIODIC_TASK_ACTIVATION(simpleOS_callback_dummy, i);
	}

	// fill up the task that are really needed
	PERIODIC_TASK_ACTIVATION(simpleOS_callback_01, 	pTask1_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<1)*2, 	pTask1_nmb);

	PERIODIC_TASK_ACTIVATION(simpleOS_callback_05, 	pTask5_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<5)*2, 	pTask5_nmb); 		// Time Tick is 2ms see in main.c #define DELAY_TIME_MS 2

	PERIODIC_TASK_ACTIVATION(simpleOS_callback_06, 	pTask6_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<6)*2, 	pTask6_nmb);

	PERIODIC_TASK_ACTIVATION(simpleOS_callback_07, 	pTask7_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<7)*2, 	pTask7_nmb);

	PERIODIC_TASK_ACTIVATION(simpleOS_callback_08, 	pTask8_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<8)*2, 	pTask8_nmb);

	PERIODIC_TASK_ACTIVATION(simpleOS_callback_09, 	pTask9_nmb);
	periodic_task_set_cyc_time_us(1000*(1<<9)*2, 	pTask9_nmb);
}

uint32_t periodic_task_set_cyc_time_us(uint32_t time_us, uint8_t task_nmb)
{
	if(task_nmb < periodic_task_number)
	{
		periodicTaskTable[task_nmb].cycle_time_us = time_us;
		return 1;
	}else
		return 0;
}

uint32_t periodic_task_set_exe_time_us(uint32_t time_us, uint8_t task_nmb)
{
	if(task_nmb < periodic_task_number)
	{
		periodicTaskTable[task_nmb].execution_time_us = time_us;
		return 1;
	}else
		return 0;
}

uint32_t periodic_task_get_cyc_time_us(uint8_t task_nmb)
{
	return (periodicTaskTable[task_nmb].cycle_time_us);
}

uint32_t periodic_task_get_exe_time_us(uint8_t task_nmb)
{
	return (periodicTaskTable[task_nmb].execution_time_us);
}


void simpleOS_measure_exe_time_tic(periodic_task_number_e task)
{
	// reset hardware timer timer (with 1MHz time base) 6 to 0
	//__HAL_TIM_SET_COUNTER(&htim6, 0);
	SIMPLE_OS_RESET_HW_TIMER();

	// start timer 6 for executing measurement
	//HAL_TIM_Base_Start(&htim6);
	SIMPLE_OS_START_HW_TIMER();

}
void simpleOS_measure_exe_time_toc(periodic_task_number_e task)
{
	// stop timer 6 for executing measurement
	//HAL_TIM_Base_Stop(&htim6);
	SIMPLE_OS_STOP_HW_TIMER();

	//periodic_task_set_exe_time_us(__HAL_TIM_GET_COUNTER(&htim6), pTask5_nmb);
	periodic_task_set_exe_time_us(SIMPLE_OS_GET_HW_TIMER(), task);
}

void simpleOS_demo(void)
{

//    char str[100];
//    printf( "Is Next Time Tick Over ?");
//    gets( str );
//    int32_t c = atoi(str);
//
//    printf( "You entered: %d \n\n", c);
//    fflush(stdin);
//
//    if(c == 1)
//    {
//        IncrementCounterAndStartPeriodicTasks();
//    }
//    else
//    {
//        //StartNonPeriodicTasks();
//        StartNonPeriodicTasks_fifo();
//    }

	/* 01.02.2023 Smart Cube */
	IncrementCounterAndStartPeriodicTasks();


}

__weak void simpleOS_callback_01(void)
{
	__NOP();
}
__weak void simpleOS_callback_02(void)
{
	__NOP();
}
__weak void simpleOS_callback_03(void)
{
	__NOP();
}
__weak void simpleOS_callback_04(void)
{
	__NOP();
}

__weak void simpleOS_callback_05(void)
{
	__NOP();
}

__weak void simpleOS_callback_06(void)
{
	__NOP();
}

__weak void simpleOS_callback_07(void)
{
	__NOP();
}

__weak void simpleOS_callback_08(void)
{
	__NOP();
}

__weak void simpleOS_callback_09(void)
{
	__NOP();
}

__weak void simpleOS_callback_10(void)
{
	__NOP();
}

__weak void simpleOS_callback_11(void)
{
	__NOP();
}

__weak void simpleOS_callback_16(void)
{
	__NOP();
}


__weak void simpleOS_callback_dummy(void)
{
	__NOP();
}
