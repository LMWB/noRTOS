#include "dsp.h"

/**
 * y(n) = x(n)*a + y(n-1)*(1-a)
 */

float exponential_avaraging_float(float new_value){
	static float alpha = 0.4;
	static float last_y = 0;
	float y = new_value*alpha + last_y*(1-alpha);
	last_y = y;
	return y;
}

/**
 * fixed comma version of float implementation for MCU without FPU
 */
int32_t exponential_avaraging_int32(int32_t new_value){
	const int32_t alpha_full_scale = 32;
	const int32_t alpha = 8;
	const int32_t alpha_compliment = alpha_full_scale - alpha;
	static int32_t last_y = 0;
	int32_t y = ((alpha * new_value) + (alpha_compliment * last_y)) / alpha_full_scale;
	last_y = y;
	return y;
}



