/**
 * @file usb_audio_rx.c
 */
#include "usb_audio_rx.h"

#include <string.h>

static float    ring[USB_AUDIO_RING_SAMPLES];
static volatile uint32_t rd;
static volatile uint32_t wr;
static volatile int      streaming;

volatile uint32_t g_usb_audio_underruns;
volatile uint32_t g_usb_audio_overruns;
volatile uint32_t g_usb_audio_bytes;

static uint32_t ring_used(void)
{
    uint32_t w = wr;
    uint32_t r = rd;
    if (w >= r) {
        return w - r;
    }
    return USB_AUDIO_RING_SAMPLES - (r - w);
}

void usb_audio_rx_init(void)
{
    usb_audio_rx_reset();
}

void usb_audio_rx_reset(void)
{
    rd = 0;
    wr = 0;
    streaming = 0;
    g_usb_audio_underruns = 0;
    g_usb_audio_overruns = 0;
    g_usb_audio_bytes = 0;
    memset(ring, 0, sizeof(ring));
}

void usb_audio_rx_push_i16(const int16_t *samples, size_t n_samples, int channels)
{
    size_t i;
    if (samples == NULL || n_samples == 0) {
        return;
    }
    if (channels < 1) {
        channels = 1;
    }

    streaming = 1;
    g_usb_audio_bytes += (uint32_t)(n_samples * (size_t)channels * sizeof(int16_t));

    for (i = 0; i < n_samples; ++i) {
        float x;
        if (channels >= 2) {
            int32_t L = samples[i * (size_t)channels + 0];
            int32_t R = samples[i * (size_t)channels + 1];
            x = (float)(L + R) * (0.5f / 32768.0f);
        } else {
            x = (float)samples[i] / 32768.0f;
        }

        uint32_t next = (wr + 1u) % USB_AUDIO_RING_SAMPLES;
        if (next == rd) {
            /* Drop oldest sample to keep latest audio */
            rd = (rd + 1u) % USB_AUDIO_RING_SAMPLES;
            g_usb_audio_overruns++;
        }
        ring[wr] = x;
        wr = next;
    }
}

size_t usb_audio_rx_pop_f32(float *dst, size_t n)
{
    size_t i;
    uint32_t used = ring_used();

    for (i = 0; i < n; ++i) {
        if (used > 0u) {
            dst[i] = ring[rd];
            rd = (rd + 1u) % USB_AUDIO_RING_SAMPLES;
            used--;
        } else {
            dst[i] = 0.0f;
            if (streaming) {
                g_usb_audio_underruns++;
            }
        }
    }
    return n;
}

int usb_audio_rx_available(void)
{
    return (int)ring_used();
}

int usb_audio_rx_streaming(void)
{
    return streaming;
}
