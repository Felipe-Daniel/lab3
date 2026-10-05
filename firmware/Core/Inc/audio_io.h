#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_FS_HZ           48000
#define AUDIO_BLOCK           32
#define AUDIO_DMA_AUDIO_LEN   (AUDIO_BLOCK * 2)

typedef enum {
    AUDIO_MODE_SINE = 0,            /* internal 80 Hz → DSP → DAC */
    AUDIO_MODE_USB_LOOPTHROUGH,     /* USB PCM → DAC */
    AUDIO_MODE_USB_DSP              /* USB PCM → dsp_chain → DAC (default) */
} audio_mode_t;

void audio_io_init(audio_mode_t mode);
void audio_io_start(void);
void audio_io_poll(void);

extern volatile float    g_audio_peak_in;
extern volatile float    g_audio_peak_out;
extern volatile float    g_audio_gain_reduction;
extern volatile uint32_t g_audio_overruns;
extern volatile int      g_audio_gate_open;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_IO_H */
