#ifndef DRIVERS_BME280_BME280_H_
#define DRIVERS_BME280_BME280_H_

#include <stdint.h>
#include "platformGlue.h"

#define BME280_ADDR (0x76 << 1)

// --- CONFIGURATION ---
#define USE_PRESSURE  1  // Setze auf 0, um Druck-Berechnung und Trimm-Daten zu entfernen
// ---------------------

DEVICE_STATUS_DEFINITION BME280_Init( void );

// Rückgabe: temp in 0.01°C, hum in 0.1% RH, press in Pa (Pascal)
void BME280_read( int16_t *temp, int16_t *hum, uint32_t *press);

int16_t BME280_get_altitude(uint32_t press_pa, uint32_t sea_level_pa);


#endif /* DRIVERS_BME280_BME280_H_ */
