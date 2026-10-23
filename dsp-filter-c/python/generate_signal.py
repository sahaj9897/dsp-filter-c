"""
generate_signal.py
------------------
Step 1 of the project.

Creates:
  data/input.txt   -> a noisy test signal (one sample per line)
  data/clean.txt   -> the same signal without noise (only used for plotting)
  data/coeffs.txt  -> coefficients of a FIR low-pass filter

The signal:
  - a slow 5 Hz sine wave  (the "useful" signal we want to keep)
  - plus a fast 150 Hz sine wave and random noise (what we want to remove)

Run from the project root:   python python/generate_signal.py
"""

import os
import numpy as np

# ---------------- Settings ----------------
FS = 1000          # sampling rate: 1000 samples per second
N = 1000           # number of samples -> 1 second of signal
NUM_TAPS = 31      # length of the FIR filter
CUTOFF_HZ = 40     # low-pass cutoff: keep frequencies below 40 Hz

np.random.seed(0)  # same "random" noise every run

# ---------------- 1. Make the signal ----------------
t = np.arange(N) / FS                                  # time of each sample (seconds)
clean = 0.5 * np.sin(2 * np.pi * 5 * t)                # useful 5 Hz signal
noise = 0.2 * np.sin(2 * np.pi * 150 * t)              # unwanted 150 Hz tone
noise += 0.08 * np.random.randn(N)                     # random noise
noisy = clean + noise

# Keep values inside -1..+1 so they fit in Q15 format
noisy = np.clip(noisy, -0.95, 0.95)

# ---------------- 2. Design the FIR low-pass filter ----------------
# "Windowed sinc" method:
#   - the ideal low-pass filter is a sinc function
#   - we cut it to NUM_TAPS values and smooth the edges with a Hamming window
fc = CUTOFF_HZ / FS                         # cutoff as a fraction of sampling rate
n = np.arange(NUM_TAPS) - (NUM_TAPS - 1) / 2  # -15 ... +15
h = 2 * fc * np.sinc(2 * fc * n)            # ideal low-pass (sinc)
h = h * np.hamming(NUM_TAPS)                # apply window
h = h / np.sum(h)                           # make gain = 1 at 0 Hz

# ---------------- 3. Save to files ----------------
os.makedirs("data", exist_ok=True)
os.makedirs("output", exist_ok=True)
os.makedirs("plots", exist_ok=True)

np.savetxt("data/input.txt", noisy, fmt="%.8f")
np.savetxt("data/clean.txt", clean, fmt="%.8f")
np.savetxt("data/coeffs.txt", h, fmt="%.8f")

print(f"Saved {N} samples to data/input.txt")
print(f"Saved {NUM_TAPS} FIR coefficients to data/coeffs.txt (cutoff = {CUTOFF_HZ} Hz)")
