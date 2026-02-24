#ifndef DRIVERS_EEPROM_VIRTUALEEPROM_H_
#define DRIVERS_EEPROM_VIRTUALEEPROM_H_
#include <stdint.h>
/*
 * // STM32F446RE Linker Script
 * MEMORY
 * {
 * // Sector 0 & 1 (32KB total)
 * FLASH_LOW (rx)  : ORIGIN = 0x08000000, LENGTH = 32k
 * // WE SKIP SECTOR 2 & 3 (The 32KB used by your EEPROM config)
 * // Sector 4 to 7 (Remaining Flash)
 * FLASH_HIGH (rx) : ORIGIN = 0x08010000, LENGTH = 448K
 * RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
 * }
 *
 * // Define which region to use for code
 * REGION_ALIAS("REGION_TEXT", FLASH_LOW);
 * // Note: If your code exceeds 32KB, you will get a 'Region Overflow' error.
 * */


/*
 * STM32 F446
 * Sector,	Size,	Your Use Case
 * 0,		16 KB,	Interrupt Vector Table & Startup Code
 * 1,		16 KB,	Your Application Code
 * 2,		16 KB,	More Application Code
 * 3,		16 KB,	More Application Code
 * 4,		64KB,	More Application Code
 * 5,		128 KB,	Reserved
 * 6,		128 KB,	EEPROM PAGE 0
 * 7,		128 KB,	EEPROM PAGE 1
 *
 */


/*
 * Example
 * Starting at Sector 6 (0x08040000) */
#define VIRTUAL_EEPROM_NUMBER_OF_VARIABLES_STORED	((uint8_t) 8)

#define VIRTUAL_EEPROM_START_ADDRESS				((uint32_t)0x08040000)

#define VIRTUAL_EEPROM_PAGE_SIZE					(uint32_t)0x20000  /* 128 KByte */
#define VIRTUAL_EEPROM_PAGE0_ID						FLASH_SECTOR_6
#define VIRTUAL_EEPROM_PAGE1_ID						FLASH_SECTOR_7

/* Virtual address defined by the user: 0xFFFF value is prohibited */
extern uint16_t VirtAddVarTab[VIRTUAL_EEPROM_NUMBER_OF_VARIABLES_STORED];
extern uint16_t VarDataTab[VIRTUAL_EEPROM_NUMBER_OF_VARIABLES_STORED];

#endif /* DRIVERS_EEPROM_VIRTUALEEPROM_H_ */
