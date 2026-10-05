/**
 * @file usb_audio_rx.h
 * @brief Ring buffer for PCM coming from USB Audio Class (PC → board).
 *
 * Expected host format (USB Audio Speaker):
 *   48 kHz, 16-bit, mono or stereo (stereo → mono mix)
 *
 * Wire STCube-generated usbd_audio_if callbacks to usb_audio_rx_push().
 */
#ifndef USB_AUDIO_RX_H
#define USB_AUDIO_RX_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ~21 ms @ 48 kHz mono float — enough for USB jitter */
#define USB_AUDIO_RING_SAMPLES  1024

void     usb_audio_rx_init(void);
void     usb_audio_rx_reset(void);

/** Called from USB ISR / Audio OUT callback with interleaved int16 PCM. */
void     usb_audio_rx_push_i16(const int16_t *samples, size_t n_samples, int channels);

/** Pop up to n floats in [-1,1]. Returns number of samples written (silence-pad if short). */
size_t   usb_audio_rx_pop_f32(float *dst, size_t n);

int      usb_audio_rx_available(void);
int      usb_audio_rx_streaming(void); /* 1 after first PLAY / data */

extern volatile uint32_t g_usb_audio_underruns;
extern volatile uint32_t g_usb_audio_overruns;
extern volatile uint32_t g_usb_audio_bytes;

#ifdef __cplusplus
}
#endif

#endif /* USB_AUDIO_RX_H */
