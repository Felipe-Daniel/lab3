#include "dsp_gate.h"

void dsp_gate_init(dsp_gate_t *g,
                   dsp_real_t threshold,
                   dsp_real_t hysteresis,
                   dsp_real_t attack_ms,
                   dsp_real_t release_ms,
                   int sample_rate)
{
    dsp_real_t fs = (dsp_real_t)sample_rate;
    g->threshold = threshold;
    g->hysteresis = hysteresis;
    g->attack = (dsp_real_t)1 - DSP_EXP((dsp_real_t)(-1) / (fs * attack_ms / (dsp_real_t)1000));
    g->release = (dsp_real_t)1 - DSP_EXP((dsp_real_t)(-1) / (fs * release_ms / (dsp_real_t)1000));
    g->env = (dsp_real_t)0;
    g->gain = (dsp_real_t)0;
    g->open = 0;
}

dsp_real_t dsp_gate_tick(dsp_gate_t *g, dsp_real_t x)
{
    dsp_real_t ax = DSP_FABS(x);
    dsp_real_t target;

    if (ax > g->env) {
        g->env += g->attack * (ax - g->env);
    } else {
        g->env += g->release * (ax - g->env);
    }

    if (!g->open && g->env >= g->threshold) {
        g->open = 1;
    } else if (g->open && g->env < g->threshold * g->hysteresis) {
        g->open = 0;
    }

    target = g->open ? (dsp_real_t)1 : (dsp_real_t)0;
    if (target > g->gain) {
        g->gain += g->attack * (target - g->gain);
    } else {
        g->gain += g->release * (target - g->gain);
    }

    return x * g->gain;
}
