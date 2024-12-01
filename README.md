# noRTOS
rudimentary C-library for microcontrollers to organize and run various task with sufficient performance without RTOS

# Preface
I'm sure there are many thousand different very good approaches out there on how to organize and manage multiple things do on a single cpu low budget microcontroller **without a RTOS**, but still I would like to share mine.

# How To Use
Copy paste the ```noRTOS.c, noRTOS.h, utils.c, utils.h``` files to your project tree.
Copy paste ```hardwareGlobal.h``` to you project tree and modify line ```19 #define GET_TICK()``` according to your platform.

In ```main.c``` right before the ```int main(void)```  do the following. Create some callback function you like to run. Consider each as individual **non blocking** main()-loops.   

```
void blinky(void) {
    LED_toggle();
}

void test_callback1(void) {
    printf("Testing printf with _write() override\n\n");
}

```
Than right before ```while(1)``` super-loop create instances of ```noRTOS_task_t``` and add them to the scheduler.   

```
noRTOS_task_t blinky_t = { .delay = eNORTOS_PERIODE_1s, .task_callback = blinky };
noRTOS_add_task_to_scheduler(&blinky_t);

noRTOS_task_t test_task1 = { .delay = eNORTOS_PERIODE_5s, .task_callback = test_callback1 };
noRTOS_add_task_to_scheduler(&test_task1);

```
Finally start the scheduler   

```
/* this runs for ever */
noRTOS_run_scheduler();
/* never get here! */
while (1) {}

```