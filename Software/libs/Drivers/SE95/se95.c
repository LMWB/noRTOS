#include "se95.h"


DEVICE_STATUS_DEFINITION SE95_Init( void ) {
    return I2C_IS_DEVICE_READY( SE95_I2C_ADDR );
}

/**
 * Liest die Temperatur in 0,1°C Schritten.
 * Beispiel: 225 entspricht 22,5°C
 */
int16_t SE95_ReadTemperature( ) {
    uint8_t buf[2];

    if (I2C_READ_MEMORY(SE95_I2C_ADDR, SE95_REG_TEMP, buf, 2) != DEVICE_OK) {
        return SE95_ERROR_CODE; // Fehlercode
    }

    // 13-Bit Rohwert extrahieren (Vorzeichen bleibt durch Cast erhalten)
    // MSB ist buf[0], LSB ist buf[1]. Die untersten 3 Bits von buf[1] sind undefiniert.
    int16_t raw = (int16_t)((buf[0] << 8) | buf[1]) >> 3;

    // Umrechnung: (raw * 10) / 32  =>  (raw * 5) / 16
    // Das ergibt die Temperatur in Zehntel-Grad Celsius.
    return (raw * 5) / 16;
}
