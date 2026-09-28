/*
 * Quantization / SNR analysis for ADC12 -> avg8 -> DSP -> DAC12 chain.
 * Reports SNR and THD in the 0..100 Hz band for an 80 Hz test tone.
 */
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "fft.hpp"
#include "../dsp/dsp_chain.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double quantize_adc(double x)
{
    double u = (x + 1.0) * 0.5;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    int code = static_cast<int>(u * 4095.0 + 0.5);
    if (code < 0) code = 0;
    if (code > 4095) code = 4095;
    return (static_cast<double>(code) / 4095.0) * 2.0 - 1.0;
}

static double quantize_dac(double x)
{
    double u = (x + 1.0) * 0.5;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    int code = static_cast<int>(u * 4095.0 + 0.5);
    if (code < 0) code = 0;
    if (code > 4095) code = 4095;
    return (static_cast<double>(code) / 4095.0) * 2.0 - 1.0;
}

int main()
{
    const int fs = 48000;
    const int overs = 8;
    const int fs_raw = fs * overs;
    const int n_fft = 8192;
    const int n_warm = fs;          /* 1 s settle for IIR + gate */
    const int n_audio = n_warm + n_fft;
    const int n_raw = n_audio * overs;
    const double tone_hz = 80.0;
    const double amp = 0.5;

    std::vector<double> raw(static_cast<size_t>(n_raw));
    for (int i = 0; i < n_raw; ++i) {
        double t = static_cast<double>(i) / fs_raw;
        double x = amp * std::sin(2.0 * M_PI * tone_hz * t);
        raw[static_cast<size_t>(i)] = quantize_adc(x);
    }

    std::vector<float> audio(static_cast<size_t>(n_audio));
    for (int i = 0; i < n_audio; ++i) {
        double sum = 0.0;
        for (int k = 0; k < overs; ++k) {
            sum += raw[static_cast<size_t>(i * overs + k)];
        }
        audio[static_cast<size_t>(i)] = static_cast<float>(sum / overs);
    }

    dsp_chain_t chain;
    dsp_init(&chain, fs);
    /* Force gate open so measurement is not gated */
    chain.gate.open = 1;
    chain.gate.gain = 1.0f;

    std::vector<float> out(static_cast<size_t>(n_audio));
    dsp_real_t in_blk[DSP_BLOCK_SIZE];
    dsp_real_t out_blk[DSP_BLOCK_SIZE];
    for (int i = 0; i < n_audio; ) {
        int n = DSP_BLOCK_SIZE;
        if (i + n > n_audio) n = n_audio - i;
        for (int k = 0; k < n; ++k) {
            in_blk[k] = static_cast<dsp_real_t>(audio[static_cast<size_t>(i + k)]);
        }
        dsp_process_block(&chain, in_blk, out_blk, n);
        for (int k = 0; k < n; ++k) {
            out[static_cast<size_t>(i + k)] =
                static_cast<float>(quantize_dac(static_cast<double>(out_blk[k])));
        }
        i += n;
    }

    /* Steady-state window */
    std::vector<std::complex<double>> X(static_cast<size_t>(n_fft));
    for (int i = 0; i < n_fft; ++i) {
        double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * i / (n_fft - 1));
        float s = out[static_cast<size_t>(n_warm + i)];
        X[static_cast<size_t>(i)] = std::complex<double>(static_cast<double>(s) * w, 0.0);
    }
    fft_radix2(X, false);

    int tone_bin = static_cast<int>(std::lround(tone_hz * n_fft / fs));
    const int exclude = 8; /* Hann main lobe */
    double signal_pow = 0.0;
    for (int k = tone_bin - exclude; k <= tone_bin + exclude; ++k) {
        if (k > 0 && k < n_fft / 2) {
            signal_pow += std::norm(X[static_cast<size_t>(k)]);
        }
    }

    double noise_pow = 0.0;
    double harm_pow = 0.0;
    int max_bin_100 = static_cast<int>(100.0 * n_fft / fs);

    for (int k = 1; k <= max_bin_100; ++k) {
        if (k >= tone_bin - exclude && k <= tone_bin + exclude) {
            continue;
        }
        noise_pow += std::norm(X[static_cast<size_t>(k)]);
    }

    for (int h = 2; h <= 5; ++h) {
        int hb = static_cast<int>(std::lround(tone_hz * h * n_fft / fs));
        if (hb > 0 && hb < n_fft / 2) {
            harm_pow += std::norm(X[static_cast<size_t>(hb)]);
        }
    }

    double snr_db = 10.0 * std::log10((signal_pow + 1e-30) / (noise_pow + 1e-30));
    double thd_db = 10.0 * std::log10((harm_pow + 1e-30) / (signal_pow + 1e-30));
    double noise_floor = 10.0 * std::log10(noise_pow / (max_bin_100 + 1e-30) + 1e-30);

    std::printf("metric,value\n");
    std::printf("tone_hz,%.1f\n", tone_hz);
    std::printf("snr_band_0_100_hz_db,%.3f\n", snr_db);
    std::printf("thd_db,%.3f\n", thd_db);
    std::printf("noise_floor_db,%.3f\n", noise_floor);
    std::printf("signal_bins_power,%.6e\n", signal_pow);
    std::printf("noise_band_power,%.6e\n", noise_pow);

    std::fprintf(stderr, "SNR(0-100Hz)=%.2f dB  THD=%.2f dB\n", snr_db, thd_db);

    if (snr_db < 50.0) {
        std::fprintf(stderr, "FAIL: SNR too low\n");
        return 1;
    }
    std::fprintf(stderr, "analyze_snr ACCEPT\n");
    return 0;
}
