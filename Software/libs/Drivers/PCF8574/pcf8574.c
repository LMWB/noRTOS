#include "pcf8574.h"

// Wir speichern den letzten Zustand, um einzelne Bits ändern zu können
static uint8_t current_output = 0xFF;

DEVICE_STATUS_DEFINITION PCF8574_Init( uint8_t init_mask) {
    current_output = init_mask;
    return I2C_TRANSMIT(PCF8574_ADDR, &current_output, 1);
}

DEVICE_STATUS_DEFINITION PCF8574_Write( uint8_t val) {
    current_output = val;
    return I2C_TRANSMIT(PCF8574_ADDR, &current_output, 1);
}

DEVICE_STATUS_DEFINITION PCF8574_Read( uint8_t *val) {
    // Einfaches Lesen vom Bus liefert den Status der Pins
    return I2C_RECEIVE( PCF8574_ADDR, val, 1);
}
