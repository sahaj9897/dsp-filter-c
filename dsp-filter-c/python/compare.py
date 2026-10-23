"""
compare.py
----------
Step 3 of the project (after running the C program).

1. Loads the outputs written by the C program (output/ folder)
2. Computes the correct outputs using the Python reference model
3. Prints the error between C and Python -> PASS or FAIL
4. Saves plots in the plots/ folder

Run from the project root:   python python/compare.py
"""

import os
import sys
import numpy as np
import matplotlib

matplotlib.use("Agg")            # save plots to files (no window needed)
import matplotlib.pyplot as plt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from reference_model import moving_average, fir_filter

FS = 1000
MA_WINDOW = 5   # must match MA_WINDOW in src/main.c


def snr_db(reference, test):
    """Signal-to-noise ratio of the error, in dB. Higher = more accurate."""
    error = reference - test
    return 10 * np.log10(np.sum(reference**2) / np.sum(error**2))


def check(name, reference, c_output, max_allowed_error):
    """Print max error and SNR, and return True if the test passes."""
    max_err = np.max(np.abs(reference - c_output))
    passed = max_err <= max_allowed_error
    status = "PASS" if passed else "FAIL"
    print(f"{name:<22} max error = {max_err:.2e}   SNR = {snr_db(reference, c_output):6.1f} dB   {status}")
    return passed


# ---------------- 1. Load data ----------------
x = np.loadtxt("data/input.txt")
clean = np.loadtxt("data/clean.txt")
h = np.loadtxt("data/coeffs.txt")

c_ma = np.loadtxt("output/moving_average_c.txt")
c_fir_float = np.loadtxt("output/fir_float_c.txt")
c_fir_q15 = np.loadtxt("output/fir_q15_c.txt")

# ---------------- 2. Python reference outputs ----------------
ref_ma = moving_average(x, MA_WINDOW)
ref_fir = fir_filter(x, h)

# ---------------- 3. Compare C vs Python ----------------
print("\nC output vs Python reference")
print("-" * 70)
ok = True
ok &= check("Moving average (float)", ref_ma, c_ma, 1e-5)
ok &= check("FIR (float)", ref_fir, c_fir_float, 1e-5)
ok &= check("FIR (Q15 fixed-point)", ref_fir, c_fir_q15, 1e-3)
print("-" * 70)
print("ALL TESTS PASSED" if ok else "SOME TESTS FAILED")

# ---------------- 4. Plot time-domain signals ----------------
t = np.arange(len(x)) / FS * 1000   # time in milliseconds
show = slice(0, 400)                # plot first 400 ms so it is easy to see

fig, ax = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

ax[0].plot(t[show], x[show], color="gray", linewidth=0.8, label="Noisy input")
ax[0].plot(t[show], clean[show], color="black", linewidth=1.5, label="Clean signal (5 Hz)")
ax[0].set_title("Input signal")

ax[1].plot(t[show], c_ma[show], color="tab:orange", label="Moving average (C)")
ax[1].plot(t[show], clean[show], "k--", linewidth=1, label="Clean signal")
ax[1].set_title(f"Moving-average filter (window = {MA_WINDOW})")

ax[2].plot(t[show], c_fir_float[show], color="tab:blue", linewidth=2, label="FIR float (C)")
ax[2].plot(t[show], c_fir_q15[show], color="tab:red", linestyle="--", label="FIR Q15 (C)")
ax[2].plot(t[show], ref_fir[show], color="black", linestyle=":", label="Python reference")
ax[2].set_title(f"FIR low-pass filter ({len(h)} taps)")
ax[2].set_xlabel("Time (ms)")

for a in ax:
    a.set_ylabel("Amplitude")
    a.grid(alpha=0.3)
    a.legend(loc="upper right", fontsize=8)

plt.tight_layout()
plt.savefig("plots/filter_output.png", dpi=120)

# ---------------- 5. Plot frequency spectrum (FFT) ----------------
freqs = np.fft.rfftfreq(len(x), 1 / FS)
spec_in = np.abs(np.fft.rfft(x)) / len(x)
spec_out = np.abs(np.fft.rfft(c_fir_q15)) / len(x)

plt.figure(figsize=(10, 4))
plt.plot(freqs, spec_in, color="gray", label="Noisy input")
plt.plot(freqs, spec_out, color="tab:red", label="After FIR filter (Q15)")
plt.title("Frequency spectrum: the 150 Hz noise is removed, 5 Hz signal is kept")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.xlim(0, 250)
plt.grid(alpha=0.3)
plt.legend()
plt.tight_layout()
plt.savefig("plots/spectrum.png", dpi=120)

print("\nSaved plots: plots/filter_output.png and plots/spectrum.png")
