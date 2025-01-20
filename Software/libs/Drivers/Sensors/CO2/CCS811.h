#ifndef DRIVERS_SENSORS_CO2_CCS811_H_
#define DRIVERS_SENSORS_CO2_CCS811_H_

#include "stdint.h"

typedef struct _ccs811_data{
	uint32_t eCO2;
	uint32_t voc;
	uint32_t temperature;
	uint32_t humidity;
	uint8_t errorID;
	uint8_t status;
}ccs811_data_t;

void ccs811_init(void);
void ccs811_soft_reset(void);
void ccs811_app_start(void);

uint8_t ccs811_read_status_register(uint8_t* satus_byte);



uint8_t ccs811_read_mode_register(uint8_t* mode_byte);
uint8_t ccs811_write_mode_register(uint8_t mode_byte);

uint8_t ccs811_read_HW_ID_register(uint8_t* hw_byte);
uint8_t ccs811_read_HW_version_register(uint8_t* version_byte);
uint8_t ccs811_read_error_ID_register(uint8_t* error_byte);

uint8_t ccs811_write_mode_register(uint8_t mode_byte);

uint8_t ccs811_read_result_registers(uint8_t* result_bytes, uint8_t size);

uint8_t ccs811_read_environment_data_registers(uint8_t* env_bytes, uint8_t size);
uint8_t ccs811_write_environment_data_registers(uint8_t* env_bytes, uint8_t size);

void ccs811_sample_data(ccs811_data_t* sensor);

#endif /* DRIVERS_SENSORS_CO2_CCS811_H_ */
