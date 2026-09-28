# Configuração STM32CubeMX — SubProc (STM32F4-Discovery)

Placa: **STM32F4-Discovery** · MCU: **STM32F407VGT6** · Projeto: [`SubProc.ioc`](../firmware/SubProc.ioc)

## Como abrir / gerar

1. Instalar **STM32CubeIDE** (inclui CubeMX).
2. File → Open Projects from File System → selecionar a pasta `firmware/` **ou**
   File → New → STM32 Project from an existing STM32CubeMX configuration file → `SubProc.ioc`.
3. Se o `.ioc` pedir migração de pacote FW_F4, aceite a versão sugerida.
4. Clique em **Generate Code**.
5. Adicione ao projeto:
   - `Core/Src/audio_io.c`, `Core/Src/main_user.c` (opcional)
   - todos os `.c` de `firmware/dsp/`
   - Include paths: `Core/Inc`, `dsp`
6. Em Project Properties → C/C++ Build → Settings → MCU Settings:
   - Floating-point ABI: **FPv4-SP-D16 Hard**
   - Optimization: `-O2`
7. Em `main.c`, nos blocos USER CODE:

```c
/* USER CODE BEGIN Includes */
#include "audio_io.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
audio_io_init(AUDIO_MODE_SINE); /* depois: LOOPTHROUGH, DSP */
audio_io_start();
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1) {
  audio_io_poll();
/* USER CODE END WHILE */
```

## Checklist de periféricos (para screenshots do relatório)

### Clock (HSE 8 MHz → 168 MHz)
| Parâmetro | Valor |
|-----------|-------|
| HSE | 8 MHz (cristal da Discovery) |
| PLLM / PLLN / PLLP | 8 / 336 / 2 |
| SYSCLK | 168 MHz |
| AHB | 168 MHz |
| APB1 | 42 MHz (timers ×2 = 84 MHz) |
| APB2 | 84 MHz |
| ADCCLK | PCLK2/4 = **21 MHz** (≤ 36 MHz) |

### TIM2 → trigger ADC @ 384 kHz
- Clock source: Internal
- Prescaler: 0
- Counter Period (ARR): **218** → f = 84e6/(218+1) ≈ **383,6 kHz**
- TRGO: Update Event

### ADC1
- Canal: **IN1 (PA1)** — **não usar PA0** (botão USER)
- Resolução: 12 bits
- Continuous: Disable
- External trigger: Timer 2 TRGO, rising
- DMA Continuous Requests: Enable
- Sampling time: 15 cycles
- DMA2 Stream0: Periph→Memory, Circular, Half-word, High priority

### TIM6 → trigger DAC @ 48 kHz
- ARR: **1749** → 84e6/(1749+1) = **48 000 Hz** exato
- TRGO: Update Event

### DAC1
- Out1 → **PA4**
- Trigger: Timer 6 TRGO
- DMA1 Stream5: Memory→Periph, Circular, Half-word

### GPIO
- **PD12**: output (LED verde Discovery) = indicador de overrun
- SWD: PA13/PA14 (já da Discovery)

## Diagrama de pinos

```
Celular (P2) --[front-end protoboard]--> PA1 (ADC1_IN1)
PA4 (DAC_OUT1) --[RC ~1–2 kHz]--> caixa ativa / fone
PD12 LED = overrun
ST-Link USB (CN1) = programação/debug
```

## Modos de áudio (`audio_io_init`)

| Modo | Uso |
|------|-----|
| `AUDIO_MODE_SINE` | Valida DSP sem front-end |
| `AUDIO_MODE_LOOPTHROUGH` | Valida ADC↔DAC |
| `AUDIO_MODE_DSP` | Cadeia completa (HPF+gate+LR4+limiter) |

## Live Expressions sugeridas

`g_audio_peak_in`, `g_audio_peak_out`, `g_audio_gain_reduction`, `g_audio_overruns`, `g_audio_gate_open`

## Nota sobre o `.ioc`

O arquivo `SubProc.ioc` é um ponto de partida alinhado a estes parâmetros. Se o CubeMX reclamar de algum campo ao abrir, reconfigure só o item listado acima — a tabela deste documento é a referência do relatório parcial.
