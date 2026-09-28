#include "dsp_chain.h"
#include "dsp_coeffs.h"

void dsp_init(dsp_chain_t *c, int sample_rate)
{
    c->sample_rate = sample_rate;
    biquad_init(&c->hpf,
                (dsp_real_t)DSP_HPF_B0, (dsp_real_t)DSP_HPF_B1, (dsp_real_t)DSP_HPF_B2,
                (dsp_real_t)DSP_HPF_A1, (dsp_real_t)DSP_HPF_A2);
    biquad_init(&c->lpf1,
                (dsp_real_t)DSP_LPF_B0, (dsp_real_t)DSP_LPF_B1, (dsp_real_t)DSP_LPF_B2,
                (dsp_real_t)DSP_LPF_A1, (dsp_real_t)DSP_LPF_A2);
    biquad_init(&c->lpf2,
                (dsp_real_t)DSP_LPF_B0, (dsp_real_t)DSP_LPF_B1, (dsp_real_t)DSP_LPF_B2,
                (dsp_real_t)DSP_LPF_A1, (dsp_real_t)DSP_LPF_A2);
    dsp_gate_init(&c->gate,
                  (dsp_real_t)0.01,   /* -40 dBFS approx */
                  (dsp_real_t)0.5,
                  (dsp_real_t)1.0,
                  (dsp_real_t)50.0,
                  sample_rate);
    dsp_limiter_init(&c->limiter,
                     (dsp_real_t)(-1.0),
                     (dsp_real_t)0.5,
                     (dsp_real_t)100.0,
                     sample_rate);
    c->peak_in = (dsp_real_t)0;
    c->peak_out = (dsp_real_t)0;
}

void dsp_reset(dsp_chain_t *c)
{
    biquad_reset(&c->hpf);
    biquad_reset(&c->lpf1);
    biquad_reset(&c->lpf2);
    c->gate.env = (dsp_real_t)0;
    c->gate.gain = (dsp_real_t)0;
    c->gate.open = 0;
    c->limiter.gain = (dsp_real_t)1;
    c->limiter.gain_reduction = (dsp_real_t)0;
    c->peak_in = (dsp_real_t)0;
    c->peak_out = (dsp_real_t)0;
}

dsp_real_t dsp_process_sample(dsp_chain_t *c, dsp_real_t x)
{
    dsp_real_t y;
    dsp_real_t ax = DSP_FABS(x);

    if (ax > c->peak_in) {
        c->peak_in = ax;
    }

    y = biquad_tick(&c->hpf, x);
    y = dsp_gate_tick(&c->gate, y);
    y = biquad_tick(&c->lpf1, y);
    y = biquad_tick(&c->lpf2, y);
    y = dsp_limiter_tick(&c->limiter, y);

    /* Hard clamp safety net */
    if (y > (dsp_real_t)1) {
        y = (dsp_real_t)1;
    } else if (y < (dsp_real_t)(-1)) {
        y = (dsp_real_t)(-1);
    }

    ax = DSP_FABS(y);
    if (ax > c->peak_out) {
        c->peak_out = ax;
    }
    return y;
}

void dsp_process_block(dsp_chain_t *c,
                       const dsp_real_t *in,
                       dsp_real_t *out,
                       int n)
{
    int i;
    for (i = 0; i < n; ++i) {
        out[i] = dsp_process_sample(c, in[i]);
    }
}
