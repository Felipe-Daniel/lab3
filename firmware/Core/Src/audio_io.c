/**
 * @file audio_io.c
 * @brief ADC (TIM2@384kHz DMA) -> average 8x -> DSP/passthrough -> DAC1 (TIM6@48kHz DMA)
 *
 * Requires CubeMX-generated handles:
 *   hadc1, hdac, htim2, htim6
 * Pins: PA1 = ADC1_IN1, PA4 = DAC_OUT1, PD12 = overrun LED
 */
#include "audio_io.h"
#include "main.h"
#include "dsp_chain.h"

#include <math.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Double-buffered DMA regions */
static uint16_t adc_dma[AUDIO_DMA_ADC_LEN];
static uint16_t dac_dma[AUDIO_DMA_AUDIO_LEN];

static volatile uint8_t adc_half_ready;
static volatile uint8_t adc_full_ready;
static volatile uint8_t busy;

static audio_mode_t mode;
static dsp_chain_t chain;
static float sine_phase;
static float sine_delta;

volatile float    g_audio_peak_in;
volatile float    g_audio_peak_out;
volatile float    g_audio_gain_reduction;
volatile uint32_t g_audio_overruns;
volatile int      g_audio_gate_open;

static void overrun_led(int on)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static float adc_to_float(uint16_t code)
{
    /* 12-bit mid-scale bias ~2048 -> AC around 0 */
    return ((float)code - 2048.0f) / 2048.0f;
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

static void process_half(const uint16_t *adc_src, uint16_t *dac_dst)
{
    float in_blk[AUDIO_BLOCK];
    float out_blk[AUDIO_BLOCK];
    int i, k;

    for (i = 0; i < AUDIO_BLOCK; ++i) {
        uint32_t sum = 0;
        for (k = 0; k < AUDIO_OVERSAMPLE; ++k) {
            sum += adc_src[i * AUDIO_OVERSAMPLE + k];
        }
        in_blk[i] = adc_to_float((uint16_t)(sum / AUDIO_OVERSAMPLE));
    }

    if (mode == AUDIO_MODE_SINE) {
        for (i = 0; i < AUDIO_BLOCK; ++i) {
            in_blk[i] = 0.5f * sinf(sine_phase);
            sine_phase += sine_delta;
            if (sine_phase > 2.0f * M_PI) {
                sine_phase -= 2.0f * M_PI;
            }
        }
    }

    if (mode == AUDIO_MODE_LOOPTHROUGH) {
        memcpy(out_blk, in_blk, sizeof(out_blk));
    } else {
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

    for (i = 0; i < AUDIO_BLOCK; ++i) {
        dac_dst[i] = float_to_dac(out_blk[i]);
    }
}

void audio_io_init(audio_mode_t m)
{
    mode = m;
    adc_half_ready = 0;
    adc_full_ready = 0;
    busy = 0;
    g_audio_overruns = 0;
    g_audio_peak_in = 0;
    g_audio_peak_out = 0;
    g_audio_gain_reduction = 0;
    g_audio_gate_open = 0;
    sine_phase = 0.0f;
    sine_delta = 2.0f * M_PI * 80.0f / (float)AUDIO_FS_HZ;

    memset(adc_dma, 0, sizeof(adc_dma));
    memset(dac_dma, 0, sizeof(dac_dma));

    dsp_init(&chain, AUDIO_FS_HZ);
    overrun_led(0);
}

void audio_io_start(void)
{
    /* DAC first (consumer), then ADC+TIM2 (producer) */
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1,
                      (uint32_t *)dac_dma, AUDIO_DMA_AUDIO_LEN, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma, AUDIO_DMA_ADC_LEN);
    HAL_TIM_Base_Start(&htim2);
}

void audio_io_poll(void)
{
    if (adc_half_ready) {
        adc_half_ready = 0;
        if (busy) {
            g_audio_overruns++;
            overrun_led(1);
        }
        busy = 1;
        process_half(&adc_dma[0], &dac_dma[0]);
        busy = 0;
    }
    if (adc_full_ready) {
        adc_full_ready = 0;
        if (busy) {
            g_audio_overruns++;
            overrun_led(1);
        }
        busy = 1;
        process_half(&adc_dma[AUDIO_DMA_ADC_LEN / 2],
                     &dac_dma[AUDIO_DMA_AUDIO_LEN / 2]);
        busy = 0;
    }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_half_ready = 1;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_full_ready = 1;
    }
}
