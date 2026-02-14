#ifndef DSP_H_
#define DSP_H_

#include <stdint.h>

typedef struct {
    int32_t x1, x2;
    int32_t y1, y2;
} IIR_State;

void IIR_Low_Pass_init(IIR_State *s);

int16_t IIR_Low_Pass_update(IIR_State *s, int16_t input);

int32_t exponential_avaraging_int32(int32_t new_value);
float exponential_avaraging_float(float new_value);

int16_t fir_int16(int16_t NewSample);
float fir_float(float NewSample);

void test(void);

#endif /* DSP_H_ */
