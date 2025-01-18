#include "SCD4x.h"
#include "hardwareGlobal.h"
#include "stm32f446xx.h"
#include "stdio.h"


#define SCD4x_I2C_ADDRESS						0x62

#define SCD4x_CMD_START_PERIODIC_MEASUREMENT	0x21b1UL
#define SCD4x_CMD_REINIT 						0x3646UL
#define SCD4x_CMD_GET_SERIAL_NUMBER 			0x3682UL
#define SCD4x_CMD_WAKE_UP 						0x36f6UL
#define SCD4x_CMD_STOP_PERIODIC_MEASUREMENT 	0x3f86UL
#define SCD4x_CMD_GET_DATA_READY_STATUS 		0xe4b8UL
#define SCD4x_CMD_READ_MEASUREMENT 				0xec05UL

#define SCD4x_I2C_BUFFER_SIZE 					128
static uint8_t scd4x_i2c_buffer[SCD4x_I2C_BUFFER_SIZE];

static void scd4x_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length);
static void scd4x_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length);
static void scd4x_i2c_write(uint8_t *data, uint16_t size);
static void scd4x_i2c_read(uint8_t *data, uint16_t count);

// do not use
static void scd4x_i2c_read_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Read(&I2C_HANDLER, SCD4x_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

// do not use
static void scd4x_i2c_write_register(uint8_t register_address, uint8_t* data, uint16_t length){
	HAL_I2C_Mem_Write(&I2C_HANDLER, SCD4x_I2C_ADDRESS<<1, register_address, 1, data, length, 0xFFFF);
}

static void scd4x_i2c_write(uint8_t *data, uint16_t size) {
	HAL_I2C_Master_Transmit(&I2C_HANDLER, (uint16_t) (SCD4x_I2C_ADDRESS << 1), data, size, 0xFFFF);
}

static void scd4x_i2c_read(uint8_t *data, uint16_t count) {
	HAL_I2C_Master_Receive(&I2C_HANDLER, (uint16_t) (SCD4x_I2C_ADDRESS << 1), data, count, 0xFFFF);
}

void scd4x_sleep(uint32_t milli_seconds){
	HAL_Delay(milli_seconds);
}

/* -------------------------------------------------------------------------------------------------- */

void sdc4x_init(void){
	// Clean up potential SCD40 states
	scd4x_wake_up();
	scd4x_stop_periodic_measurement();
	scd4x_reinit();

	uint16_t serial_0;
	uint16_t serial_1;
	uint16_t serial_2;
	scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
	printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
	// Start Measurement
	scd4x_start_periodic_measurement();
}


void scd4x_wake_up(void){
	uint16_t size = 2;
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_WAKE_UP & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_WAKE_UP & 0x00FF) >> 0);

	// Sensor does not acknowledge the wake-up call, error is ignored
	scd4x_i2c_write( &scd4x_i2c_buffer[0], size);
	scd4x_sleep(200);

}
void scd4x_stop_periodic_measurement(void){
	uint16_t size = 2;
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_STOP_PERIODIC_MEASUREMENT & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_STOP_PERIODIC_MEASUREMENT & 0x00FF) >> 0);
	scd4x_i2c_write( &scd4x_i2c_buffer[0], size);
	scd4x_sleep(500);

}
void scd4x_reinit(void){
	uint16_t size = 2;
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_REINIT & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_REINIT & 0xFF00) >> 0);
	scd4x_i2c_write( &scd4x_i2c_buffer[0], size);
	scd4x_sleep(20);

}
void scd4x_get_serial_number(uint16_t *serial_0, uint16_t *serial_1, uint16_t *serial_2){
	uint16_t write_size = 2;
	uint16_t read_size = 9;

	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_GET_SERIAL_NUMBER & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_GET_SERIAL_NUMBER & 0x00FF) >> 0);
	scd4x_i2c_write( &scd4x_i2c_buffer[0], write_size);
	scd4x_sleep(1);

	//sensirion_i2c_read_data_inplace(SCD4X_I2C_ADDRESS, &buffer[0], 6);
	scd4x_i2c_read(scd4x_i2c_buffer, read_size);

	//*serial_0 = sensirion_common_bytes_to_uint16_t(&buffer[0]);
	*serial_0 = (uint16_t) scd4x_i2c_buffer[0] << 8 | (uint16_t) scd4x_i2c_buffer[1];
	//*serial_1 = sensirion_common_bytes_to_uint16_t(&buffer[2]);
	*serial_1 = (uint16_t) scd4x_i2c_buffer[2] << 8 | (uint16_t) scd4x_i2c_buffer[3];
	//*serial_2 = sensirion_common_bytes_to_uint16_t(&buffer[4]);
	*serial_2 = (uint16_t) scd4x_i2c_buffer[4] << 8 | (uint16_t) scd4x_i2c_buffer[5];

}

void scd4x_start_periodic_measurement(void){
	uint16_t size = 2;
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_START_PERIODIC_MEASUREMENT & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_START_PERIODIC_MEASUREMENT & 0x00FF) >> 0);

	scd4x_i2c_write( &scd4x_i2c_buffer[0], size);
	scd4x_sleep(1);
}

void scd4x_get_data_ready_flag(bool *data_ready_flag){

	uint16_t local_data_ready = 0;
	uint16_t write_size = 2;
	uint16_t read_size = 3;
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_GET_DATA_READY_STATUS & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_GET_DATA_READY_STATUS & 0x00FF) >> 0);
	scd4x_i2c_write( &scd4x_i2c_buffer[0], write_size);
	scd4x_sleep(1);
	scd4x_i2c_read(scd4x_i2c_buffer, read_size);
	local_data_ready = (uint16_t) scd4x_i2c_buffer[0] << 8 | (uint16_t) scd4x_i2c_buffer[1];
	*data_ready_flag = (local_data_ready & 0x07FF) != 0;
}

void scd4x_read_measurement(uint16_t *co2, int32_t *temperature_m_deg_c, int32_t *humidity_m_percent_rh){
	int32_t local_temperature;
	int32_t local_humidity;
	uint16_t local_co2;
	uint16_t write_size = 2;
	uint16_t read_size = 9;

	// scd4x_read_measurement_ticks(co2, &temperature, &humidity);
	scd4x_i2c_buffer[0] = (uint8_t) ((SCD4x_CMD_READ_MEASUREMENT & 0xFF00) >> 8);
	scd4x_i2c_buffer[1] = (uint8_t) ((SCD4x_CMD_READ_MEASUREMENT & 0x00FF) >> 0);
	scd4x_i2c_write( &scd4x_i2c_buffer[0], write_size);
	scd4x_sleep(1);

	//sensirion_i2c_read_data_inplace(SCD4X_I2C_ADDRESS, &buffer[0], 6);
	scd4x_i2c_read(scd4x_i2c_buffer, read_size);

	//local_co2			= sensirion_common_bytes_to_uint16_t(&buffer[0]);
	local_co2			= (uint16_t) scd4x_i2c_buffer[0] << 8 | (uint16_t) scd4x_i2c_buffer[1];
	//local_temperature 	= sensirion_common_bytes_to_uint16_t(&buffer[2]);
	local_temperature 	= (uint16_t) scd4x_i2c_buffer[3] << 8 | (uint16_t) scd4x_i2c_buffer[4];
	//local_humidity 		= sensirion_common_bytes_to_uint16_t(&buffer[4]);
	local_humidity 		= (uint16_t) scd4x_i2c_buffer[6] << 8 | (uint16_t) scd4x_i2c_buffer[7];

	*co2 = local_co2;
	*temperature_m_deg_c = ((21875 * (int32_t) local_temperature) >> 13) - 45000;
	*humidity_m_percent_rh = ((12500 * (int32_t) local_humidity) >> 13);
}
