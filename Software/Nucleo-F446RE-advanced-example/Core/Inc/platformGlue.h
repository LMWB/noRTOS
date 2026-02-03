#ifndef PLATFORM_GLUE_H
#define PLATFORM_GLUE_H

#define PLATFORM_STM32F446

#define PLATFORM_HAS_UART
//#define PLATFORM_HAS_TIMER
//#define PLATFORM_HAS_I2C
//#define PLATFORM_HAS_SPI
//#define PLATFORM_HAS_WATCHDOG
//#define PLATFORM_HAS_CAN
//#define PLATFORM_HAS_RTC

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

#define TOGGLE_PIN(PORT, PIN)		HAL_GPIO_TogglePin(PORT, PIN)			/* void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) */
#define WRITE_PIN(PORT, PIN, STATE)	HAL_GPIO_WritePin(PORT, PIN, STATE); 	/* void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) */
#define READ_PIN(PORT, PIN)			HAL_GPIO_ReadPin(PORT, PIN)

/* *** UART *************************************************************************** */
#ifdef PLATFORM_HAS_UART
#define UART_TERMINAL_HANDLER 	            		huart2
#define UART_TERMINAL_INSTANCE 	            		USART2
#define UART_TERMINAL_SEND(string, size)    		HAL_UART_Transmit(&UART_TERMINAL_HANDLER, string, size, HAL_MAX_DELAY)

#define UART_TERMINAL_READ_BYTE_IRQ(buffer)				HAL_UART_Receive_IT(&UART_TERMINAL_HANDLER, buffer, 1)
#define UART_TERMINAL_READ_LINE_IRQ(buffer, size)		HAL_UARTEx_ReceiveToIdle_IT(&UART_TERMINAL_HANDLER, buffer, size)
#define UART_TERMINAL_READ_LINE_IRQ_DMA(buffer, size)	HAL_UARTEx_ReceiveToIdle_DMA(&UART_TERMINAL_HANDLER, buffer, size)
#endif

/* *** I2C ********************************************************************************/
#ifdef PLATFORM_HAS_I2C

#endif
/* *** SPI ********************************************************************************/
#ifdef PLATFORM_HAS_SPI

#endif
/* *** Hardware Timer *********************************************************************/
#ifdef PLATFORM_HAS_TIMER

#endif
/* *** Watch Dog **************************************************************************/
#ifdef PLATFORM_HAS_WATCHDOG

#endif
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

/* *** CAN ********************************************************************/
#ifdef PLATFORM_HAS_RTC
#include "can.h"
#define CAN_HANDLER 							hcan
#define CAN_INSTANCE            				CAN
#define CAN_CONFIGURE_FILTER(canFilterConfig)	HAL_CAN_ConfigFilter(&CAN_HANDLER, &canFilterConfig)
#define CAN_START_PERIPHERAL() 					HAL_CAN_Start(&CAN_HANDLER)
#define CAN_STOP_PERIPHERAL() 					HAL_CAN_Stop(&CAN_HANDLER)
#define CAN_ACTIVATE_RX_INTERRUPT() 			HAL_CAN_ActivateNotification(&CAN_HANDLER, CAN_IT_RX_FIFO0_MSG_PENDING)
static uint32_t  TxMailbox1;
#define SEND_CAN_MESSAGE(txHeader, txData) 		HAL_CAN_AddTxMessage(&CAN_HANDLER, &txHeader, txData, &TxMailbox1);
#endif

/* *** UID ********************************************************************/
typedef union {
	uint32_t uid[3];
	uint8_t uid_byte[12]; // 3 * uint32 = 3 * 4bytes = 12 uint8
} cpu_uid_t;

#define READ_CPU_UID(cpu_uid_type)	{cpu_uid_type.uid[0] = HAL_GetUIDw0(); cpu_uid_type.uid[1] = HAL_GetUIDw1(); cpu_uid_type.uid[2] = HAL_GetUIDw2();}


#endif /* PLATFORM_GLUE_H */
