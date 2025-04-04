#ifndef DSP_H_
#define DSP_H_

#include <stdint.h>

int32_t exponential_avaraging_int32(int32_t new_value);
float exponential_avaraging_float(float new_value);

int16_t fir_int16(int16_t NewSample);
float fir_float(int16_t NewSample);


#endif /* DSP_H_ */
