#ifndef DSP_LIMITER_H
#define DSP_LIMITER_H

#include "dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dsp_real_t ceiling;       /* max abs output, e.g. 0.891 (-1 dBFS) */
    dsp_real_t attack;        /* coeff toward lower gain */
    dsp_real_t release;       /* coeff toward gain=1 */
    dsp_real_t gain;
    dsp_real_t gain_reduction; /* 1 - gain, for monitoring */
} dsp_limiter_t;

void dsp_limiter_init(dsp_limiter_t *l,
                      dsp_real_t ceiling_dbfs,
                      dsp_real_t attack_ms,
                      dsp_real_t release_ms,
                      int sample_rate);

dsp_real_t dsp_limiter_tick(dsp_limiter_t *l, dsp_real_t x);

#ifdef __cplusplus
}
#endif

#endif /* DSP_LIMITER_H */
