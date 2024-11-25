/*
 * app.c
 *
 *  Created on: 2 Feb 2023
 *      Author: lukasbartels
 */

#include <app.h>



void app_FSM_LED_knightrider(void) {
	static uint8_t cnt = 1;
	static uint8_t dir = 1;

	if (cnt & 0x01) {
		LED1_ON();
	} else {
		LED1_OFF();
	}

	if (cnt & 0x02) {
		LED2_ON();
	} else {
		LED2_OFF();
	}

	if (cnt & 0x04) {
		LED3_ON();
	} else {
		LED3_OFF();
	}

	if (cnt & 0x08) {
		LED4_ON();
	} else {
		LED4_OFF();
	}

	if(dir == 1) cnt = cnt<<1;
	else cnt = cnt>>1;

	if(cnt == 0x8) dir = 0;
	if(cnt == 0x1) dir = 1;
}

void app_FSM_LED_snake(void)
{
	const uint8_t MAX_LED = 0xF; // 4 Bit = 4 LED
	static uint8_t reg = 0;
	static int8_t dir = 1;

	if(reg&0x01)
	{
		LED1_ON();
	}else
	{
		LED1_OFF();
	}

	if(reg&0x02)
	{
		LED2_ON();
	}else
	{
		LED2_OFF();;
	}

	if(reg&0x04)
	{
		LED3_ON();
	}else
	{
		LED3_OFF();
	}

	if(reg&0x08)
	{
		LED4_ON();
	}else
	{
		LED4_OFF();
	}

	// State Machine
	if(MAX_LED == reg)
	{
		// start backwards
		dir = -1;
	}
	else if(0 == reg)
	{
		dir = 1;
	}
	else
	{
		__NOP();
	}

	if(1 == dir)
	{
		// left shift, fill register
		reg = reg << 1;
		reg |= 0x1;

	}else
		// right shift, empty register
		reg = reg >> 1;
}


void app_FSM_LED_bin_cunter(void)
{
	static uint8_t cnt = 0;

	if(cnt&0x01)
	{
		LED1_ON();
	}else
	{
		LED1_OFF();
	}

	if(cnt&0x02)
	{
		LED2_ON();
	}else
	{
		LED2_OFF();
	}

	if(cnt&0x04)
	{
		LED3_ON();
	}else
	{
		LED3_OFF();
	}

	if(cnt&0x08)
	{
		LED4_ON();
	}else
	{
		LED4_OFF();
	}

	cnt++;
}

void app_FSM_SEGMENT_1_counter(void)
{
	static uint8_t cnt = 0;

	mf_write_to_display_memory(0, cnt+'0');

	cnt++;
	if(cnt > 9) cnt = 0;
}

void app_FSM_SEGMENT_2_counter(void)
{
	static uint8_t cnt = 0;

	/* instead of sending data directly to the display, put that information to the display memory buffer */
	//mf_write_char(3, cnt + 'A');
	mf_write_to_display_memory(3, cnt+'A');

	cnt++;
	if(cnt > 25) cnt = 0;
}
