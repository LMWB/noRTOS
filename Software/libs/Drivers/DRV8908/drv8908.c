#include "drv8908.h"
#include "platformGlue.h"


/* Register addresses */
#define REG_CONFIG      0x07U
#define REG_OP_CTRL_1   0x08U    /* HB1..HB4: bit pairs [HS,LS] */
#define REG_OP_CTRL_2   0x09U    /* HB5..HB8 */

#define PWM_FREQ_CTRL_1	0x13
#define PWM_FREQ_CTRL_2	0x14

#define SR_CTRL_1		0x1D
#define SR_CTRL_2		0x1E

#define OLD_CTRL_1 		0x1F
#define OLD_CTRL_2 		0x20
#define OLD_CTRL_3 		0x21
#define OLD_CTRL_4 		0x22
#define OLD_CTRL_5 		0x23
#define OLD_CTRL_6 		0x24

/* --- low-level helpers --- */

/* Transfer a single 16-bit word (MSB first). Returns the 16-bit response.
 * SPI Full-Duplex-Master, Motorola MSB first, 8Bit, CPOL->Low, CPHA->2Edge
 * 2,625 MBits/s
 * */
static uint16_t drv_spi_xfer(uint16_t word)
{
    uint8_t tx[2], rx[2];
    tx[0] = (uint8_t)(word >> 8);
    tx[1] = (uint8_t)(word & 0xFF);
    rx[0] = rx[1] = 0;

    SPI_CS_LOW();
    SPI_SEND_RECEIVE(tx, rx, 2);
    SPI_CS_HIGH();

    return (uint16_t)((rx[0] << 8) | rx[1]);
}

/* Note: SDI frame format (16 bits):
   B15 = (reserved/header 0), B14 = W (1=read,0=write), B13..B8 = addr[5:0], B7..B0 = data
*/
static void drv_write_reg(uint8_t addr, uint8_t value)
{
    uint16_t cmd = (uint16_t)((addr & 0x3FU) << 8) | (uint16_t)value; /* W = 0 => write */
    (void)drv_spi_xfer(cmd); /* write returns old register value on SDO, ignore here */
}

static uint8_t drv_read_reg(uint8_t addr)
{
    uint16_t cmd = (uint16_t)(1U << 14) | (uint16_t)((addr & 0x3FU) << 8); /* W = 1 => read */
    uint16_t resp = drv_spi_xfer(cmd);
    return (uint8_t)(resp & 0xFF); /* low byte is register data / report */
}

/* --- high-level: treat channels as 0..7 (HB1..HB8) --- */
void drv8908_Init(void)
{
    /* wake device */
    // no need nSLEEP Pin tide always high
    HAL_Delay(5);

    uint8_t reg = drv_read_reg(REG_CONFIG);
    reg = drv_read_reg(REG_CONFIG);

    /* clear faults (CLR_FLT = bit0 of CONFIG) by writing 1 (auto-clears) */
    drv_write_reg(REG_CONFIG, 0x01U);
    HAL_Delay(10);

    reg = drv_read_reg(REG_CONFIG);
    reg = drv_read_reg(REG_CONFIG);
    reg = drv_read_reg(REG_CONFIG);
    reg = drv_read_reg(REG_OP_CTRL_1);
    reg = drv_read_reg(REG_OP_CTRL_2);
    reg = drv_read_reg(OLD_CTRL_1);
    reg = drv_read_reg(OLD_CTRL_2);
    reg = drv_read_reg(OLD_CTRL_3);
    reg = drv_read_reg(OLD_CTRL_4);
    reg = drv_read_reg(OLD_CTRL_5);
    reg = drv_read_reg(OLD_CTRL_6);

    drv_write_reg(OLD_CTRL_1, 0b11111111); // Disable open load detection on channels 1-8
//    drv_write_reg(OLD_CTRL_2, 0b11001111); // Disable errors from open load detection, open load detect on channels 9-12
    drv_write_reg(OLD_CTRL_3, 0b10000000); // set Overcurrent protection to the most forgiving setting
    drv_write_reg(SR_CTRL_1, 0b11111111); // Set slew rate to 2.5us vrs default 0.6us on half bridges (1-8)
//    drv_write_reg(SR_CTRL_2,  0b00001111); // Set slew rate to 2.5us vrs default 0.6us on half bridges (9-12)
    drv_write_reg(PWM_FREQ_CTRL_1,  0xFF);  // Set all 4 PWM channels to 2kHz (max speed)

    reg = reg;

}

/*
 * write 8Bits at same time
 * channel: 0..7  (0 => HB1, 7 => HB8)
 * state: 0 => drive LOW (enable LS), 1 => drive HIGH (enable HS)
 * Operation Control 1 (OP_CTRL_1) Register (Address = 0x08) [reset = 0x00]
 * HB4_HS_EN	HB4_LS_EN 	HB3_HS_EN	HB3_LS_EN	HB2_HS_EN	HB2_LS_EN	HB1_HS_EN	HB1_LS_EN
 *
 * Operation Control 2 (OP_CTRL_2) Register (Address = 0x09) [reset = 0x00]
 * HB8_HS_EN	HB8_LS_EN	HB7_HS_EN	HB7_LS_EN	HB6_HS_EN	HB6_LS_EN	HB5_HS_EN	HB5_LS_EN
 * Example
 * all output low
 * OP_CTRL_1 = 0b01010101 & OP_CTRL_2 = 0b01010101
 *
 * output 0 and 7 high
 * OP_CTRL_1 = 0b01010110 & OP_CTRL_2 = 0b10010101
 *
 *
 * */
void DRV8908_write_bulk_output_register(uint8_t state) {
	uint8_t regVal1 = 0b01010101; // all half bridges to low side enable as default
	uint8_t regVal2 = 0b01010101;
	uint8_t s = state;

	// set low nibble half bridges according to LSB state-nibble
	for (uint_fast8_t i = 0; i < 4; i++) {
		if (s & (1U << i)) {
			// set the HS bit
			regVal1 |= 1 << (2 * i + 1);
			// clear the LS bit
			regVal1 &= ~(1 << 2 * i);
		}
	}

	// write register to drv8908
	drv_write_reg(REG_OP_CTRL_1, regVal1);

	// Divide the 'state' register in 4 Bit low nibble and 4 bit high nibble
	s = s >> 4;

	// set high nibble half bridges according to MSB state-nibble
	for (uint_fast8_t i = 0; i < 4; i++) {
		if (s & (1U << i)) {
			// set the HS bit
			regVal2 |= 1 << (2 * i + 1);
			// clear the LS bit
			regVal2 &= ~(1 << 2 * i);
		}
	}

	// write register to drv8908
	drv_write_reg(REG_OP_CTRL_2, regVal2);

	// option read back status register
	regVal1 = drv_read_reg(REG_OP_CTRL_2);
	regVal1 = drv_read_reg(REG_OP_CTRL_2);

}

/* Write 1Bit and while preserve the other 7 Bits
 * channel: 0..7  (0 => HB1, 7 => HB8)
 * state: 0 => drive LOW (enable LS), 1 => drive HIGH (enable HS)
*/
void DRV8908_set_output(uint8_t channel, uint8_t state)
{
    uint8_t regAddr;
    uint8_t idx;
    uint8_t bitLS, bitHS;
    uint8_t regVal;

    if (channel > 7U) { return; }

    if (channel < 4U) { regAddr = REG_OP_CTRL_1; idx = channel; }
    else              { regAddr = REG_OP_CTRL_2; idx = (uint8_t)(channel - 4U); }

    /* OP_CTRL registers layout (per datasheet):
       bit positions per channel: [ ... , HBn_HS_EN, HBn_LS_EN, ... ]
       for channel i (0..3) within the register:
         LS bit   = (1 << (i*2))
         HS bit   = (1 << (i*2 + 1))
    */
    bitLS = (uint8_t)(1U << (idx * 2U));
    bitHS = (uint8_t)(1U << (idx * 2U + 1U));

    regVal = drv_read_reg(regAddr);

    /* clear both bits first */
    regVal &= (uint8_t)~(uint8_t)(bitLS | bitHS);

    if (state != 0U)
    {
        regVal |= bitHS; /* enable high-side -> output HIGH */
    }
    else
    {
        regVal |= bitLS; /* enable low-side  -> output LOW */
    }

    drv_write_reg(regAddr, regVal);
}

void drv8908_demo(void){
	DRV8908_set_output(0, 1);
	HAL_Delay(500);
	DRV8908_set_output(1, 1);
	HAL_Delay(500);
	DRV8908_set_output(2, 1);
	HAL_Delay(500);
	DRV8908_set_output(3, 1);
	HAL_Delay(500);
	DRV8908_set_output(4, 1);
	HAL_Delay(500);
	DRV8908_set_output(5, 1);
	HAL_Delay(500);
	DRV8908_set_output(6, 1);
	HAL_Delay(500);
	DRV8908_set_output(7, 1);
	HAL_Delay(500);

	DRV8908_set_output(0, 0);
	HAL_Delay(500);
	DRV8908_set_output(1, 0);
	HAL_Delay(500);
	DRV8908_set_output(2, 0);
	HAL_Delay(500);
	DRV8908_set_output(3, 0);
	HAL_Delay(500);
	DRV8908_set_output(4, 0);
	HAL_Delay(500);
	DRV8908_set_output(5, 0);
	HAL_Delay(500);
	DRV8908_set_output(6, 0);
	HAL_Delay(500);
	DRV8908_set_output(7, 0);
	HAL_Delay(500);
}
