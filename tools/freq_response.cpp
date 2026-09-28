/*
 * Evaluate H(z) for HPF * (LPF)^2 on a log frequency grid; CSV to stdout.
 */
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>

#include "../dsp/dsp_coeffs.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using cplx = std::complex<double>;

static cplx biquad_H(double b0, double b1, double b2,
                     double a1, double a2, cplx z)
{
    cplx z1 = 1.0 / z;
    cplx z2 = z1 * z1;
    cplx num = b0 + b1 * z1 + b2 * z2;
    cplx den = 1.0 + a1 * z1 + a2 * z2;
    return num / den;
}

static double unwrap_diff(double prev, double cur)
{
    double d = cur - prev;
    while (d > M_PI) d -= 2.0 * M_PI;
    while (d < -M_PI) d += 2.0 * M_PI;
    return d;
}

int main()
{
    const double fs = DSP_FS_HZ;
    const int n_pts = 200;
    const double f_min = 10.0;
    const double f_max = 20000.0;

    std::printf("freq_hz,mag_db,phase_rad,group_delay_s\n");

    double prev_phase = 0.0;
    double prev_f = f_min;
    int first = 1;
    double mag100 = 0.0;
    double mag200 = 0.0;
    double mag400 = 0.0;

    for (int i = 0; i < n_pts; ++i) {
        double t = static_cast<double>(i) / (n_pts - 1);
        double f = f_min * std::pow(f_max / f_min, t);
        double w = 2.0 * M_PI * f / fs;
        cplx z = std::exp(cplx(0.0, w));

        cplx H = biquad_H(DSP_HPF_B0, DSP_HPF_B1, DSP_HPF_B2,
                          DSP_HPF_A1, DSP_HPF_A2, z);
        cplx Lp = biquad_H(DSP_LPF_B0, DSP_LPF_B1, DSP_LPF_B2,
                           DSP_LPF_A1, DSP_LPF_A2, z);
        H *= Lp * Lp; /* LR4 */

        double mag = std::abs(H);
        double mag_db = 20.0 * std::log10(mag + 1e-30);
        double phase = std::arg(H);
        double gd = 0.0;

        if (!first) {
            double dphi = unwrap_diff(prev_phase, phase);
            double dw = 2.0 * M_PI * (f - prev_f) / fs;
            /* group delay in samples = -dphi/dw ; convert to seconds */
            if (std::fabs(dw) > 1e-18) {
                gd = (-dphi / dw) / fs;
            }
        }

        std::printf("%.6f,%.6f,%.6f,%.9f\n", f, mag_db, phase, gd);

        if (std::fabs(f - 100.0) < std::fabs(prev_f - 100.0) || i == 0) {
            /* track closest — refined below */
        }
        if (f <= 100.0) mag100 = mag_db;
        if (f <= 200.0) mag200 = mag_db;
        if (f <= 400.0) mag400 = mag_db;

        prev_phase = phase;
        prev_f = f;
        first = 0;
    }

    /* Exact evaluation at key frequencies */
    auto eval_db = [&](double f) {
        double w = 2.0 * M_PI * f / fs;
        cplx z = std::exp(cplx(0.0, w));
        cplx H = biquad_H(DSP_HPF_B0, DSP_HPF_B1, DSP_HPF_B2,
                          DSP_HPF_A1, DSP_HPF_A2, z);
        cplx Lp = biquad_H(DSP_LPF_B0, DSP_LPF_B1, DSP_LPF_B2,
                           DSP_LPF_A1, DSP_LPF_A2, z);
        H *= Lp * Lp;
        return 20.0 * std::log10(std::abs(H) + 1e-30);
    };

    mag100 = eval_db(100.0);
    mag200 = eval_db(200.0);
    mag400 = eval_db(400.0);
    double slope = mag400 - mag200; /* should be ~ -24 dB/oct (one octave) */

    std::fprintf(stderr, "CHECK mag@100Hz=%.3f dB (want ~-6)\n", mag100);
    std::fprintf(stderr, "CHECK slope 200->400Hz=%.3f dB/oct (want ~-24)\n", slope);

    if (std::fabs(mag100 + 6.02) > 1.0) {
        std::fprintf(stderr, "FAIL: |mag@100 + 6| too large\n");
        return 1;
    }
    if (std::fabs(slope + 24.0) > 4.0) {
        std::fprintf(stderr, "FAIL: slope not near -24 dB/oct\n");
        return 1;
    }
    std::fprintf(stderr, "freq_response ACCEPT\n");
    return 0;
}
