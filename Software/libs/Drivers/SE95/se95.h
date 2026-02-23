#ifndef DRIVERS_SE95_SE95_H_
#define DRIVERS_SE95_SE95_H_

#include <stdint.h>
#include "platformGlue.h"

/*
 * The SE95 from NXP has status END OF LIVE
 * New generation is PCT2075 and the good old classic LM75
 * Mostly Register compatible
 */

#define SE95_I2C_ADDR	(0x4F << 1) // defined by hardware solder bridges
#define SE95_REG_TEMP	0x00
#define SE95_ERROR_CODE	-9999

DEVICE_STATUS_DEFINITION	SE95_Init( void );
int16_t 					SE95_read_temperature( void );

int16_t se95_state_machine(void);

#endif /* DRIVERS_SE95_SE95_H_ */
