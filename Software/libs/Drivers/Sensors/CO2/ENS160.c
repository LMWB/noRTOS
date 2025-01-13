#include "ENS160.h"
#include "hardwareGlobal.h"
#include "stm32f446xx.h"


void ens160_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Read(&I2C_HANDLER, ENS160_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

void ens160_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Write(&I2C_HANDLER, ENS160_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}


// todo printf muss weg
void ens160_translate_status_byte(uint8_t status_byte){
	uint8_t NEWGPR;
	uint8_t NEWDAT;
	uint8_t VALIDITY_FLAG;
	uint8_t STATER;
	uint8_t STATAS;
	NEWGPR 			= (status_byte & 0x01);		 // High indicates that a new data is available in the GPR_READx registers. Cleared automatically at first GPR_READx read.
	NEWDAT 			= (status_byte & 0x02) >> 1; // High indicates that a new data is available in the DATA_x registers. Cleared automatically at first DATA_x read.
	VALIDITY_FLAG 	= (status_byte & 0x0C) >> 2; // 0: Normal operation; 1: Warm-Up phase; 2: Initial Start-Up phase; 3: Invalid output
	STATER 			= (status_byte & 0x40) >> 6; // High indicates that an error is detected. E.g. Invali Operating Mode has been selected.
	STATAS 			= (status_byte & 0x80) >> 7; // High indicates that an OPMODE is running
	printf("ENS160 Status: ");
	if(NEWGPR) printf("GPR data available, ");
	if(NEWDAT) printf("Sensor data available, ");
	if(VALIDITY_FLAG) printf("Operation: %d, ", VALIDITY_FLAG);
	if(STATER) printf("Error is present, ");
	if(STATAS) printf("OPMODE is running");
	printf("\n");
}
