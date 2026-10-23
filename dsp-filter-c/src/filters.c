/*
 * filters.c
 * ---------
 * Implementation of the filters declared in filters.h.
 */

#include "filters.h"

/* ============================================================
 * Q15 conversion helpers
 * ============================================================ */

/* Convert a float (-1.0 to +1.0) into Q15 format.
 * We multiply by 32768, round to the nearest integer,
 * and clamp ("saturate") so the value fits inside 16 bits. */
int16_t float_to_q15(float x)
{
    float scaled = x * Q15_ONE;

    /* round to nearest integer */
    if (scaled >= 0.0f) scaled += 0.5f;
    else                scaled -= 0.5f;

    /* saturate: 16-bit signed range is -32768 ... 32767 */
    if (scaled > 32767.0f)  return 32767;
    if (scaled < -32768.0f) return -32768;

    return (int16_t)scaled;
}

/* Convert a Q15 number back to float by dividing by 32768. */
float q15_to_float(int16_t x)
{
    return (float)x / Q15_ONE;
}

/* ============================================================
 * 1. Moving-average filter (float)
 * ============================================================
 * output[i] = (input[i] + input[i-1] + ... + input[i-window+1]) / window
 * Samples before the start of the signal are treated as 0.
 */
void moving_average_float(const float *input, float *output, int n, int window)
{
    for (int i = 0; i < n; i++) {
        float sum = 0.0f;

        for (int k = 0; k < window; k++) {
            int idx = i - k;           /* index of an older sample */
            if (idx >= 0) {
                sum += *(input + idx); /* same as input[idx], written with a pointer */
            }
        }
        *(output + i) = sum / window;  /* same as output[i] = ... */
    }
}

/* ============================================================
 * 2. FIR filter (float)
 * ============================================================
 * output[i] = coeffs[0]*input[i] + coeffs[1]*input[i-1] + ...
 * This is called "convolution". The coefficients decide which
 * frequencies pass through (here: a low-pass filter).
 */
void fir_float(const float *input, float *output, int n,
               const float *coeffs, int num_taps)
{
    for (int i = 0; i < n; i++) {
        float acc = 0.0f;   /* accumulator: running sum */

        for (int k = 0; k < num_taps; k++) {
            int idx = i - k;
            if (idx >= 0) {
                acc += coeffs[k] * input[idx];   /* multiply-accumulate (MAC) */
            }
        }
        output[i] = acc;
    }
}

/* ============================================================
 * 3. FIR filter (Q15 fixed-point)
 * ============================================================
 * Same maths as fir_float, but using only integers. This is how
 * filters are often written for small chips that have no fast
 * floating-point hardware (no FPU).
 *
 * Steps:
 *   - Q15 x Q15 gives a Q30 result (a 32-bit number).
 *   - We add up all products in a 64-bit accumulator so it never overflows.
 *   - We shift right by 15 to go back from Q30 to Q15.
 *   - We saturate so the result fits inside 16 bits.
 */
void fir_q15(const int16_t *input, int16_t *output, int n,
             const int16_t *coeffs, int num_taps)
{
    for (int i = 0; i < n; i++) {
        int64_t acc = 0;

        for (int k = 0; k < num_taps; k++) {
            int idx = i - k;
            if (idx >= 0) {
                acc += (int32_t)coeffs[k] * (int32_t)input[idx];  /* Q30 product */
            }
        }

        /* rounding: add half of 2^15 before shifting */
        acc += (1 << 14);

        /* convert Q30 -> Q15 by shifting right by 15 bits */
        acc = acc >> 15;

        /* saturation */
        if (acc > 32767)  acc = 32767;
        if (acc < -32768) acc = -32768;

        output[i] = (int16_t)acc;
    }
}
