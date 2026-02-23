#include "dsp.h"


void IIR_Low_Pass_init(IIR_State *s) {
    s->x1 = s->x2 = s->y1 = s->y2 = 0;
}

int16_t IIR_Low_Pass_update(IIR_State *s, int16_t input) {
    // Koeffizienten (2^14 skaliert) für 10Hz/1Hz
    const int32_t b0 = 1105, b1 = 2210, b2 = 1105;
    const int32_t a1 = -18727, a2 = 6763;

    int32_t accumulator = b0 * input + b1 * s->x1 + b2 * s->x2
                          - a1 * s->y1 - a2 * s->y2;

    int32_t output = accumulator >> 14;

    s->x2 = s->x1;
    s->x1 = input;
    s->y2 = s->y1;
    s->y1 = output;

    return (int16_t)output;
}


/**
 * y(n) = x(n)*a + y(n-1)*(1-a)
 */

float exponential_avaraging_float(float new_value) {
	static float alpha = 0.4;
	static float last_y = 0;
	float y = new_value * alpha + last_y * (1 - alpha);
	last_y = y;
	return y;
}

/**
 * fixed comma version of float implementation for MCU without FPU
 */
int32_t exponential_avaraging_int32(int32_t new_value) {
	const int32_t alpha_full_scale = 32;
	const int32_t alpha = 8;
	const int32_t alpha_compliment = alpha_full_scale - alpha;
	static int32_t last_y = 0;
	int32_t y = ((alpha * new_value) + (alpha_compliment * last_y))
			/ alpha_full_scale;
	last_y = y;
	return y;
}

/**************************************************************
 WinFilter version 0.8
 http://www.winfilter.20m.com
 akundert@hotmail.com

 Filter type: Low Pass
 Filter model: Bessel
 Filter order: 3
 Sampling Frequency: 100 Hz
 Cut Frequency: 10.000000 Hz
 Coefficents Quantization: 16-bit

 Z domain Zeros
 z = -1.000000 + j 0.000000
 z = -1.000000 + j 0.000000
 z = -1.000000 + j 0.000000

 Z domain Poles
 z = 0.531459 + j -0.000000
 z = 0.556083 + j -0.289524
 z = 0.556083 + j 0.289524
 ***************************************************************/
#define Ntap 15
#define DCgain 131072

int16_t fir_int16(int16_t NewSample) {
	int16_t FIRCoef[Ntap] = { -306, 1, 1032, 3430, 7961, 15266, 23994, 28309,
			23994, 15266, 7961, 3430, 1032, 1, -306 };
	static int16_t x[Ntap]; //input samples
	int32_t y = 0;          //output sample
	int16_t n;
	//shift the old samples
	for (n = Ntap - 1; n > 0; n--) {
		x[n] = x[n - 1];
	}
	//Calculate the new output
	x[0] = NewSample;
	for (n = 0; n < Ntap; n++) {
		y += FIRCoef[n] * x[n];
	}
	return (y / DCgain);
}

/**************************************************************
 WinFilter version 0.8
 http://www.winfilter.20m.com
 akundert@hotmail.com

 Filter type: Low Pass
 Filter model: Bessel
 Filter order: 4
 Sampling Frequency: 100 Hz
 Cut Frequency: 10.000000 Hz
 Coefficents Quantization: float

 Z domain Zeros
 z = -1.000000 + j 0.000000
 z = -1.000000 + j 0.000000
 z = -1.000000 + j 0.000000
 z = -1.000000 + j 0.000000

 Z domain Poles
 z = 0.538506 + j -0.104662
 z = 0.538506 + j 0.104662
 z = 0.570481 + j -0.349073
 z = 0.570481 + j 0.349073
 ***************************************************************/
#define Ntap 15
float fir_float(float NewSample) {
	float FIRCoef[Ntap] = { -0.00302545400812832470, 0.00050546323844514475,
			0.01123575532269642300, 0.03361348520434432500,
			0.07094203156708520100, 0.12122527483689413000,
			0.16996497926519682000, 0.19107692914693272000,
			0.16996497926519682000, 0.12122527483689413000,
			0.07094203156708520100, 0.03361348520434432500,
			0.01123575532269642300, 0.00050546323844514475,
			-0.00302545400812832470 };

	static float x[Ntap]; //input samples
	float y = 0;          //output sample
	int16_t n;
	//shift the old samples
	for (n = Ntap - 1; n > 0; n--) {
		x[n] = x[n - 1];
	}
	//Calculate the new output
	x[0] = NewSample;
	for (n = 0; n < Ntap; n++) {
		y += FIRCoef[n] * x[n];
	}
	return y;
}

void test(void){
}

