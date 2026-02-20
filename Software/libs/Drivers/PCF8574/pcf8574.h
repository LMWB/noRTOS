#ifndef DRIVERS_PCF8574_PCF8574_H_
#define DRIVERS_PCF8574_PCF8574_H_

#include "platformGlue.h"

// Basisadresse 0x20, mit A0-A2 auf GND -> 0x40 (linksbündig)
#define PCF8574_ADDR (0x20 << 1)

/**
 * Initialisiert den Expander.
 * Pins, die als Eingang dienen sollen, MÜSSEN im init_mask auf 1 gesetzt sein.
 */
DEVICE_STATUS_DEFINITION PCF8574_Init( uint8_t init_mask);

/**
 * Schreibt alle 8 Pins gleichzeitig.
 */
DEVICE_STATUS_DEFINITION PCF8574_Write( uint8_t val);

/**
 * Liest den aktuellen Status aller 8 Pins.
 */
DEVICE_STATUS_DEFINITION PCF8574_Read( uint8_t *val);


#endif /* DRIVERS_PCF8574_PCF8574_H_ */
