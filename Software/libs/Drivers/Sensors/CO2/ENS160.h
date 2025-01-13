#ifndef DRIVERS_SENSORS_CO2_ENS160_H_
#define DRIVERS_SENSORS_CO2_ENS160_H_

#include <stdint.h>

#define ENS160_I2C_ADDRESS	0x52
#define ENS160_OPMODE 		0x10	// Operating mode:0x00: DEEP SLEEP mode (low power standby); 0x01: IDLE mode (low-power); 0x02: STANDARD Gas Sensing Modes

#define ENS160_TEMP_IN 		0x13	// 2 Bytes Length
#define ENS160_RH_IN 		0x15	// 2 Bytes Length
#define ENS160_DATA_STATUS	0x20
#define ENS160_DATA_ECO2	0x24 	// 2 Bytes Length
#define ENS160_DATA_T 		0x30	// 2 Bytes Length
#define ENS160_DATA_RH 		0x32	// 2 Bytes Length


void ens160_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length);
void ens160_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length);

void ens160_init(void);
void ens160_translate_status_byte(uint8_t status_byte);







#endif /* DRIVERS_SENSORS_CO2_ENS160_H_ */
