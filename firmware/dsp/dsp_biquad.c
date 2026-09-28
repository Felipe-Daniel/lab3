#include "dsp_biquad.h"

void biquad_init(biquad_t *f,
                 dsp_real_t b0, dsp_real_t b1, dsp_real_t b2,
                 dsp_real_t a1, dsp_real_t a2)
{
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
    f->a1 = a1;
    f->a2 = a2;
    f->s1 = (dsp_real_t)0;
    f->s2 = (dsp_real_t)0;
}

void biquad_reset(biquad_t *f)
{
    f->s1 = (dsp_real_t)0;
    f->s2 = (dsp_real_t)0;
}

void biquad_process(biquad_t *f, const dsp_real_t *in, dsp_real_t *out, int n)
{
    int i;
    for (i = 0; i < n; ++i) {
        out[i] = biquad_tick(f, in[i]);
    }
}
