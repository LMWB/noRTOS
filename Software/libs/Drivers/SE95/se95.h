#ifndef DRIVERS_SE95_SE95_H_
#define DRIVERS_SE95_SE95_H_

#include <stdint.h>
#include "platformGlue.h"

#define SE95_I2C_ADDR	(0x4F << 1)
#define SE95_REG_TEMP	0x00
#define SE95_ERROR_CODE	-9999

DEVICE_STATUS_DEFINITION SE95_Init( void );
int16_t SE95_ReadTemperature( void );


#endif /* DRIVERS_SE95_SE95_H_ */
