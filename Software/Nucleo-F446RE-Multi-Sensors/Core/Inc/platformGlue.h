#ifndef PLATFORM_GLUE_H
#define PLATFORM_GLUE_H

#define PLATFORM_STM32F446

//#define PLATFORM_HAS_UART
//#define PLATFORM_HAS_TIMER
#define PLATFORM_HAS_I2C
//#define PLATFORM_HAS_SPI
//#define PLATFORM_HAS_WATCHDOG
//#define PLATFORM_HAS_CAN
//#define PLATFORM_HAS_RTC

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
#define NUCLEO_LED_toggle()			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)


/* *** UART ***************************************************************** */
#define UART_TERMINAL_HANDLER 	            		huart2
#define UART_TERMINAL_INSTANCE 	            		USART2
#define UART_TERMINAL_SEND(string, size)    		HAL_UART_Transmit(&UART_TERMINAL_HANDLER, string, size, HAL_MAX_DELAY)

#define UART_TERMINAL_READ_BYTE_IRQ(buffer)			HAL_UART_Receive_IT(&UART_TERMINAL_HANDLER, buffer, 1)
#define UART_TERMINAL_READ_LINE_IRQ(buffer, size)	HAL_UARTEx_ReceiveToIdle_DMA(&UART_TERMINAL_HANDLER, buffer, size)


#define UART_INTERNET_HANDLER 	            		huart1
#define UART_INTERNET_INSTANCE 	            		USART1
#define UART_INTERNET_SEND(string, size)    		HAL_UART_Transmit(&UART_INTERNET_HANDLER, string, size, HAL_MAX_DELAY)

#define UART_INTERNET_READ_BYTE_IRQ(buffer)			HAL_UART_Receive_IT(			&UART_INTERNET_HANDLER, buffer, 1)
#define UART_INTERNET_READ_LINE_IRQ(buffer, size)	HAL_UARTEx_ReceiveToIdle_DMA(	&UART_INTERNET_HANDLER, buffer, size)
#define UART_INTERNET_ABORT_IRQ()					HAL_UART_AbortReceive_IT(		&UART_INTERNET_HANDLER)

/* *** I2C ********************************************************************/
#ifdef PLATFORM_HAS_I2C
#include "i2c.h"
#define I2C_HANDLER 				        		hi2c1
#define I2C_GAS_SENSOR                  			I2C1_HANDLER
#define I2C_INSTANCE				        		I2C1
#define IS_I2C_DEVICE_READY(dev_address)    		HAL_I2C_IsDeviceReady(&I2C_HANDLER, dev_address, 1, 100)
#endif

/* *** SPI ********************************************************************/

/* *** Hardware Timer ********************************************************************/

/* *** Watch Dog ********************************************************************/

/* *** Real Time Clock ********************************************************************/

#endif /* PLATFORM_GLUE_H */
