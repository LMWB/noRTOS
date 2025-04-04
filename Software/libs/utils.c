#include "utils.h"


/* override _write, is used by puts and printf */
int _write(int file, char *ptr, int len)
{
    UART_TERMINAL_SEND((uint8_t*) ptr, (uint16_t)len);
	return len;
}


uint32_t raw_buffer_to_hex_string(const uint8_t *buffer, size_t buffer_size, char *hex_string) {
	uint32_t bytes_written = 0;
	for (uint_fast32_t i = 0; i < buffer_size; i++) {
		bytes_written += sprintf(hex_string + 2 * i, "%02X", buffer[i]);
	}
	hex_string[2 * buffer_size] = '\0';

	return bytes_written + 1;
}


void myprintf(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int len = strlen(buffer);
	UART_TERMINAL_SEND((uint8_t *) buffer, len);
}

#ifdef PLATFORM_HAS_I2C
void scan_i2c_sensors(void) {
	myprintf("Scanning I2C bus...\n");

	DEVICE_STATUS_DEFINITION res;
	for (uint16_t i = 0; i < 128; i++) {
		res = IS_I2C_DEVICE_READY(i << 1);
		if( i%32 == 0){
			myprintf("\n");
		}
		if (res == DEVICE_OK) {
			myprintf("0x%02X", i);
		} else {
			myprintf(".");
		}
		DELAY(20);
	}

	myprintf("\r\n");
	DELAY(1000);
}
#endif

#ifdef PLATFORM_HAS_RTC
struct tm *get_gmtime_stm32() {
	/* Reference: https://cplusplus.com/reference/ctime/tm/
	 *
	 * tm_sec	int	seconds after the minute	0-60*
	 * tm_min	int	minutes after the hour		0-59
	 * tm_hour	int	hours since midnight		0-23
	 * tm_mday	int	day of the month			1-31
	 * tm_mon	int	months since January		0-11
	 * tm_year	int	years since 				1900
	 * tm_wday	int	days since Sunday			0-6
	 * tm_yday	int	days since January 1		0-365
	 * tm_isdst	int	Daylight Saving Time flag
	 *
	 * tm_sec is generally 0-59. The extra range is to accommodate for leap seconds in certain systems.
	 * */

	static struct tm ts;

	/* how to check for RTC is working */
	if (true) {
		RTC_TimeTypeDef current_time = {0};
		RTC_DateTypeDef current_date = {0};

		GET_DEVICE_TIME(&current_time);
		GET_DEVICE_DATE(&current_date);

		ts.tm_year = current_date.Year + 100;
		ts.tm_mday = current_date.Date;
		ts.tm_mon = current_date.Month - 1;
		ts.tm_hour = current_time.Hours;
		ts.tm_min = current_time.Minutes;
		ts.tm_sec = current_time.Seconds;
	} else {
		// default timestamp
		ts.tm_year = 123;
		ts.tm_mday = 1;
		ts.tm_mon = 0;
		ts.tm_hour = 0;
		ts.tm_min = 0;
		ts.tm_sec = 0;
	}
	return &ts;
}

time_t get_epoch_time(void) {
	time_t timestamp = 0;
	struct tm *ptm;

	ptm = get_gmtime_stm32();
	timestamp = mktime(ptm);

	return timestamp;
}

static const char MONTH_NAMES[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
static const char WEEK_NAMES[] = "SunMonTueWedThuFriSat";

time_t cvt_asctime(const char *linux_asctime_str, struct tm *time) {
	struct tm t = {0};
	int hour, minutes, seconds;
	int year, month, day;
	char buffer_day[5];
	char buffer_month[5];

	sscanf(linux_asctime_str, "%s %s %2d %2d:%2d:%2d %4d", buffer_day,
		   buffer_month, &day, &hour, &minutes, &seconds, &year);

	// Find where is s_month in month_names. Deduce month value.

	month = (strstr(MONTH_NAMES, buffer_month) - MONTH_NAMES) / 3;

	t.tm_year = year - 1900;
	t.tm_mon = month;
	t.tm_mday = day;

	t.tm_hour = hour;
	t.tm_min = minutes;
	t.tm_sec = seconds;

	// not that important
	t.tm_wday = (strstr(WEEK_NAMES, buffer_day) - WEEK_NAMES) / 3;

	/* return value for param 1 */
	time->tm_year = t.tm_year;
	time->tm_mon = t.tm_mon;
	time->tm_mday = t.tm_mday;
	time->tm_hour = t.tm_hour;
	time->tm_min = t.tm_min;
	time->tm_sec = t.tm_sec;
	time->tm_wday = t.tm_wday;

	/* return */
	return mktime(&t);
}

uint8_t set_gmtime_stm32(struct tm *time) {
	RTC_TimeTypeDef s_time = {0};
	RTC_DateTypeDef s_date = {0};
	s_time.Hours = time->tm_hour;
	s_time.Minutes = time->tm_min;
	s_time.Seconds = time->tm_sec;
	s_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	s_time.StoreOperation = RTC_STOREOPERATION_RESET;
	if (SET_DEVICE_TIME(&s_time) != DEVICE_OK) {
		Error_Handler();
	}
	s_date.WeekDay = time->tm_wday;
	s_date.Month = time->tm_mon + 1;
	s_date.Date = time->tm_mday;
	s_date.Year = time->tm_year - 100;
	if (SET_DEVICE_DATE(&s_date) != DEVICE_OK) {
		Error_Handler();
	}
	return 1;
}
#endif







