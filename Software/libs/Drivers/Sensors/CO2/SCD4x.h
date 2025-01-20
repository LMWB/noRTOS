#ifndef DRIVERS_SENSORS_CO2_SCD4X_H_
#define DRIVERS_SENSORS_CO2_SCD4X_H_

#include "stdint.h"
#include "stdbool.h"

typedef struct _scd4x_data{
	uint32_t eCO2;
	int32_t temperature;
	int32_t humidity;
}scd4x_data_t;

void scd4x_sleep(uint32_t milli_seconds);

void sdc4x_init(void);
void scd4x_wake_up(void);
void scd4x_stop_periodic_measurement(void);
void scd4x_reinit(void);
void scd4x_get_serial_number(uint16_t *serial_0, uint16_t *serial_1, uint16_t *serial_2);
void scd4x_start_periodic_measurement(void);
void scd4x_get_data_ready_flag(bool *data_ready_flag);
void scd4x_read_measurement(uint16_t *co2, int32_t *temperature_m_deg_c, int32_t *humidity_m_percent_rh);
void scd4x_sample_data(scd4x_data_t* sensor);

#endif /* DRIVERS_SENSORS_CO2_SCD4X_H_ */
