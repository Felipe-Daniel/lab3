#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "wav.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void write_tone(const char *path, double freq, double amp, double seconds, int fs)
{
    int n = static_cast<int>(seconds * fs);
    std::vector<float> buf(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        buf[static_cast<size_t>(i)] = static_cast<float>(
            amp * std::sin(2.0 * M_PI * freq * i / fs));
    }
    if (wav_write(path, buf.data(), n, fs, 1) != 0) {
        std::perror(path);
        std::exit(1);
    }
    std::printf("wrote %s\n", path);
}

static void write_silence(const char *path, double seconds, int fs)
{
    int n = static_cast<int>(seconds * fs);
    std::vector<float> buf(static_cast<size_t>(n), 0.0f);
    wav_write(path, buf.data(), n, fs, 1);
    std::printf("wrote %s\n", path);
}

static void write_sweep(const char *path, double f0, double f1, double seconds, int fs)
{
    int n = static_cast<int>(seconds * fs);
    std::vector<float> buf(static_cast<size_t>(n));
    double k = std::log(f1 / f0);
    for (int i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / fs;
        double phase = 2.0 * M_PI * f0 * seconds / k * (std::exp(k * t / seconds) - 1.0);
        buf[static_cast<size_t>(i)] = static_cast<float>(0.5 * std::sin(phase));
    }
    wav_write(path, buf.data(), n, fs, 1);
    std::printf("wrote %s\n", path);
}

static void write_burst(const char *path, double freq, double seconds, int fs)
{
    int n = static_cast<int>(seconds * fs);
    std::vector<float> buf(static_cast<size_t>(n), 0.0f);
    int burst = static_cast<int>(0.1 * fs);
    for (int i = 0; i < burst && i < n; ++i) {
        buf[static_cast<size_t>(i)] = static_cast<float>(
            std::sin(2.0 * M_PI * freq * i / fs)); /* 0 dBFS */
    }
    wav_write(path, buf.data(), n, fs, 1);
    std::printf("wrote %s\n", path);
}

int main(int argc, char **argv)
{
    const int fs = 48000;
    const char *outdir = "out";
    if (argc >= 2) {
        outdir = argv[1];
    }

    auto path = [&](const char *name) {
        return std::string(outdir) + "/" + name;
    };

#ifdef _WIN32
    std::string cmd = std::string("mkdir \"") + outdir + "\" 2>nul";
    std::system(cmd.c_str());
#else
    std::string cmd = std::string("mkdir -p \"") + outdir + "\"";
    std::system(cmd.c_str());
#endif

    const double freqs[] = {20, 50, 80, 100, 200, 400, 1000};
    for (double f : freqs) {
        char name[64];
        std::snprintf(name, sizeof(name), "tone_%.0fHz.wav", f);
        write_tone(path(name).c_str(), f, 0.5, 1.0, fs);
    }
    write_silence(path("silence.wav").c_str(), 1.0, fs);
    write_sweep(path("sweep_log.wav").c_str(), 20.0, 20000.0, 2.0, fs);
    write_burst(path("burst_0dBFS.wav").c_str(), 80.0, 1.0, fs);
    write_tone(path("tone_80Hz.wav").c_str(), 80.0, 0.5, 2.0, fs);
    write_tone(path("tone_1kHz.wav").c_str(), 1000.0, 0.5, 2.0, fs);

    return 0;
}
