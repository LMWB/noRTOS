#ifndef DRIVERS_AHT21_AHT21_H_
#define DRIVERS_AHT21_AHT21_H_

#include "main.h"

#define AHT21_I2C_ADDR (0x38 << 1)

// Befehle
#define AHT21_CMD_INIT    0xBE
#define AHT21_CMD_TRIGGER 0xAC

HAL_StatusTypeDef AHT21_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AHT21_ReadData(I2C_HandleTypeDef *hi2c, int16_t *temp_x10, int16_t *hum_x10);


#endif /* DRIVERS_AHT21_AHT21_H_ */
