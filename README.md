# Processador Digital de Subgraves — STM32F4-Discovery

Simulação C/C++ no PC + firmware: **PC envia áudio USB em tempo real** → DSP 100 Hz na placa → DAC.

## Fluxo

```
PC (Spotify/VLC/…)  --USB CN5-->  STM32 (dsp/)  --PA4 DAC-->  caixa/fone
```

## Estrutura

```
lab3/
├─ dsp/                      # C99 compartilhado (PC + MCU)
├─ tools/                    # Simulação host (make test)
├─ firmware/
│  ├─ Core/Src/audio_io.c    # USB/sine → DSP → DAC
│  ├─ Core/Src/usb_audio_rx.c
│  ├─ USB_Device/App/        # glue USBD Audio → ring buffer
│  └─ CUBEMX_SETUP.md        # USB Device Speaker + pinout
└─ docs/
```

## Simulação (algoritmo no PC)

```bash
cd tools
make test
```

## Firmware (áudio ao vivo via USB)

1. Abrir `firmware/SubProc.ioc` no CubeIDE.
2. Seguir **[`firmware/CUBEMX_SETUP.md`](firmware/CUBEMX_SETUP.md)** (ativar USB Audio Device).
3. Ligar `usbd_audio_if` → `SubProc_USB_Audio_Receive`.
4. Modo: `AUDIO_MODE_USB_DSP`.
5. Cabo no **USB OTG (CN5)**; no Windows, selecionar a placa como saída de som.

Modos: `SINE` → `USB_LOOPTHROUGH` → `USB_DSP`.

Protoboard (saida DAC): [`docs/figures/protoboard_montagem.png`](docs/figures/protoboard_montagem.png) — guia em [`docs/figures/protoboard_guia.md`](docs/figures/protoboard_guia.md).
