/**
 * @file main_user.c
 * @brief Drop-in user logic for CubeMX-generated main.c (USB Audio path)
 */
#include "main.h"
#include "audio_io.h"

extern void SystemClock_Config(void);
extern void MX_GPIO_Init(void);
extern void MX_DMA_Init(void);
extern void MX_DAC_Init(void);
extern void MX_TIM6_Init(void);
/* After CubeMX USB generate: extern void MX_USB_DEVICE_Init(void); */

void main_user_run(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_DAC_Init();
    MX_TIM6_Init();
    /* MX_USB_DEVICE_Init(); */

    audio_io_init(AUDIO_MODE_USB_DSP);
    audio_io_start();

    while (1) {
        audio_io_poll();
    }
}
