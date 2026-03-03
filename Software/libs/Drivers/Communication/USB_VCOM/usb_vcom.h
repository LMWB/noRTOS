/*
 * usb_vcom.h
 *
 *  Created on: Feb 23, 2026
 *      Author: lukas
 */

#ifndef DRIVERS_COMMUNICATION_USB_VCOM_USB_VCOM_H_
#define DRIVERS_COMMUNICATION_USB_VCOM_USB_VCOM_H_

/* *************** USB VCOM Buffer *************** */
#include <stdint.h>

/* */
#define USB_BUFFER_SIZE 512

/* */
extern uint8_t usb_RX_buffer[USB_BUFFER_SIZE];

/* */
int vcom_write(char *ptr, int len);

#endif /* DRIVERS_COMMUNICATION_USB_VCOM_USB_VCOM_H_ */
