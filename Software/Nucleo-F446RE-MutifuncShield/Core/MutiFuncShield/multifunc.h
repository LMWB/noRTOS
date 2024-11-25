/*
 * multifunc.h
 *
 *  Created on: Feb 1, 2023
 *      Author: lukasbartels
 */

#ifndef MUTIFUNCSHIELD_MULTIFUNC_H_
#define MUTIFUNCSHIELD_MULTIFUNC_H_

#include <main.h>
#include <gpio.h>

/* LEDs are low active */
#define LED1_OFF() 		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET)
#define LED1_ON() 		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET)
#define LED1_TOGGLE()	HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin)

#define LED2_OFF() 		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET)
#define LED2_ON() 		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)
#define LED2_TOGGLE()	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)

#define LED3_OFF() 		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET)
#define LED3_ON() 		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET)
#define LED3_TOGGLE()	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin)

#define LED4_OFF() 		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET)
#define LED4_ON() 		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET)
#define LED4_TOGGLE()	HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin)

/* Buzzer Stuff */
#define BEEP_OFF()		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET)
#define BEEP_ON()		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET)

/* Shift Register Stuff */
#define SHCP_PIN_LOW()	HAL_GPIO_WritePin(SHCP_GPIO_Port, SHCP_Pin, 0)
#define SHCP_PIN_HIGH()	HAL_GPIO_WritePin(SHCP_GPIO_Port, SHCP_Pin, 1)

#define STCP_PIN_LOW()	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 0)
#define STCP_PIN_HIGH()	HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, 1)




void mf_reset_segment(void);
void mf_refresh_segment(void);
void mf_demo_segment(void);
void mf_write_to_display_memory(const uint8_t segment, char data);

char mf_uint_to_hex(uint8_t uint);

/* make this one static and move it to multifunc.c */
void mf_write_char(const uint8_t segment, char data);

void mf_delay(void);






#endif /* MUTIFUNCSHIELD_MULTIFUNC_H_ */
