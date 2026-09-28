#include <cstdio>
#include <cstdlib>
#include <vector>

#include "wav.h"
#include "../dsp/dsp_chain.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s in.wav out.wav\n", argv[0]);
        return 1;
    }

    wav_t w;
    if (wav_read(argv[1], &w) != 0) {
        std::fprintf(stderr, "failed to read %s\n", argv[1]);
        return 1;
    }

    /* Mixdown to mono if needed */
    std::vector<float> mono(static_cast<size_t>(w.n_frames));
    if (w.channels == 1) {
        for (int32_t i = 0; i < w.n_frames; ++i) {
            mono[static_cast<size_t>(i)] = w.data[i];
        }
    } else {
        for (int32_t i = 0; i < w.n_frames; ++i) {
            float s = 0.0f;
            for (int c = 0; c < w.channels; ++c) {
                s += w.data[i * w.channels + c];
            }
            mono[static_cast<size_t>(i)] = s / static_cast<float>(w.channels);
        }
    }

    dsp_chain_t chain;
    dsp_init(&chain, w.sample_rate > 0 ? w.sample_rate : DSP_SAMPLE_RATE);

    std::vector<float> out(mono.size());
    const int block = DSP_BLOCK_SIZE;
    dsp_real_t in_blk[DSP_BLOCK_SIZE];
    dsp_real_t out_blk[DSP_BLOCK_SIZE];

    for (size_t i = 0; i < mono.size(); ) {
        int n = block;
        if (i + static_cast<size_t>(n) > mono.size()) {
            n = static_cast<int>(mono.size() - i);
        }
        for (int k = 0; k < n; ++k) {
            in_blk[k] = static_cast<dsp_real_t>(mono[i + static_cast<size_t>(k)]);
        }
        dsp_process_block(&chain, in_blk, out_blk, n);
        for (int k = 0; k < n; ++k) {
            out[i + static_cast<size_t>(k)] = static_cast<float>(out_blk[k]);
        }
        i += static_cast<size_t>(n);
    }

    if (wav_write(argv[2], out.data(), static_cast<int32_t>(out.size()),
                  w.sample_rate, 1) != 0) {
        std::fprintf(stderr, "failed to write %s\n", argv[2]);
        wav_free(&w);
        return 1;
    }

    std::printf("processed %ld frames -> %s (peak_in=%.4f peak_out=%.4f gr=%.4f)\n",
                static_cast<long>(w.n_frames), argv[2],
                static_cast<double>(chain.peak_in),
                static_cast<double>(chain.peak_out),
                static_cast<double>(chain.limiter.gain_reduction));

    wav_free(&w);
    return 0;
}
