#ifndef DRIVERS_SENSORS_CO2_SCD4X_H_
#define DRIVERS_SENSORS_CO2_SCD4X_H_

#include "stdint.h"
#include "stdbool.h"
#define SCD4x_I2C_ADDRESS	0x62

#define SCD4x_CMD_START_PERIODIC_MEASUREMENT	0x21b1
#define SCD4x_CMD_REINIT 						0x3646
#define SCD4x_CMD_GET_SERIAL_NUMBER 			0x3682
#define SCD4x_CMD_WAKE_UP 						0x36f6
#define SCD4x_CMD_STOP_PERIODIC_MEASUREMENT 	0x3f86
#define SCD4x_CMD_GET_DATA_READY_STATUS 		0xe4b8
#define SCD4x_CMD_READ_MEASUREMENT 				0xec05


void scd4x_sleep(uint32_t milli_seconds);

void scd4x_wake_up(void);
void scd4x_stop_periodic_measurement(void);
void scd4x_reinit(void);
void scd4x_get_serial_number(uint16_t *serial_0, uint16_t *serial_1, uint16_t *serial_2);
void scd4x_start_periodic_measurement(void);
void scd4x_get_data_ready_flag(bool *data_ready_flag);
void scd4x_read_measurement(uint16_t *co2, int32_t *temperature_m_deg_c, int32_t *humidity_m_percent_rh);


#endif /* DRIVERS_SENSORS_CO2_SCD4X_H_ */
