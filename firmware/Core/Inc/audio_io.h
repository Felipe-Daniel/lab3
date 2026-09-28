#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Raw ADC rate / audio rate */
#define AUDIO_OVERSAMPLE      8
#define AUDIO_FS_HZ           48000
#define AUDIO_ADC_FS_HZ       (AUDIO_FS_HZ * AUDIO_OVERSAMPLE) /* 384000 */
#define AUDIO_BLOCK           32
#define AUDIO_DMA_AUDIO_LEN   (AUDIO_BLOCK * 2)  /* double buffer in audio samples */
#define AUDIO_DMA_ADC_LEN     (AUDIO_DMA_AUDIO_LEN * AUDIO_OVERSAMPLE)

typedef enum {
    AUDIO_MODE_LOOPTHROUGH = 0, /* ADC averaged -> DAC (no DSP) */
    AUDIO_MODE_DSP         = 1, /* full dsp_chain */
    AUDIO_MODE_SINE        = 2  /* internal 80 Hz sine -> DSP -> DAC (ADC ignored) */
} audio_mode_t;

void audio_io_init(audio_mode_t mode);
void audio_io_start(void);
void audio_io_poll(void); /* call from main loop */

/* Live monitors (for CubeIDE Live Expressions) */
extern volatile float    g_audio_peak_in;
extern volatile float    g_audio_peak_out;
extern volatile float    g_audio_gain_reduction;
extern volatile uint32_t g_audio_overruns;
extern volatile int      g_audio_gate_open;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_IO_H */
