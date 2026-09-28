#ifndef DSP_GATE_H
#define DSP_GATE_H

#include "dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_real_t threshold;   /* open threshold (linear, e.g. 0.01) */
    dsp_real_t hysteresis;  /* close below threshold * hysteresis */
    dsp_real_t attack;      /* gain slew toward 1 when open */
    dsp_real_t release;     /* gain slew toward 0 when closed */
    dsp_real_t env;         /* envelope follower */
    dsp_real_t gain;        /* current gate gain 0..1 */
    int open;
} dsp_gate_t;

void dsp_gate_init(dsp_gate_t *g,
                   dsp_real_t threshold,
                   dsp_real_t hysteresis,
                   dsp_real_t attack_ms,
                   dsp_real_t release_ms,
                   int sample_rate);

dsp_real_t dsp_gate_tick(dsp_gate_t *g, dsp_real_t x);

#ifdef __cplusplus
}
#endif

#endif /* DSP_GATE_H */
