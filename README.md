# Get back to work with this branch on April 3rd 2025

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
# Examples

- Basic Example
- Multifunction Shield
- ESP32 as Wifi Coprocessor with AT-Command-Firmware
- Multi Sensor Reading and Processing 

There are two examples. One very simple which uses a NucleoF446RE Board. It will work with every other Nucleo-Board, since the app uses just the blue onboard button, the LED and the UART2 which is rooted to the onboard STlink USB debugger.   
Second example uses the [multifunction shield](https://www.amazon.de/Hailege-Multifunktionale-Erweiterungskarte-Infrarot-Empf%C3%A4nger-Erweiterungsfunktion/dp/B07Y82V2SK/ref=asc_df_B07Y82V2SK?mcid=ca525e76b08b3d098cc3bf185179ebcf&th=1&psc=1&tag=googshopde-21&linkCode=df0&hvadid=696321262544&hvpos=&hvnetw=g&hvrand=6870903724215630989&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9042490&hvtargid=pla-1650224597464&psc=1&gad_source=1) to demonstrate "parallel" processing of multiple task.   
