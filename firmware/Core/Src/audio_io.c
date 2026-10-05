/**
 * @file audio_io.c
 * @brief USB Audio (PC) / sine → DSP → DAC1 (TIM6 @ 48 kHz DMA)
 *
 * ADC path removed from default .ioc (entrada = USB).
 */
#include "audio_io.h"
#include "main.h"
#include "dsp_chain.h"
#include "usb_audio_rx.h"

#include <math.h>
#include <string.h>

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;

#ifdef AUDIO_ENABLE_ADC
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
static uint16_t adc_dma[AUDIO_DMA_ADC_LEN];
static volatile uint8_t adc_half_ready;
static volatile uint8_t adc_full_ready;
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static uint16_t dac_dma[AUDIO_DMA_AUDIO_LEN];

static volatile uint8_t dac_half_ready;
static volatile uint8_t dac_full_ready;
static volatile uint8_t busy;

static audio_mode_t mode;
static dsp_chain_t chain;
static float sine_phase;
static float sine_delta;
static int use_usb;

volatile float    g_audio_peak_in;
volatile float    g_audio_peak_out;
volatile float    g_audio_gain_reduction;
volatile uint32_t g_audio_overruns;
volatile int      g_audio_gate_open;

static void overrun_led(int on)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint16_t float_to_dac(float x)
{
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    int v = (int)((x + 1.0f) * 2047.5f);
    if (v < 0) v = 0;
    if (v > 4095) v = 4095;
    return (uint16_t)v;
}

static void fill_input(float *in_blk)
{
    int i;

    if (mode == AUDIO_MODE_SINE) {
        for (i = 0; i < AUDIO_BLOCK; ++i) {
            in_blk[i] = 0.5f * sinf(sine_phase);
            sine_phase += sine_delta;
            if (sine_phase > 2.0f * M_PI) {
                sine_phase -= 2.0f * M_PI;
            }
        }
        return;
    }

    if (use_usb) {
        usb_audio_rx_pop_f32(in_blk, (size_t)AUDIO_BLOCK);
        return;
    }

    memset(in_blk, 0, sizeof(float) * (size_t)AUDIO_BLOCK);
}

static void run_dsp_or_copy(const float *in_blk, float *out_blk)
{
    int i;
    int do_dsp = (mode == AUDIO_MODE_USB_DSP || mode == AUDIO_MODE_SINE);

    if (!do_dsp) {
        memcpy(out_blk, in_blk, sizeof(float) * (size_t)AUDIO_BLOCK);
        return;
    }

    {
        dsp_real_t din[AUDIO_BLOCK];
        dsp_real_t dout[AUDIO_BLOCK];
        for (i = 0; i < AUDIO_BLOCK; ++i) {
            din[i] = (dsp_real_t)in_blk[i];
        }
        dsp_process_block(&chain, din, dout, AUDIO_BLOCK);
        for (i = 0; i < AUDIO_BLOCK; ++i) {
            out_blk[i] = (float)dout[i];
        }
        g_audio_peak_in = (float)chain.peak_in;
        g_audio_peak_out = (float)chain.peak_out;
        g_audio_gain_reduction = (float)chain.limiter.gain_reduction;
        g_audio_gate_open = chain.gate.open;
    }
}

static void process_half(uint16_t *dac_dst)
{
    float in_blk[AUDIO_BLOCK];
    float out_blk[AUDIO_BLOCK];
    int i;

    fill_input(in_blk);
    run_dsp_or_copy(in_blk, out_blk);
    for (i = 0; i < AUDIO_BLOCK; ++i) {
        dac_dst[i] = float_to_dac(out_blk[i]);
    }
}

void audio_io_init(audio_mode_t m)
{
    mode = m;
    use_usb = (m == AUDIO_MODE_USB_LOOPTHROUGH || m == AUDIO_MODE_USB_DSP);

    dac_half_ready = 0;
    dac_full_ready = 0;
    busy = 0;
    g_audio_overruns = 0;
    g_audio_peak_in = 0;
    g_audio_peak_out = 0;
    g_audio_gain_reduction = 0;
    g_audio_gate_open = 0;
    sine_phase = 0.0f;
    sine_delta = 2.0f * M_PI * 80.0f / (float)AUDIO_FS_HZ;

    memset(dac_dma, 0, sizeof(dac_dma));
    usb_audio_rx_init();
    dsp_init(&chain, AUDIO_FS_HZ);
    overrun_led(0);
}

void audio_io_start(void)
{
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1,
                      (uint32_t *)dac_dma, AUDIO_DMA_AUDIO_LEN, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);
}

void audio_io_poll(void)
{
    if (dac_half_ready) {
        dac_half_ready = 0;
        if (busy) {
            g_audio_overruns++;
            overrun_led(1);
        }
        busy = 1;
        process_half(&dac_dma[0]);
        busy = 0;
    }
    if (dac_full_ready) {
        dac_full_ready = 0;
        if (busy) {
            g_audio_overruns++;
            overrun_led(1);
        }
        busy = 1;
        process_half(&dac_dma[AUDIO_DMA_AUDIO_LEN / 2]);
        busy = 0;
    }
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac_cb)
{
    (void)hdac_cb;
    dac_half_ready = 1;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac_cb)
{
    (void)hdac_cb;
    dac_full_ready = 1;
}
