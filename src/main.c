/*
 * main.c
 * ------
 * 1. Reads the noisy input signal  (data/input.txt)
 * 2. Reads the FIR coefficients    (data/coeffs.txt)
 * 3. Runs the 3 filters
 * 4. Writes the results to the output/ folder
 * 5. Measures how long the float and Q15 FIR filters take
 *
 * Run it from the project root folder:  ./filter_app
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "filters.h"

#define MAX_SAMPLES  4096
#define MAX_TAPS     128
#define MA_WINDOW    5      /* moving-average window size */
#define TIMING_RUNS  2000   /* repeat filters many times to measure time */

/* Read numbers (one per line) from a text file into an array.
 * Returns how many numbers were read, or -1 on error. */
static int read_file(const char *path, float *buffer, int max_count)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("ERROR: cannot open %s\n", path);
        printf("Did you run: python python/generate_signal.py ?\n");
        return -1;
    }

    int count = 0;
    while (count < max_count && fscanf(fp, "%f", &buffer[count]) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

/* Write an array of floats to a text file, one per line. */
static int write_file(const char *path, const float *buffer, int count)
{
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        printf("ERROR: cannot create %s (does the output/ folder exist?)\n", path);
        return -1;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%.8f\n", buffer[i]);
    }
    fclose(fp);
    return 0;
}

/* Arrays are 'static' so they live in global memory, not on the stack. */
static float   input_f[MAX_SAMPLES];
static float   coeffs_f[MAX_TAPS];
static float   out_ma[MAX_SAMPLES];
static float   out_fir_f[MAX_SAMPLES];

static int16_t input_q[MAX_SAMPLES];
static int16_t coeffs_q[MAX_TAPS];
static int16_t out_fir_q[MAX_SAMPLES];
static float   out_fir_q_as_float[MAX_SAMPLES];

int main(void)
{
    /* ---------- 1. Load data ---------- */
    int n = read_file("data/input.txt", input_f, MAX_SAMPLES);
    if (n <= 0) return 1;

    int taps = read_file("data/coeffs.txt", coeffs_f, MAX_TAPS);
    if (taps <= 0) return 1;

    printf("Loaded %d input samples and %d FIR coefficients\n", n, taps);

    /* ---------- 2. Convert input and coefficients to Q15 ---------- */
    for (int i = 0; i < n; i++)    input_q[i]  = float_to_q15(input_f[i]);
    for (int k = 0; k < taps; k++) coeffs_q[k] = float_to_q15(coeffs_f[k]);

    /* ---------- 3. Run the filters ---------- */
    moving_average_float(input_f, out_ma, n, MA_WINDOW);
    fir_float(input_f, out_fir_f, n, coeffs_f, taps);
    fir_q15(input_q, out_fir_q, n, coeffs_q, taps);

    /* convert Q15 output back to float so we can save and compare it */
    for (int i = 0; i < n; i++) out_fir_q_as_float[i] = q15_to_float(out_fir_q[i]);

    /* ---------- 4. Save results ---------- */
    if (write_file("output/moving_average_c.txt", out_ma, n)        != 0) return 1;
    if (write_file("output/fir_float_c.txt",      out_fir_f, n)     != 0) return 1;
    if (write_file("output/fir_q15_c.txt",   out_fir_q_as_float, n) != 0) return 1;
    printf("Saved results in output/ folder\n");

    /* ---------- 5. Timing: float vs fixed-point ---------- */
    clock_t start, end;

    start = clock();
    for (int r = 0; r < TIMING_RUNS; r++) fir_float(input_f, out_fir_f, n, coeffs_f, taps);
    end = clock();
    double t_float = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC / TIMING_RUNS;

    start = clock();
    for (int r = 0; r < TIMING_RUNS; r++) fir_q15(input_q, out_fir_q, n, coeffs_q, taps);
    end = clock();
    double t_q15 = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC / TIMING_RUNS;

    printf("\nAverage time per run (%d runs):\n", TIMING_RUNS);
    printf("  FIR float : %.4f ms\n", t_float);
    printf("  FIR Q15   : %.4f ms\n", t_q15);
    printf("(On a laptop CPU both are fast. On a small chip without an FPU,\n");
    printf(" the Q15 version is usually much faster.)\n");

    return 0;
}
