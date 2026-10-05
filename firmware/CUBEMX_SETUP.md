# Configuração STM32CubeMX — SubProc (USB Audio → DSP → DAC)

Placa: **STM32F4-Discovery** · MCU: **STM32F407VGT6** · Projeto: [`SubProc.ioc`](SubProc.ioc)

## Arquitetura (adaptada)

```
PC (player) --USB OTG FS (CN5)--> STM32 USB Audio Device
                                      │
                                      ▼
                              usb_audio_rx (ring)
                                      │
                              DSP 100 Hz (dsp/)
                                      │
                                      ▼
                              DAC PA4 @ 48 kHz → RC → caixa/fone
```

ST-Link (USB mini/micro de **debug**) continua só para gravar/depurar.  
Áudio ao vivo usa o conector **USB OTG (CN5)** com cabo até o PC.

Modo padrão de aplicação: `AUDIO_MODE_USB_DSP`.

---

## Como abrir / gerar

1. Instalar **STM32CubeIDE** (inclui CubeMX).
2. Abrir `firmware/SubProc.ioc`.
3. Se pedir migração FW_F4, aceite.
4. **Acrescentar USB Device Audio** (passos abaixo) → **Generate Code**.
5. No build, incluir:
   - `Core/Src/audio_io.c`, `Core/Src/usb_audio_rx.c`
   - `USB_Device/App/usbd_audio_if.c` (glue) **e** o gerado pelo Cube (merge)
   - todos os `.c` de `firmware/dsp/`
   - Include: `Core/Inc`, `dsp`, `USB_Device/App`
6. MCU Settings: FPU **Hard**, `-O2`.

### main.c (USER CODE)

```c
/* USER CODE BEGIN Includes */
#include "audio_io.h"
#include "usb_audio_rx.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
audio_io_init(AUDIO_MODE_USB_DSP);
audio_io_start();
/* MX_USB_DEVICE_Init() já é chamado pelo Cube antes/depois — mantenha a ordem
 * gerada: init USB Device, depois audio_io_start se preferir. */
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1) {
  audio_io_poll();
/* USER CODE END WHILE */
```

---

## CubeMX — USB Device (Speaker) @ 48 kHz

Já existe no `.ioc`: clock 168 MHz com **PLLQ = 48 MHz** (necessário para USB).

### Middleware

1. **Connectivity → USB_OTG_FS** → Mode: **Device_Only**  
   - Pinos: **PA11** (DM), **PA12** (DP) — CN5 da Discovery.
2. **Middleware → USB_DEVICE**  
   - Class: **Audio Device Class**  
   - AUDIO_DEVICE → **Speaker** (OUT do host = IN na placa)  
   - Frequency: **48000**  
   - Channels: **2** (stereo; o firmware mistura para mono no DSP)  
   - Bit depth: **16-bit**
3. **NVIC**: habilitar `OTG_FS_IRQn` (prioridade alta, ex. 5).
4. Em `stm32f4xx_hal_conf.h` (após generate): `HAL_PCD_MODULE_ENABLED`.

### Ligar PCM ao ring buffer

No `usbd_audio_if.c` **gerado**, nos hooks `AUDIO_AudioCmd_FS` / `AUDIO_PeriodicTC_FS`, chame:

```c
#include "usbd_audio_if.h" /* pasta USB_Device/App deste repo */

SubProc_USB_Audio_Receive(pbuf, size);   /* PLAY / dados */
SubProc_USB_Audio_Stop();                /* STOP */
```

Há um template comentado em [`USB_Device/App/usbd_audio_if.c`](USB_Device/App/usbd_audio_if.c).

---

## Checklist periféricos restantes (já no `.ioc`)

### Clock
| Parâmetro | Valor |
|-----------|-------|
| HSE | 8 MHz |
| SYSCLK | 168 MHz |
| APB1 / timers | 42 / 84 MHz |
| PLLQ | **48 MHz** (USB) |

### TIM6 → DAC @ 48 kHz
- ARR **1749** → 84e6/(1749+1) = 48 kHz  
- DAC Out1 **PA4**, DMA1 Stream5 circular  

### GPIO
- **PD12** LED verde = overrun (DSP/USB)  
- Live: `g_usb_audio_underruns`, `g_usb_audio_overruns`, `g_usb_audio_bytes`

---

## Modos (`audio_io_init`)

| Modo | Uso |
|------|-----|
| `AUDIO_MODE_SINE` | Valida DSP sem USB |
| `AUDIO_MODE_USB_LOOPTHROUGH` | USB → DAC (sem filtro) |
| `AUDIO_MODE_USB_DSP` | **USB → LPF 100 Hz → DAC** |

Ordem sugerida: SINE → USB_LOOPTHROUGH → USB_DSP.

---

## No PC (Windows)

1. Grave o firmware; conecte **CN5** ao PC (além do ST-Link se for depurar).
2. Windows deve listar um dispositivo de **áudio USB** (nome do descriptor Cube).
3. Em *Configurações → Sistema → Som*, escolha essa saída.
4. Toque qualquer música/player — o stream vai para o DSP na placa.
5. Ouça em PA4 (RC ~1–2 kHz) / caixa ativa.

Se não aparecer placa de som: confira Device_Only, PLLQ 48 MHz, cabo em **CN5** (não só o ST-Link).

---

## Diagrama de pinos

```
PC USB  ----CN5 (OTG FS)----> USB Audio Device stack
                                  |
                                  v
                            usb_audio_rx ring
                                  |
                            dsp_chain (100 Hz)
                                  |
PA4 (DAC_OUT1) --[RC]--> caixa / fone
PD12 LED = overrun
ST-Link USB = só programação/debug
```
