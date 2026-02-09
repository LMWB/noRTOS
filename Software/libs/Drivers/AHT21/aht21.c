#include "aht21.h"


/*
 * https://www.fredscave.com/sensors/wss-012aht2x-hum-temp.html
 * */

HAL_StatusTypeDef AHT21_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t check_cmd[] = {AHT21_CMD_INIT, 0x08, 0x00};
    HAL_Delay(40); // Einschaltverzögerung laut Datenblatt

    // Kalibrierungsbefehl senden
    //return HAL_I2C_Master_Transmit(hi2c, AHT21_I2C_ADDR, check_cmd, 3, 100);
    return HAL_OK;
}

HAL_StatusTypeDef AHT21_ReadData(I2C_HandleTypeDef *hi2c, int16_t *temp_x10, int16_t *hum_x10) {
    uint8_t trigger_cmd[] = {AHT21_CMD_TRIGGER, 0x33, 0x00};
    uint8_t data[6];

    // 1. Messung triggern
    if (HAL_I2C_Master_Transmit(hi2c, AHT21_I2C_ADDR, trigger_cmd, 3, 100) != HAL_OK) return HAL_ERROR;

    // 2. Warten (Messung dauert ca. 250ms)
    HAL_Delay(250);

    // 3. 6 Bytes lesen (Status + 20-Bit Hum + 20-Bit Temp)
    if (HAL_I2C_Master_Receive(hi2c, AHT21_I2C_ADDR, data, 6, 100) != HAL_OK) return HAL_ERROR;

    // Status prüfen (Bit 7 muss 0 sein -> Messung fertig)
    if ((data[0] & 0x80) != 0) return HAL_BUSY;

    // 4. Daten extrahieren (20-Bit Werte)
    uint32_t raw_hum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t raw_temp = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    // 5. Berechnung in Zehntel-Schritten (Fixed Point)
    // Feuchte: (raw_hum * 100 * 10) / 2^20  => (raw_hum * 125) / 131072
    *hum_x10 = (int16_t)((raw_hum * 125) >> 17);

    // Temperatur: ((raw_temp * 200 * 10) / 2^20) - 500 => ((raw_temp * 125) / 65536) - 500
    *temp_x10 = (int16_t)(((raw_temp * 125) >> 16) - 500);

    return HAL_OK;
}
