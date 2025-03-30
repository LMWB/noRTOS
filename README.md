# noRTOS
rudimentary C-library for microcontrollers to organize and run various task with sufficient performance without RTOS

# Preface
I'm sure there are many thousand different very good approaches out there on how to organize and manage multiple "things to do" on a single cpu low budget microcontroller **without a RTOS**, but still I would like to share mine.

# How To Use
Copy paste the ```noRTOS.c, noRTOS.h, utils.c, utils.h``` files to your project tree.
Copy paste ```hardwareGlobal.h``` to you project tree and modify line ```19 #define GET_TICK()``` according to your platform.

In ```main.c``` right before the ```int main(void)```  do the following.
Create some callback functions you like to run.
Consider each as individual **non blocking** main()-loops.   

```
void blinky(void) {
    LED_toggle();
}

void test_callback1(void) {
    printf("Testing printf with _write() override\n\n");
}

```
After that and right before the ```while(1)``` super-loop create corresponding amount of instances of type ```noRTOS_task_t``` and add them to the scheduler.   

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
# Pros and Cons of noRTOS compared to "proper RTOS"
With proper RTOS I mean the very popular freeRTOS, Azure-RTOS. But there are many more like the CMSIS-RTOS, mBedded-RTOS, Zephyr-RTOS, RIOT to name a few.  
My **personal** thoughts on when to use a RTOS vs when to use noRTOS  

## Pros
- no need to consider what RTOS should I use
- no licenses no costs
- easy to debug without expensive tools or software
- no need to worry about crazy terms like: mutexes, semaphores, task priorities, preemption, etc
- mainly HAL driven
- c-code only!
    - no crazy JSON or YAML file where nobody knows there syntax or what they are used for

## Cons
- no support (not yet ;) for file-system, network-module, usb-stack
- mainly HAL driven

# Examples
- [Basic Example](Projects/Basic-Nucleo/README.md)
- [Multifunction Shield](Projects/Multi-Function-Shield/README.md)
- [ESP32 as Wifi Coprocessor with AT-Command-Firmware](Projects/AT-Commands/README.md)
- [Multi Sensor Reading and Processing](Projects/Multi-Sensors/README.md)