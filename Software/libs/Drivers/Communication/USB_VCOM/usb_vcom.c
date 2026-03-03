#include "usb_vcom.h"

/* *************** USB VCOM Buffer *************** */
uint8_t usb_RX_buffer[USB_BUFFER_SIZE] = {0}; // used at usbd_csc_if.c line 266
uint32_t msg_cnt = 0;

int vcom_write(char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*) ptr, (uint16_t)len);
	return len;
}
