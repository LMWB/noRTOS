#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H


#include <stdlib.h>
#include <string.h>
#include <stdarg.h> //for va_list var arg function
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "platformGlue.h"

#ifdef PLATFORM_HAS_UART
/* *************** Alternative to standard printf() which not need the "\n" terminator ******** */
void myprintf(const char *fmt, ...);

#endif

/**
 *
 */
uint32_t raw_buffer_to_hex_string(const uint8_t *buffer, size_t buffer_size, char *hex_string);

/**
 *
 */
void scan_i2c_sensors(void);


/**
 * Uses the value pointed by STM32 RTC timer to fill a tm structure with the values
 * that represent the corresponding time, expressed as a UTC time
 * (i.e., the time at the GMT timezone).
 */
struct tm *get_gmtime_stm32();

/**
 * @brief 	set the Microcontrollers Real Time Clock
 * 			implementation is vendor specific
 * 			here we use STM32
 */
uint8_t set_gmtime_stm32(struct tm *time);

/**
 * @brief  Convert the STM32 HAL RTC timestamp structure into unix epoch time in seconds from Jan 1st 1900 00:00:00
 * @param  non
 * @retval SECONDS since epoch time 1970 Jan 1st 00:00:00
 */
time_t get_epoch_time(void);

/**
 * @brief  Convert the unix epoch time string e.g. "Tue Oct 19 17:47:56 2021"
 * 		to unix epoch time in seconds from Jan 1st 1900 00:00:00
 * 		e.g. python time.asctime()
 * @param  pointer to char array (string) asctime() String e.g. "Tue Oct 19 17:47:56 2021".
 * @param  pointer to struct tm from time.h for application use on exact time/date Format
 * @retval time_t Unix Epoch Time
 */
time_t cvt_asctime(const char *linux_asctime_str, struct tm *time);

/*
 * CPU Tick Measurement
 */
void DWT_Init(void);

#endif
