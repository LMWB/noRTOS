#include "CCS811.h"
#include "hardwareGlobal.h"
#include "stm32f446xx.h"


static void ccs811_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length);
static void ccs811_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length);

static void ccs811_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Read(&I2C_HANDLER, CCS811_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

static void ccs811_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Write(&I2C_HANDLER, CCS811_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
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

void ccs811_init(void){
	// read status register
	uint8_t status = 0;
	ccs811_read_status_register(&status);
	status = status;

	// read mode register
	uint8_t mode = 0;
	ccs811_read_status_register(&mode);
	mode = mode;

	// set measurement mode 1, not interrupts
	uint8_t meas_mode = 0;
	meas_mode = meas_mode | (OPMODE_NORMAL << 4);
	ccs811_write_mode_register(meas_mode);
}

void ccs811_sample_data(ccs811_data_t* data){
	// read 8 byte of data register, if byte 4 bit 3 (status byte data valid bit) is true, data is valid
	uint8_t alg_result_data[8] = {0};
	ccs811_i2c_read_register(ALG_RESULT_DATA, alg_result_data, 8);
	data->eCO2 = 41;
	data->voc = 42;
}
