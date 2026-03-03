#include "virtualEEPROM.h"



uint16_t VirtAddVarTab[VIRTUAL_EEPROM_NUMBER_OF_VARIABLES_STORED] = {
		0x00,
		0x10,
		0x20,
		0x30,
		0x40,
		0x50,
		0x60,
		0x70};

uint16_t VarDataTab[VIRTUAL_EEPROM_NUMBER_OF_VARIABLES_STORED] = {};



typedef union {
    float asFloat;
    uint32_t asInt;
    uint16_t asArray[2]; // Two 16-bit halves
} EE_Data32;

uint16_t EE_WriteFloat(uint16_t virtAddrStart, float value) {
    EE_Data32 converter;
    converter.asFloat = value;

    // Write Low Half (Address X)
    uint16_t status = EE_WriteVariable(virtAddrStart, converter.asArray[0]);
    if (status != HAL_OK) return status;

    // Write High Half (Address X + 1)
    return EE_WriteVariable(virtAddrStart + 1, converter.asArray[1]);
}
