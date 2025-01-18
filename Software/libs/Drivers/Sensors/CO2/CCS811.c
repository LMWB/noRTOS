#include "CCS811.h"
#include "hardwareGlobal.h"
#include "stm32f446xx.h"

#define CCS811_I2C_ADDRESS	0x5A

/*
Mode 0: Idle, low current mode
Mode 1: Constant power mode, IAQ measurement every second
Mode 2: Pulse heating mode IAQ measurement every 10 seconds
Mode 3: Low power pulse heating mode IAQ measurement every 60 seconds
Mode 4: Constant power mode, sensor measurement every 250ms

In Modes 1, 2, 3, the equivalent CO 2 concentration (ppm) and TVOC concentration (ppb) are calculated for every sample.
Mode 1 reacts fastest to gas presence, but has a higher operating current
Mode 3 reacts more slowly to gas presence but has the lowest average operating current.
*/

#define OPMODE_IDLE 			0
#define OPMODE_NORMAL 			1
#define OPMODE_PULSE 			2
#define OPMODE_LOW_POWER 		3
#define OPMODE_CONSTANT_POWER 	4

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
#define APP_START 		0xF4 // Application start. Used to transition the CCS811 state from boot to application mode, a write with no data is required. Before performing a write to APP_START the Status register should be accessed to check if there is a valid application present.

static void ccs811_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length);
static void ccs811_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length);

static void ccs811_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Read(&I2C_HANDLER, CCS811_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

static void ccs811_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Write(&I2C_HANDLER, CCS811_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

static void ccs811_delay(uint32_t milli_sec){
	HAL_Delay(milli_sec);
}

uint8_t ccs811_read_status_register(uint8_t* satus_byte){
	ccs811_i2c_read_register(STATUS, satus_byte, 1);
	return 1;
}

uint8_t ccs811_read_mode_register(uint8_t* mode_byte){
	ccs811_i2c_read_register(MEAS_MODE, mode_byte, 1);
	return 1;
}

uint8_t ccs811_write_mode_register(uint8_t mode_byte){
	ccs811_i2c_write_register(MEAS_MODE, &mode_byte, 1);
	return 1;
}

uint8_t ccs811_read_HW_ID_register(uint8_t* hw_byte){
	ccs811_i2c_read_register(HW_ID, hw_byte, 1);
	return 1;
}

uint8_t ccs811_read_HW_version_register(uint8_t* version_byte){
	ccs811_i2c_read_register(HW_Version, version_byte, 1);
	return 1;
}

uint8_t ccs811_read_error_ID_register(uint8_t* error_byte){
	ccs811_i2c_read_register(ERROR_ID, error_byte, 1);
	return 1;
}

void ccs811_soft_reset(void){
	uint8_t soft_rest_sequenc[4] = {0x11, 0xE5, 0x72, 0x8A};
	ccs811_i2c_write_register(SW_RESET, soft_rest_sequenc, 4);
}

void ccs811_app_start(void){
	ccs811_i2c_write_register(APP_START, NULL, 0);
}

void ccs811_init(void){

	ccs811_soft_reset();
	ccs811_delay(5);

	// read status register
	uint8_t status = 0;
	ccs811_read_status_register(&status);
	status = status;

	ccs811_app_start();
	ccs811_delay(5);

	// read status register
	status = 0;
	ccs811_read_status_register(&status);
	status = status;

	// read error register
	uint8_t error_ID = 0;
	ccs811_read_error_ID_register(&error_ID);
	error_ID = error_ID;

	// read mode register
	uint8_t mode = 0;
	ccs811_read_mode_register(&mode);
	mode = mode;

	// set measurement mode 1, not interrupts
	uint8_t meas_mode = 0;
	meas_mode = meas_mode | (OPMODE_NORMAL << 4);
	ccs811_write_mode_register(meas_mode);

	// read mode register again
	ccs811_read_mode_register(&mode);
	mode = mode;

	// read hardware ID register, should be 0x81
	uint8_t hw_ID = 0;
	ccs811_read_HW_ID_register(&hw_ID);
	hw_ID = hw_ID;

	// read hardwere version register. should be 0x1X
	uint8_t hw_version = 0;
	ccs811_read_HW_version_register(&hw_version);
	hw_version = hw_version;

	// read error register
	error_ID = 0;
	ccs811_read_error_ID_register(&error_ID);
	error_ID = error_ID;
}

void ccs811_sample_data(ccs811_data_t* data){
	// read 8 byte of data register, if byte 4 bit 3 (status byte data valid bit) is true, data is valid
	uint8_t alg_result_data[8] = {0};
	ccs811_i2c_read_register(ALG_RESULT_DATA, alg_result_data, 8);
	data->eCO2 		= (alg_result_data[0] << 8) | alg_result_data[1];
	data->voc 		= (alg_result_data[2] << 8) | alg_result_data[3];
	data->status 	= alg_result_data[4];
	data->errorID 	= alg_result_data[5];
}
