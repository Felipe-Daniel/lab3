#include "wav.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    char riff[4];
    uint32_t size;
    char wave[4];
    char fmt_[4];
    uint32_t fmt_size;
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits;
    char data[4];
    uint32_t data_size;
} wav_hdr_t;
#pragma pack(pop)

int wav_read(const char *path, wav_t *w)
{
    FILE *fp;
    wav_hdr_t h;
    int32_t n_samples;
    int16_t *pcm;
    int32_t i;

    memset(w, 0, sizeof(*w));
    fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }
    if (fread(&h, sizeof(h), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    if (memcmp(h.riff, "RIFF", 4) || memcmp(h.wave, "WAVE", 4) ||
        h.format != 1 || h.bits != 16) {
        fclose(fp);
        return -2;
    }
    /* Skip extra fmt bytes if present */
    if (h.fmt_size > 16) {
        fseek(fp, (long)(h.fmt_size - 16), SEEK_CUR);
    }
    /* Find data chunk if header layout unusual — for simplicity assume standard */
    n_samples = (int32_t)(h.data_size / (h.bits / 8));
    w->n_frames = n_samples / h.channels;
    w->sample_rate = (int)h.sample_rate;
    w->channels = (int)h.channels;
    w->data = (float *)malloc((size_t)n_samples * sizeof(float));
    if (!w->data) {
        fclose(fp);
        return -3;
    }
    pcm = (int16_t *)malloc((size_t)h.data_size);
    if (!pcm) {
        free(w->data);
        w->data = NULL;
        fclose(fp);
        return -3;
    }
    if (fread(pcm, 1, h.data_size, fp) != h.data_size) {
        free(pcm);
        free(w->data);
        w->data = NULL;
        fclose(fp);
        return -1;
    }
    fclose(fp);
    for (i = 0; i < n_samples; ++i) {
        w->data[i] = (float)pcm[i] / 32768.0f;
    }
    free(pcm);
    return 0;
}

int wav_write(const char *path, const float *data, int32_t n_frames,
              int sample_rate, int channels)
{
    FILE *fp;
    wav_hdr_t h;
    int32_t n_samples = n_frames * channels;
    int32_t i;
    int16_t s;

    memset(&h, 0, sizeof(h));
    memcpy(h.riff, "RIFF", 4);
    memcpy(h.wave, "WAVE", 4);
    memcpy(h.fmt_, "fmt ", 4);
    memcpy(h.data, "data", 4);
    h.fmt_size = 16;
    h.format = 1;
    h.channels = (uint16_t)channels;
    h.sample_rate = (uint32_t)sample_rate;
    h.bits = 16;
    h.block_align = (uint16_t)(channels * 2);
    h.byte_rate = (uint32_t)(sample_rate * h.block_align);
    h.data_size = (uint32_t)(n_samples * 2);
    h.size = 36 + h.data_size;

    fp = fopen(path, "wb");
    if (!fp) {
        return -1;
    }
    fwrite(&h, sizeof(h), 1, fp);
    for (i = 0; i < n_samples; ++i) {
        float x = data[i];
        if (x > 1.0f) x = 1.0f;
        if (x < -1.0f) x = -1.0f;
        s = (int16_t)(x * 32767.0f);
        fwrite(&s, 2, 1, fp);
    }
    fclose(fp);
    return 0;
}

void wav_free(wav_t *w)
{
    free(w->data);
    w->data = NULL;
    w->n_frames = 0;
}
