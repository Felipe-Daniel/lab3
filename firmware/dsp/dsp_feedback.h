#ifndef DSP_FEEDBACK_H
#define DSP_FEEDBACK_H

#include "dsp_types.h"
#include "dsp_biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_FB_MAX_NOTCHES 4

typedef struct {
    dsp_real_t freq_hz;
    dsp_real_t Q;
    dsp_real_t depth; /* how deep the notch, 0..1 */
    int active;
    biquad_t notch;
} dsp_fb_notch_t;

typedef struct {
    int sample_rate;
    int n_bins;
    dsp_real_t detect_threshold; /* magnitude threshold for howling */
    dsp_real_t min_hz;
    dsp_real_t max_hz;
    dsp_fb_notch_t notches[DSP_FB_MAX_NOTCHES];
    /* Goertzel state for candidate frequencies */
    int n_candidates;
    dsp_real_t candidate_hz[16];
    dsp_real_t goertzel_q1[16];
    dsp_real_t goertzel_q2[16];
    dsp_real_t goertzel_coeff[16];
    int goertzel_count;
    int goertzel_N;
} dsp_feedback_t;

void dsp_feedback_init(dsp_feedback_t *fb, int sample_rate);
void dsp_feedback_reset(dsp_feedback_t *fb);
dsp_real_t dsp_feedback_tick(dsp_feedback_t *fb, dsp_real_t x);

#ifdef __cplusplus
}
#endif

#endif /* DSP_FEEDBACK_H */
