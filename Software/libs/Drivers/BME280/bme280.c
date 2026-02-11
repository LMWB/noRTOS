#include "bme280.h"

typedef struct {
    uint16_t 	dig_T1;
    int16_t 	dig_T2, dig_T3;
#if USE_PRESSURE
    uint16_t	dig_P1;
    int16_t		dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
#endif
    uint8_t 	dig_H1, dig_H3;
    int16_t 	dig_H2, dig_H4, dig_H5;
    int8_t		dig_H6;
} BME280_Calib;

static BME280_Calib cal;
static int32_t t_fine;

DEVICE_STATUS_DEFINITION BME280_Init( void ) {
    uint8_t data[26];

    // 1. T-Daten (und P-Daten falls nötig) auslesen
#if USE_PRESSURE
    HAL_I2C_Mem_Read(&I2C_HANDLER, BME280_ADDR, 0x88, 1, data, 26, 100);
    cal.dig_P1 = (data[7] << 8) | data[6];   cal.dig_P2 = (int16_t)((data[9] << 8) | data[8]);
    cal.dig_P3 = (int16_t)((data[11] << 8) | data[10]); cal.dig_P4 = (int16_t)((data[13] << 8) | data[12]);
    cal.dig_P5 = (int16_t)((data[15] << 8) | data[14]); cal.dig_P6 = (int16_t)((data[17] << 8) | data[16]);
    cal.dig_P7 = (int16_t)((data[19] << 8) | data[18]); cal.dig_P8 = (int16_t)((data[21] << 8) | data[20]);
    cal.dig_P9 = (int16_t)((data[23] << 8) | data[22]);
#else
    HAL_I2C_Mem_Read(hi2c, BME280_ADDR, 0x88, 1, data, 6, 100); // Nur T1-T3
#endif
    cal.dig_T1 = (data[1] << 8) | data[0];
    cal.dig_T2 = (int16_t)((data[3] << 8) | data[2]);
    cal.dig_T3 = (int16_t)((data[5] << 8) | data[4]);

    // 2. H-Daten auslesen (Adresse 0xA1 für H1, 0xE1 für Rest)
    HAL_I2C_Mem_Read(&I2C_HANDLER, BME280_ADDR, 0xA1, 1, &cal.dig_H1, 1, 100);
    HAL_I2C_Mem_Read(&I2C_HANDLER, BME280_ADDR, 0xE1, 1, data, 7, 100);
    cal.dig_H2 = (int16_t)((data[1] << 8) | data[0]);
    cal.dig_H3 = data[2];
    cal.dig_H4 = (int16_t)((data[3] << 4) | (data[4] & 0x0F));
    cal.dig_H5 = (int16_t)((data[5] << 4) | (data[4] >> 4));
    cal.dig_H6 = (int8_t)data[6];

    // 3. Konfiguration: Hum x1, Temp x1, Press x1 (oder 0), Mode: Normal
    uint8_t os_p = USE_PRESSURE ? 0x01 : 0x00;
    uint8_t f2 = 0x01; // Hum oversampling x1
    uint8_t f4 = (0x01 << 5) | (os_p << 2) | 0x03; // Temp x1, Press x1/0, Normal Mode

    HAL_I2C_Mem_Write(&I2C_HANDLER, BME280_ADDR, 0xF2, 1, &f2, 1, 100);
    return HAL_I2C_Mem_Write(&I2C_HANDLER, BME280_ADDR, 0xF4, 1, &f4, 1, 100);
}

// --- Kompensation-Logik ---

static int32_t compensate_T(int32_t adc_T) {
    int32_t v1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * ((int32_t)cal.dig_T2)) >> 11;
    int32_t v2 = (((((adc_T >> 4) - ((int32_t)cal.dig_T1)) * ((adc_T >> 4) - ((int32_t)cal.dig_T1))) >> 12) * ((int32_t)cal.dig_T3)) >> 14;
    t_fine = v1 + v2;
    return (t_fine * 5 + 128) >> 8;
}

#if USE_PRESSURE
static uint32_t compensate_P(int32_t adc_P) {
    int64_t v1, v2, p;
    v1 = ((int64_t)t_fine) - 128000;
    v2 = v1 * v1 * (int64_t)cal.dig_P6;
    v2 = v2 + ((v1 * (int64_t)cal.dig_P5) << 17);
    v2 = v2 + (((int64_t)cal.dig_P4) << 35);
    v1 = ((v1 * v1 * (int64_t)cal.dig_P3) >> 8) + ((v1 * (int64_t)cal.dig_P2) << 12);
    v1 = (((((int64_t)1) << 47) + v1)) * ((int64_t)cal.dig_P1) >> 33;
    if (v1 == 0) return 0; // Division durch Null verhindern
    p = 1048576 - adc_P;
    p = (((p << 31) - v2) * 3125) / v1;
    v1 = (((int64_t)cal.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    v2 = (((int64_t)cal.dig_P8) * p) >> 19;
    p = ((p + v1 + v2) >> 8) + (((int64_t)cal.dig_P7) << 4);
    //return (uint32_t)p; // Ergebnis in Pa
    // Wir schieben um 8 Bit nach rechts, um das Q24.8 Format zu normalisieren
    return (uint32_t)(p >> 8);
}
#endif

static uint32_t compensate_H(int32_t adc_H) {
    int32_t v = (t_fine - 76800);
    v = (((((adc_H << 14) - (((int32_t)cal.dig_H4) << 20) - (((int32_t)cal.dig_H5) * v)) + 16384) >> 15) *
         (((((((v * (int32_t)cal.dig_H6) >> 10) * (((v * (int32_t)cal.dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
           (int32_t)cal.dig_H2 + 8192) >> 14));
    v = (v - (((((v >> 15) * (v >> 15)) >> 7) * (int32_t)cal.dig_H1) >> 4));
    v = (v < 0 ? 0 : (v > 419430400 ? 419430400 : v));
    return (uint32_t)((v >> 12) * 10 / 1024);
}

/**
 * Lineare Annäherung der Höhe (reicht bis ca. 2000m völlig aus)
 * 1 hPa Abfall entspricht ca. 8.5 Meter Steigung.
 */
int16_t BME280_GetAltitudeLinear(uint32_t press_pa, uint32_t sea_level_pa) {
    // Differenz in Pascal zu hPa umrechnen (/ 100)
    int32_t diff_hpa = (int32_t)sea_level_pa - (int32_t)press_pa;
    diff_hpa /= 100;

    // 8.5m pro hPa -> wir rechnen * 17 und / 2 um 8.5 zu erhalten
    return (int16_t)((diff_hpa * 17) / 2);
}

void BME280_Read( int16_t *temp, int16_t *hum, uint32_t *press) {
    uint8_t data[8];
    DEVICE_STATUS_DEFINITION status = HAL_I2C_Mem_Read(&I2C_HANDLER, BME280_ADDR, 0xF7, 1, data, 8, 100);
    if (status != HAL_OK) return;

    int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    int32_t adc_H = (data[6] << 8) | data[7];

    *temp = (int16_t)compensate_T(adc_T);
    *hum  = (int16_t)compensate_H(adc_H);
#if USE_PRESSURE
    *press = compensate_P(adc_P);
#else
    *press = 0;
#endif
}
