"""
reference_model.py
------------------
The "correct answer" for each filter, written in Python with NumPy.

In real firmware teams, an algorithm is first written in Python or MATLAB
(the reference model). The C code is then checked against it.
"""

import numpy as np


def moving_average(x, window):
    """Average of the last `window` samples (older samples before start = 0)."""
    kernel = np.ones(window) / window
    return np.convolve(x, kernel)[: len(x)]


def fir_filter(x, coeffs):
    """FIR filter: y[i] = sum over k of coeffs[k] * x[i - k]."""
    return np.convolve(x, coeffs)[: len(x)]
