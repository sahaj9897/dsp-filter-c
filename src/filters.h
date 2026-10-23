/*
 * filters.h
 * ---------
 * Declarations of all the filter functions in this library.
 *
 * Three filters are provided:
 *   1. Moving-average filter   (float)
 *   2. FIR low-pass filter     (float)
 *   3. FIR low-pass filter     (Q15 fixed-point)
 */

#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>   /* for int16_t, int32_t, int64_t */

/* ---------- Q15 helpers ----------
 * Q15 stores a decimal number between -1.0 and +1.0 inside a 16-bit integer.
 *   real value = integer value / 32768
 * Example: 0.5 is stored as 16384, -0.25 is stored as -8192.
 */
#define Q15_ONE 32768

int16_t float_to_q15(float x);
float   q15_to_float(int16_t x);

/* ---------- Filters ----------
 * All filters:
 *   input  : pointer to the input samples
 *   output : pointer to where the filtered samples are written
 *   n      : number of samples
 */

/* Moving average: each output = average of the last 'window' inputs. */
void moving_average_float(const float *input, float *output, int n, int window);

/* FIR filter (float): output[i] = sum over k of coeffs[k] * input[i - k] */
void fir_float(const float *input, float *output, int n,
               const float *coeffs, int num_taps);

/* FIR filter (Q15 fixed-point): same maths, but only integer operations. */
void fir_q15(const int16_t *input, int16_t *output, int n,
             const int16_t *coeffs, int num_taps);

#endif /* FILTERS_H */
