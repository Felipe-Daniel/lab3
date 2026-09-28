#include "dsp_feedback.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void notch_design(biquad_t *f, dsp_real_t fc, dsp_real_t Q, dsp_real_t fs)
{
    dsp_real_t w0 = (dsp_real_t)(2.0 * M_PI) * fc / fs;
    dsp_real_t cosw = DSP_COS(w0);
    dsp_real_t sinw = DSP_SIN(w0);
    dsp_real_t alpha = sinw / ((dsp_real_t)2 * Q);
    dsp_real_t a0 = (dsp_real_t)1 + alpha;
    dsp_real_t b0 = (dsp_real_t)1 / a0;
    dsp_real_t b1 = ((dsp_real_t)(-2) * cosw) / a0;
    dsp_real_t b2 = (dsp_real_t)1 / a0;
    dsp_real_t a1 = ((dsp_real_t)(-2) * cosw) / a0;
    dsp_real_t a2 = ((dsp_real_t)1 - alpha) / a0;
    biquad_init(f, b0, b1, b2, a1, a2);
}

void dsp_feedback_init(dsp_feedback_t *fb, int sample_rate)
{
    int i;
    static const float cands[] = {
        250.f, 315.f, 400.f, 500.f, 630.f, 800.f,
        1000.f, 1250.f, 1600.f, 2000.f, 2500.f, 3150.f,
        4000.f, 5000.f, 6300.f, 8000.f
    };

    fb->sample_rate = sample_rate;
    fb->n_bins = 16;
    fb->detect_threshold = (dsp_real_t)0.25;
    fb->min_hz = (dsp_real_t)200;
    fb->max_hz = (dsp_real_t)8000;
    fb->n_candidates = 16;
    fb->goertzel_N = 480; /* 10 ms at 48 kHz */
    fb->goertzel_count = 0;

    for (i = 0; i < DSP_FB_MAX_NOTCHES; ++i) {
        fb->notches[i].active = 0;
        fb->notches[i].freq_hz = (dsp_real_t)0;
        fb->notches[i].Q = (dsp_real_t)10;
        fb->notches[i].depth = (dsp_real_t)0;
        biquad_reset(&fb->notches[i].notch);
    }

    for (i = 0; i < 16; ++i) {
        dsp_real_t k;
        fb->candidate_hz[i] = (dsp_real_t)cands[i];
        k = (dsp_real_t)(fb->goertzel_N) * fb->candidate_hz[i] / (dsp_real_t)sample_rate;
        fb->goertzel_coeff[i] = (dsp_real_t)2 * DSP_COS((dsp_real_t)(2.0 * M_PI) * k / (dsp_real_t)fb->goertzel_N);
        fb->goertzel_q1[i] = (dsp_real_t)0;
        fb->goertzel_q2[i] = (dsp_real_t)0;
    }
}

void dsp_feedback_reset(dsp_feedback_t *fb)
{
    int i;
    fb->goertzel_count = 0;
    for (i = 0; i < 16; ++i) {
        fb->goertzel_q1[i] = (dsp_real_t)0;
        fb->goertzel_q2[i] = (dsp_real_t)0;
    }
    for (i = 0; i < DSP_FB_MAX_NOTCHES; ++i) {
        fb->notches[i].active = 0;
        biquad_reset(&fb->notches[i].notch);
    }
}

static void try_activate_notch(dsp_feedback_t *fb, dsp_real_t freq)
{
    int i;
    int free_slot = -1;

    for (i = 0; i < DSP_FB_MAX_NOTCHES; ++i) {
        if (fb->notches[i].active) {
            dsp_real_t df = DSP_FABS(fb->notches[i].freq_hz - freq);
            if (df < (dsp_real_t)30) {
                return; /* already covered */
            }
        } else if (free_slot < 0) {
            free_slot = i;
        }
    }

    if (free_slot < 0) {
        return;
    }

    fb->notches[free_slot].freq_hz = freq;
    fb->notches[free_slot].Q = (dsp_real_t)8;
    fb->notches[free_slot].active = 1;
    notch_design(&fb->notches[free_slot].notch, freq, fb->notches[free_slot].Q,
                 (dsp_real_t)fb->sample_rate);
}

dsp_real_t dsp_feedback_tick(dsp_feedback_t *fb, dsp_real_t x)
{
    int i;
    dsp_real_t y = x;

    for (i = 0; i < fb->n_candidates; ++i) {
        dsp_real_t q0 = fb->goertzel_coeff[i] * fb->goertzel_q1[i] - fb->goertzel_q2[i] + x;
        fb->goertzel_q2[i] = fb->goertzel_q1[i];
        fb->goertzel_q1[i] = q0;
    }
    fb->goertzel_count++;

    if (fb->goertzel_count >= fb->goertzel_N) {
        dsp_real_t best_mag = (dsp_real_t)0;
        dsp_real_t best_hz = (dsp_real_t)0;
        for (i = 0; i < fb->n_candidates; ++i) {
            dsp_real_t re = fb->goertzel_q1[i] - fb->goertzel_q2[i] * fb->goertzel_coeff[i] / (dsp_real_t)2;
            dsp_real_t im = fb->goertzel_q2[i] * DSP_SIN(
                (dsp_real_t)(2.0 * M_PI) * fb->candidate_hz[i] * (dsp_real_t)fb->goertzel_N /
                ((dsp_real_t)fb->sample_rate * (dsp_real_t)fb->goertzel_N));
            /* Simplified magnitude from Goertzel power */
            dsp_real_t mag = DSP_SQRT(fb->goertzel_q1[i] * fb->goertzel_q1[i] +
                                      fb->goertzel_q2[i] * fb->goertzel_q2[i] -
                                      fb->goertzel_q1[i] * fb->goertzel_q2[i] * fb->goertzel_coeff[i]);
            mag /= (dsp_real_t)fb->goertzel_N;
            (void)re;
            (void)im;
            if (mag > best_mag) {
                best_mag = mag;
                best_hz = fb->candidate_hz[i];
            }
            fb->goertzel_q1[i] = (dsp_real_t)0;
            fb->goertzel_q2[i] = (dsp_real_t)0;
        }
        fb->goertzel_count = 0;
        if (best_mag > fb->detect_threshold &&
            best_hz >= fb->min_hz && best_hz <= fb->max_hz) {
            try_activate_notch(fb, best_hz);
        }
    }

    for (i = 0; i < DSP_FB_MAX_NOTCHES; ++i) {
        if (fb->notches[i].active) {
            y = biquad_tick(&fb->notches[i].notch, y);
        }
    }
    return y;
}
