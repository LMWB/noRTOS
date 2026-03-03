#ifndef DRV8908_H_
#define DRV8908_H_

#include <stdint.h>

void DRV8908_set_output(uint8_t channel, uint8_t state);
void DRV8908_write_bulk_output_register(uint8_t state);

void drv8908_Init(void);

//void drv8908_demo(void);

void drv8908_state_machine(void);

#endif /* DRV8908_H_ */
