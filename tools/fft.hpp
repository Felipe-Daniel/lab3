#pragma once

#include <complex>
#include <cmath>
#include <vector>
#include <cstddef>

/* In-place Cooley–Tukey radix-2 FFT. n must be a power of 2. */
inline void fft_radix2(std::vector<std::complex<double>> &a, bool inverse = false)
{
    const std::size_t n = a.size();
    std::size_t j = 0;
    for (std::size_t i = 1; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    for (std::size_t len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * 3.14159265358979323846 / static_cast<double>(len) * (inverse ? 1.0 : -1.0);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                std::complex<double> u = a[i + k];
                std::complex<double> v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse) {
        for (auto &x : a) {
            x /= static_cast<double>(n);
        }
    }
}

inline std::size_t next_pow2(std::size_t n)
{
    std::size_t p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}
