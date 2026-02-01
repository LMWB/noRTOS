
#ifndef PLATFORM_GLUE_H
#define PLATFORM_GLUE_H

#define PLATFORM_STM32F446

//#define HAS_UART
//#define HAS_TIMER
//#define HAS_I2C
//#define HAS_SPI
//#define HAS_WATCHDOG
//#define HAS_CAN
#define PLATFORM_HAS_RTC

#include "main.h" // -> includes "stm32f4xx_hal.h" and all drivers
#include "gpio.h"
#include "usart.h"


/* *** Platform delay (polling) ******************************************************** */
#define MAX_DELAY        HAL_MAX_DELAY
#define DELAY(ms)        HAL_Delay(ms)
#define GET_TICK()       HAL_GetTick()

/* *** Platform return codes ********************************************************** */
#define DEVICE_STATUS_DEFINITION        HAL_StatusTypeDef
#define DEVICE_OK                       HAL_OK
#define DEVICE_ERROR                    HAL_ERROR
#define DEVICE_BUSY                     HAL_BUSY
#define DEVICE_TIMEOUT                  HAL_TIMEOUT

/* *** GPIO *************************************************************************** */
#define SW_NUCLEO   				B1_Pin
#define NUCLEO_BUTTON_READ()		HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)

#define NUCLEO_LED_turn_on()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1)
#define NUCLEO_LED_turn_off()		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0)
#define NUCLEO_LED_toggle()			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)

// green
#define LED_GREEN_ON() 			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET)
#define LED_GREEN_OFF()			HAL_GPIO_WritePin(	USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_RESET)
#define LED_GREEN_TOGGLE()		HAL_GPIO_TogglePin(	USER_LED1_GPIO_Port, USER_LED1_Pin)

// red
#define LED_RED_ON() 			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_SET)
#define LED_RED_OFF()			HAL_GPIO_WritePin(	USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_RESET)
#define LED_RED_TOGGLE()		HAL_GPIO_TogglePin(	USER_LED2_GPIO_Port, USER_LED2_Pin)

// green it has no yellow
#define LED_YELLOW_ON()			HAL_GPIO_WritePin(	USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_SET)
#define LED_YELLOW_OFF()		HAL_GPIO_WritePin(  USER_LED3_GPIO_Port, USER_LED3_Pin, GPIO_PIN_RESET)
#define LED_YELLOW_TOGGLE()		HAL_GPIO_TogglePin(	USER_LED3_GPIO_Port, USER_LED3_Pin)

// red as well
#define LED4_ON() 				HAL_GPIO_WritePin(	USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_SET)
#define LED4_OFF() 				HAL_GPIO_WritePin(  USER_LED4_GPIO_Port, USER_LED4_Pin, GPIO_PIN_RESET)
#define LED4_TOGGLE()			HAL_GPIO_TogglePin( USER_LED4_GPIO_Port, USER_LED4_Pin)


/* *** UART *************************************************************************** */
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

/* *** I2C ********************************************************************************/

/* *** SPI ********************************************************************************/

/* *** Hardware Timer *********************************************************************/

/* *** Watch Dog **************************************************************************/

/* *** Real Time Clock ********************************************************************/
#ifdef PLATFORM_HAS_RTC
#include "rtc.h"
#define RTC_HANDLER             hrtc
#define RTC_INSTANCE            RTC
#define SET_DEVICE_TIME(sTime)  HAL_RTC_SetTime( &RTC_HANDLER, sTime, RTC_FORMAT_BIN)
#define SET_DEVICE_DATE(sDate)  HAL_RTC_SetDate( &RTC_HANDLER, sDate, RTC_FORMAT_BIN)
#define GET_DEVICE_TIME(sTime)  HAL_RTC_GetTime( &RTC_HANDLER, sTime, RTC_FORMAT_BIN)
#define GET_DEVICE_DATE(sDate)  HAL_RTC_GetDate( &RTC_HANDLER, sDate, RTC_FORMAT_BIN)
#endif

#endif /* PLATFORM_GLUE_H*/
