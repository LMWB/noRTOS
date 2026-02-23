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

void pfc8574_state_machine(void) {
	static uint8_t z = 0;
	switch (z) {
	case 0:
		// 0xF0 -> 1111 0000 (Pins 4-7 auf High für Input, Pins 0-3 auf Low für LEDs aus)
		if ( PCF8574_Init( 0xF0 ) == DEVICE_OK) {
			z = 1;
		}
		break;
	case 1:
		PCF8574_Write( 0x0F );
		z = 2;
		break;
	case 2:
		PCF8574_Write( 0xF0 );
		z = 1;
		break;
	default:
		z = 0;
		break;
	}
}
