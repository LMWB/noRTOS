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

/* Struct-based exponential averaging API — reentrant, supports multiple instances */
typedef struct {
    int32_t last_y;
    int32_t alpha;
    int32_t alpha_complement;
    int32_t full_scale;
} ExpAvg_i32_State;

typedef struct {
    float last_y;
    float alpha;
} ExpAvg_f32_State;

void exp_avg_i32_init(ExpAvg_i32_State *s, int32_t alpha, int32_t full_scale);
int32_t exp_avg_i32_update(ExpAvg_i32_State *s, int32_t new_value);

void exp_avg_f32_init(ExpAvg_f32_State *s, float alpha);
float exp_avg_f32_update(ExpAvg_f32_State *s, float new_value);

#endif /* DSP_H_ */
