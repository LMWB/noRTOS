#ifndef DRIVERS_AHT21_AHT21_H_
#define DRIVERS_AHT21_AHT21_H_

#include <stdint.h>
#include "platformGlue.h"

#define AHT21_I2C_ADDR		(0x38 << 1)
#define AHT21_CMD_INIT    	0xBE
#define AHT21_CMD_TRIGGER 	0xAC

DEVICE_STATUS_DEFINITION AHT21_Init( void );

DEVICE_STATUS_DEFINITION AHT21_start_convertion( void );

DEVICE_STATUS_DEFINITION AHT21_ReadData( int16_t *temp_x10, int16_t *hum_x10);


#endif /* DRIVERS_AHT21_AHT21_H_ */
