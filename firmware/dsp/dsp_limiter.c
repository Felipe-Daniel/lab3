#include "dsp_limiter.h"

void dsp_limiter_init(dsp_limiter_t *l,
                      dsp_real_t ceiling_dbfs,
                      dsp_real_t attack_ms,
                      dsp_real_t release_ms,
                      int sample_rate)
{
    dsp_real_t fs = (dsp_real_t)sample_rate;
    l->ceiling = DSP_POW((dsp_real_t)10, ceiling_dbfs / (dsp_real_t)20);
    l->attack = (dsp_real_t)1 - DSP_EXP((dsp_real_t)(-1) / (fs * attack_ms / (dsp_real_t)1000));
    l->release = (dsp_real_t)1 - DSP_EXP((dsp_real_t)(-1) / (fs * release_ms / (dsp_real_t)1000));
    l->gain = (dsp_real_t)1;
    l->gain_reduction = (dsp_real_t)0;
}

dsp_real_t dsp_limiter_tick(dsp_limiter_t *l, dsp_real_t x)
{
    dsp_real_t ax = DSP_FABS(x);
    dsp_real_t needed = (dsp_real_t)1;
    dsp_real_t y;

    if (ax * l->gain > l->ceiling && ax > (dsp_real_t)1e-12) {
        needed = l->ceiling / ax;
    }

    if (needed < l->gain) {
        l->gain += l->attack * (needed - l->gain);
    } else {
        l->gain += l->release * ((dsp_real_t)1 - l->gain);
    }

    y = x * l->gain;
    if (y > l->ceiling) {
        y = l->ceiling;
    } else if (y < -l->ceiling) {
        y = -l->ceiling;
    }

    l->gain_reduction = (dsp_real_t)1 - l->gain;
    return y;
}
