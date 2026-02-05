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

/* channel: 0..7  (0 => HB1, 7 => HB8)
   state: 0 => drive LOW (enable LS), 1 => drive HIGH (enable HS)
*/
void DRV8908_SetOutput(uint8_t channel, uint8_t state)
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
	DRV8908_SetOutput(0, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(1, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(2, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(3, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(4, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(5, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(6, 1);
	HAL_Delay(500);
	DRV8908_SetOutput(7, 1);
	HAL_Delay(500);

	DRV8908_SetOutput(0, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(1, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(2, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(3, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(4, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(5, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(6, 0);
	HAL_Delay(500);
	DRV8908_SetOutput(7, 0);
	HAL_Delay(500);
}
