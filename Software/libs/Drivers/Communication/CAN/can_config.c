#include "can_config.h"

static bool can_message_pending = false;

bool is_can_message_pending(void){
	return can_message_pending;
}

void set_can_message_pending(void){
	can_message_pending = true;
}

/* not really a fifo yet */
bool pop_can_message_from_fifo(uint8_t** pointer_to_payload){
	if(can_message_pending){
		can_message_pending = false;
		*pointer_to_payload = &canRxData[0];
		return true;
	}else{
		*pointer_to_payload = NULL;
		return false;
	}
}

#if defined(PLATFORM_STM32F446)
// Instantiation of global CAN variables
CAN_FilterTypeDef canFilterConfig;
CAN_TxHeaderTypeDef canTxHeader;
CAN_RxHeaderTypeDef canRxHeader;

uint8_t canTxData[8] = {'R', 'R', '-', 'F', '4', '4', '6', '\n'}; // define 8 bytes because 8 is the maximum any way
uint8_t canRxData[8];
uint32_t canID = 0xF446;

// void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) -> Interrupt is placed in lemonbee.c

void set_can_extended_ID_filter(uint32_t extID)
{
	CAN_STOP_PERIPHERAL();

	canFilterConfig.FilterActivation = ENABLE;
	canFilterConfig.FilterBank = 0; // Use filter bank 0
	canFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    // Set filter mode and scale for extended ID
	canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

    // Convert 29-bit extended ID to standard 32-bit format for filter
    // Bits: [31:3] = ID[28:0] << 3, [2:0] = CAN ID type flags
    uint32_t extID_FilterFormat = (extID << 3) | (1 << 2); // Set IDE bit for extended ID

    // Filter mask = all bits must match
    canFilterConfig.FilterIdHigh = (extID_FilterFormat >> 16) & 0xFFFF;
    canFilterConfig.FilterIdLow  = extID_FilterFormat & 0xFFFF;
    canFilterConfig.FilterMaskIdHigh = 0xFFFF;
    canFilterConfig.FilterMaskIdLow  = 0xFFFF;

    CAN_CONFIGURE_FILTER( canFilterConfig );
    CAN_START_PERIPHERAL();
    CAN_ACTIVATE_RX_INTERRUPT();
}

void configure_can_tx_identifier(uint32_t extID)
{
	canTxHeader.ExtId = 0x1FFFFFFF & extID;
	canTxHeader.RTR = CAN_RTR_DATA;
	canTxHeader.IDE = CAN_ID_EXT;
}

void configure_can_tx_data(uint8_t dlc, uint8_t* data )
{
	if(dlc <= 8)
	{
		canTxHeader.DLC = dlc;
		memcpy(canTxData, data, dlc);
	}
}

#endif



#if defined(PLATFORM_STM32C092) || defined(PLATFORM_STM32G0B1)
FDCAN_FilterTypeDef canFilterConfig;
FDCAN_RxHeaderTypeDef canRxHeader;
FDCAN_TxHeaderTypeDef canTxHeader;

uint8_t canTxData[8] = {'R','R','-','C','0','9','2','\n', }; // define 8 bytes because 8 is the maximum any way
uint8_t canRxData[8];
uint32_t canID = 0xC091;

void set_can_extended_ID_filter(uint32_t extID)
{
	// todo

    CAN_START_PERIPHERAL();
    CAN_ACTIVATE_RX_INTERRUPT();
}

void configure_can_tx_identifier(uint32_t extID)
{
	canTxHeader.Identifier = 0x1FFFFFFF & extID;
	canTxHeader.IdType = FDCAN_EXTENDED_ID;
	canTxHeader.TxFrameType = FDCAN_DATA_FRAME;
}

void configure_can_tx_data(uint8_t dlc, uint8_t* data )
{
	if(dlc <= 8)
	{
		canTxHeader.DataLength = dlc;
		memcpy(canTxData, data, dlc);
	}
}

#endif

// just for debugging
static uint8_t uartTXbuffer[128];
static uint8_t uartRXbuffer[32];

/* most important function in this API */
void send_can_message(uint32_t CAN_ID, uint8_t* can_data, uint8_t can_data_length){
	configure_can_tx_identifier(CAN_ID);
	configure_can_tx_data(can_data_length, can_data);
	SEND_CAN_MESSAGE(canTxHeader, can_data);
}
