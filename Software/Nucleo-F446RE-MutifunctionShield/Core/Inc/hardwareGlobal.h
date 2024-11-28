/*
 * hardware_global.h
 *
 *  Created on: Nov 15, 2023
 *      Author: lukas
 *
 */

#ifndef HARDWARE_GLOBAL_H
#define HARDWARE_GLOBAL_H

#include "main.h" // -> includes "stm32f4xx_hal.h" and all drivers
#include "gpio.h"
#include "usart.h"

/* *** Platform delay (polling) ***************************************************************** */
#define MAX_DELAY        HAL_MAX_DELAY
#define DELAY(ms)        HAL_Delay(ms)
#define GET_TICK()       HAL_GetTick()

/* *** Platform return codes ***************************************************************** */
#define DEVICE_STATUS_DEFINITION        HAL_StatusTypeDef
#define DEVICE_OK                       HAL_OK
#define DEVICE_ERROR                    HAL_ERROR
#define DEVICE_BUSY                     HAL_BUSY
#define DEVICE_TIMEOUT                  HAL_TIMEOUT

/* *** GPIO ***************************************************************** */
#define SW_NUCLEO   				B1_Pin
#define NUCLEO_BUTTON_READ()		HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)

#define NUCLEO_LED_turn_on()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1)
#define NUCLEO_LED_turn_off()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0)
#define NUCLEO_LED_turn_toggle()	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)

#define MF_BUTTON_1   			S1_Pin
#define MF_BUTTON_1_READ()		HAL_GPIO_ReadPin(S1_GPIO_Port, S1_Pin)
#define MF_BUTTON_2   			S2_Pin
#define MF_BUTTON_2_READ()		HAL_GPIO_ReadPin(S2_GPIO_Port, S2_Pin)
#define MF_BUTTON_3   			S3_Pin
#define MF_BUTTON_3_READ()		HAL_GPIO_ReadPin(S3_GPIO_Port, S3_Pin)

#define MF_LED1_turn_on()		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1)
#define MF_LED1_turn_off()		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 0)
#define MF_LED1_toggle()		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin)

#define MF_LED2_turn_on()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1)
#define MF_LED2_turn_off()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0)
#define MF_LED2_toggle()		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)

#define MF_LED3_turn_on()		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1)
#define MF_LED3_turn_off()		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0)
#define MF_LED3_toggle()		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin)

#define MF_LED4_turn_on()		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 1)
#define MF_LED4_turn_off()		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 0)
#define MF_LED4_toggle()		HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin)

#define MF_BEEP_OFF()			HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET)
#define MF_BEEP_ON()			HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET)


/* *** UART ***************************************************************** */
#define UART_TERMINAL_HANDLER 	            huart2
#define UART_TERMINAL_INSTANCE 	            USART2
#define UART_SEND_TERMINAL(string, size)    HAL_UART_Transmit(&UART_TERMINAL_HANDLER, string, size, HAL_MAX_DELAY)

/* *** I2C ********************************************************************/

/* *** SPI ********************************************************************/

/* *** Hardware Timer ********************************************************************/

/* *** Watch Dog ********************************************************************/

/* *** Real Time Clock ********************************************************************/

#endif /* HARDWARE_GLOBAL_H */
