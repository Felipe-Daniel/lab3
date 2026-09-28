#ifndef WAV_H
#define WAV_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sample_rate;
    int channels;
    int32_t n_frames;
    float *data; /* interleaved float -1..1, owned by caller after wav_read */
} wav_t;

int wav_read(const char *path, wav_t *w);
int wav_write(const char *path, const float *data, int32_t n_frames,
              int sample_rate, int channels);
void wav_free(wav_t *w);

#ifdef __cplusplus
}
#endif

#endif /* WAV_H */
