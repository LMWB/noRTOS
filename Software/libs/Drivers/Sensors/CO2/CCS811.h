#ifndef DRIVERS_SENSORS_CO2_CCS811_H_
#define DRIVERS_SENSORS_CO2_CCS811_H_


#define STATUS 			0x00 // R 1 byte Status register
#define MEAS_MODE 		0x01 // R/W 1 byte Measurement mode and conditions register
#define ALG_RESULT_DATA 0x02 // R up to 8 bytes Algorithm result. The most significant 2 bytes contain a ppm estimate of the equivalent CO 2 (eCO 2) level, and the next two bytes contain a ppb estimate of the total VOC level.
#define RAW_DATA 		0x03 // R 2 bytes Raw ADC data values for resistance and current source used.
#define ENV_DATA 		0x05 // W 4 bytes Temperature and Humidity data can be written to enable compensation
#define NTC 			0x06 // R 4 bytes Provides the voltage across the reference resistor and the voltage across the NTC resistor – from which the ambient temperature can be determined.
#define THRESHOLDS 		0x10 // W 5 bytes Thresholds for operation when interrupts are only generated when eCO2 ppm crosses a threshold
#define BASELINE 		0x11 // R/W 2 bytes The encoded current baseline value can be read. A previously saved encoded baseline can be written.
#define HW_ID 			0x20 // R 1 byte Hardware ID. The value is 0x81
#define HW_Version 		0x21 // R 1 byte Hardware Version. The value is 0x1X
#define FW_Boot_Version 0x23 // R 2 bytes Firmware Boot Version. The first 2 bytes contain the firmware version number for the boot code.
#define FW_App_Version 	0x24 // R 2 bytes Firmware Application Version. The first 2 bytes contain the firmware version number for the application code
#define ERROR_ID 		0xE0 // R 1 byte Error ID. When the status register reports an error its source is located in this register
#define SW_RESET 		0xFF // W 4 bytes If the correct 4 bytes (0x11 0xE5 0x72 0x8A) are written to this register in a single sequence the device will reset and return to BOOT mode.


#endif /* DRIVERS_SENSORS_CO2_CCS811_H_ */
