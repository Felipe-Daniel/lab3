#ifndef DSP_TYPES_H
#define DSP_TYPES_H

#include <math.h>

#ifdef DSP_REAL_DOUBLE
typedef double dsp_real_t;
#define DSP_SIN(x)  sin(x)
#define DSP_COS(x)  cos(x)
#define DSP_FABS(x) fabs(x)
#define DSP_SQRT(x) sqrt(x)
#define DSP_LOG10(x) log10(x)
#define DSP_EXP(x)  exp(x)
#define DSP_POW(x,y) pow((x),(y))
#else
typedef float dsp_real_t;
#define DSP_SIN(x)  sinf(x)
#define DSP_COS(x)  cosf(x)
#define DSP_FABS(x) fabsf(x)
#define DSP_SQRT(x) sqrtf(x)
#define DSP_LOG10(x) log10f(x)
#define DSP_EXP(x)  expf(x)
#define DSP_POW(x,y) powf((x),(y))
#endif

#ifndef DSP_SAMPLE_RATE
#define DSP_SAMPLE_RATE 48000
#endif

#ifndef DSP_BLOCK_SIZE
#define DSP_BLOCK_SIZE 32
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* DSP_TYPES_H */
