/**
 * @file main_user.c
 * @brief Drop-in user logic for CubeMX-generated main.c
 *
 * After CubeMX generates the project:
 *  1. Add audio_io.c to the build
 *  2. Add ../dsp (or firmware/dsp) sources + include path
 *  3. In main.c USER CODE BEGIN 2 / WHILE call the functions below
 *     — or replace the generated main() body with main_user_run().
 */
#include "main.h"
#include "audio_io.h"

extern void SystemClock_Config(void);
extern void MX_GPIO_Init(void);
extern void MX_DMA_Init(void);
extern void MX_ADC1_Init(void);
extern void MX_DAC_Init(void);
extern void MX_TIM2_Init(void);
extern void MX_TIM6_Init(void);

void main_user_run(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_DAC_Init();
    MX_TIM2_Init();
    MX_TIM6_Init();

    /* Sprint validation order:
     *   1) AUDIO_MODE_SINE        — DSP only, no analog
     *   2) AUDIO_MODE_LOOPTHROUGH — ADC/DAC path
     *   3) AUDIO_MODE_DSP         — full chain
     */
    audio_io_init(AUDIO_MODE_SINE);
    audio_io_start();

    while (1) {
        audio_io_poll();
    }
}

/*
 * Minimal snippets to paste into CubeMX main.c:
 *
 * USER CODE BEGIN Includes
 * #include "audio_io.h"
 * USER CODE END Includes
 *
 * USER CODE BEGIN 2
 * audio_io_init(AUDIO_MODE_DSP);
 * audio_io_start();
 * USER CODE END 2
 *
 * USER CODE BEGIN WHILE
 * while (1) {
 *   audio_io_poll();
 * USER CODE END WHILE
 */
