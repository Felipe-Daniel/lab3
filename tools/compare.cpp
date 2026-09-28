#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "wav.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s a.wav b.wav\n", argv[0]);
        return 1;
    }

    wav_t a, b;
    if (wav_read(argv[1], &a) != 0 || wav_read(argv[2], &b) != 0) {
        std::fprintf(stderr, "read failed\n");
        return 1;
    }
    if (a.n_frames != b.n_frames || a.channels != b.channels) {
        std::fprintf(stderr, "length/channel mismatch\n");
        return 1;
    }

    double max_err = 0.0;
    double sum_err2 = 0.0;
    double sum_ref2 = 0.0;
    int32_t n = a.n_frames * a.channels;

    for (int32_t i = 0; i < n; ++i) {
        double e = static_cast<double>(a.data[i] - b.data[i]);
        double r = static_cast<double>(b.data[i]);
        double ae = std::fabs(e);
        if (ae > max_err) max_err = ae;
        sum_err2 += e * e;
        sum_ref2 += r * r;
    }

    double snr = 10.0 * std::log10((sum_ref2 + 1e-30) / (sum_err2 + 1e-30));
    std::printf("max_abs_err=%.6e\n", max_err);
    std::printf("snr_db=%.3f\n", snr);

    /* Adequacy vs ADC: error must stay below ~1 LSB of 12-bit FS, and SNR >> 12-bit floor.
       WAV round-trip is 16-bit, so absolute error floors near ~3e-5 even for identical signals. */
    const double lsb12 = 1.0 / 2048.0;
    if (max_err > lsb12 || snr < 60.0) {
        std::fprintf(stderr, "FAIL: max_err %.3e (lim %.3e) or snr %.1f dB (lim 60)\n",
                     max_err, lsb12, snr);
        wav_free(&a);
        wav_free(&b);
        return 1;
    }

    std::printf("compare ACCEPT (err << 12-bit LSB, snr=%.1f dB)\n", snr);
    wav_free(&a);
    wav_free(&b);
    return 0;
}
