#ifndef DRV8908_H_
#define DRV8908_H_

#include <stdint.h>

void DRV8908_SetOutput(uint8_t channel, uint8_t state);
void DRV8908_Set_Bulk_Output(uint8_t state);

void drv8908_Init(void);

void drv8908_demo(void);

#endif /* DRV8908_H_ */
