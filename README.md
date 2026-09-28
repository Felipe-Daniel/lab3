# Processador Digital de Subgraves — STM32F4-Discovery

Simulação C/C++ + firmware CubeIDE para crossover digital de 100 Hz.

## Estrutura

```
lab3/
├─ dsp/                 # C99 compartilhado (PC + MCU)
├─ tools/               # Simulação host (make test)
├─ firmware/            # CubeMX SubProc.ioc + audio_io + cópia de dsp/
└─ docs/                # Relatório parcial + pinout
```

## Simulação (PC)

Pré-requisito: `g++` e `make` (MSYS2 UCRT64 ou w64devkit).

```bash
cd tools
make test
```

Artefatos em `tools/out/` (CSV, WAV, log).

## Firmware

1. Abrir `firmware/SubProc.ioc` no STM32CubeIDE / CubeMX.
2. Seguir [`firmware/CUBEMX_SETUP.md`](firmware/CUBEMX_SETUP.md).
3. Modos: `AUDIO_MODE_SINE` → `LOOPTHROUGH` → `DSP`.

## Relatório parcial

[`docs/relatorio_parcial.md`](docs/relatorio_parcial.md)
# lab3
