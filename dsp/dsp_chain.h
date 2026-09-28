#ifndef DSP_CHAIN_H
#define DSP_CHAIN_H

#include "dsp_types.h"
#include "dsp_biquad.h"
#include "dsp_gate.h"
#include "dsp_limiter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    biquad_t hpf;
    biquad_t lpf1;
    biquad_t lpf2;
    dsp_gate_t gate;
    dsp_limiter_t limiter;
    dsp_real_t peak_in;
    dsp_real_t peak_out;
    int sample_rate;
} dsp_chain_t;

void dsp_init(dsp_chain_t *c, int sample_rate);
void dsp_reset(dsp_chain_t *c);
void dsp_process_block(dsp_chain_t *c,
                       const dsp_real_t *in,
                       dsp_real_t *out,
                       int n);

/* Optional: process one sample through full chain */
dsp_real_t dsp_process_sample(dsp_chain_t *c, dsp_real_t x);

#ifdef __cplusplus
}
#endif

#endif /* DSP_CHAIN_H */
