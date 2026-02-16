#ifndef CAN_CAN_CONFIG_H_
#define CAN_CAN_CONFIG_H_

#include "platformGlue.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define CANID_SIGNALS_BASE	 		0x1000
#define CANID_OFFSET_ANALOG1 		0x01
#define CANID_OFFSET_ANALOG2 		0x02
#define CANID_OFFSET_DIGITAL1 		0x03
#define CANID_OFFSET_TEMPERATURE 	0x04

bool is_can_message_pending(void);
void set_can_message_pending(void);

bool pop_can_message_from_fifo(uint8_t** pointer_to_payload);

#if defined(PLATFORM_STM32F446)
// Instantiation of global CAN variables
extern CAN_FilterTypeDef canFilterConfig;
extern CAN_TxHeaderTypeDef canTxHeader;
extern CAN_RxHeaderTypeDef canRxHeader;

extern uint8_t canTxData[8]; // define 8 bytes because 8 is the maximum any way
extern uint8_t canRxData[8];

extern uint32_t canID;
#endif


#if defined(PLATFORM_STM32C092) || defined(PLATFORM_STM32G0B1)
extern FDCAN_FilterTypeDef canFilterConfig;
extern FDCAN_RxHeaderTypeDef canRxHeader;
extern FDCAN_TxHeaderTypeDef canTxHeader;

extern uint8_t canTxData[8];
extern uint8_t canRxData[8];

extern uint32_t canID;
#endif

void set_can_extended_ID_filter(uint32_t extID);
void configure_can_tx_identifier(uint32_t extID);
void configure_can_tx_data(uint8_t dlc, uint8_t* data );

void send_can_message(uint32_t CAN_ID, uint8_t* can_data, uint8_t can_data_length);

#endif /* CAN_CAN_CONFIG_H_ */
