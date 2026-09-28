#ifndef DSP_BIQUAD_H
#define DSP_BIQUAD_H

#include "dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_real_t b0, b1, b2;
    dsp_real_t a1, a2;
    dsp_real_t s1, s2;
} biquad_t;

void biquad_init(biquad_t *f,
                 dsp_real_t b0, dsp_real_t b1, dsp_real_t b2,
                 dsp_real_t a1, dsp_real_t a2);

void biquad_reset(biquad_t *f);

static inline dsp_real_t biquad_tick(biquad_t *f, dsp_real_t x)
{
    dsp_real_t y = f->b0 * x + f->s1;
    f->s1 = f->b1 * x - f->a1 * y + f->s2;
    f->s2 = f->b2 * x - f->a2 * y;
    return y;
}

void biquad_process(biquad_t *f, const dsp_real_t *in, dsp_real_t *out, int n);

#ifdef __cplusplus
}
#endif

#endif /* DSP_BIQUAD_H */
