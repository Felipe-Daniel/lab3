# Host tools for DSP simulation (C++17 + shared C99 dsp/)
# Usage from tools/: make all && make test
#
# Windows/MSYS2: prefer UCRT64 native toolchain.
#   Best: open "MSYS2 UCRT64" terminal, then: cd tools && make test
#   Or:   export PATH=/ucrt64/bin:$PATH
#   Avoid /usr/bin/g++ (MSYS) — it fails with: fatal error: cmath: No such file

UCRT_GXX := C:/msys64/ucrt64/bin/g++.exe
UCRT_GCC := C:/msys64/ucrt64/bin/gcc.exe
ifneq ($(wildcard $(UCRT_GXX)),)
  CXX := $(UCRT_GXX)
  CC  := $(UCRT_GCC)
else
  CXX ?= g++
  CC  ?= gcc
endif

CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -I. -I../dsp
CFLAGS   ?= -std=c99 -O2 -Wall -Wextra -I../dsp
LDFLAGS  ?= -lm

DSP_SRCS = ../dsp/dsp_biquad.c ../dsp/dsp_gate.c ../dsp/dsp_limiter.c \
           ../dsp/dsp_chain.c ../dsp/dsp_feedback.c
DSP_OBJS = $(DSP_SRCS:.c=.o)

WAV_OBJ  = wav.o

.PHONY: all clean test gen_coeffs freq_response gen_signal run_chain \
        run_chain_f32 run_chain_f64 compare analyze_snr dirs

all: dirs gen_coeffs.exe freq_response.exe gen_signal.exe \
     run_chain_f32.exe run_chain_f64.exe compare.exe analyze_snr.exe

dirs:
	@mkdir -p out 2>/dev/null || mkdir out 2>nul || true

# --- coefficient generator (no dsp link needed beyond writing header) ---
gen_coeffs.exe: gen_coeffs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

gen_coeffs: gen_coeffs.exe
	./gen_coeffs.exe ../dsp/dsp_coeffs.h

# Needs dsp_coeffs.h already generated
freq_response.exe: freq_response.cpp ../dsp/dsp_coeffs.h
	$(CXX) $(CXXFLAGS) -o $@ freq_response.cpp $(LDFLAGS)

gen_signal.exe: gen_signal.cpp $(WAV_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ gen_signal.cpp $(WAV_OBJ) $(LDFLAGS)

# Float32 build
DSP_OBJS_F32 = dsp_biquad_f32.o dsp_gate_f32.o dsp_limiter_f32.o dsp_chain_f32.o dsp_feedback_f32.o

dsp_%_f32.o: ../dsp/dsp_%.c ../dsp/dsp_coeffs.h
	$(CC) $(CFLAGS) -c -o $@ $<

run_chain_f32.exe: run_chain.cpp $(WAV_OBJ) $(DSP_OBJS_F32)
	$(CXX) $(CXXFLAGS) -o $@ run_chain.cpp $(WAV_OBJ) $(DSP_OBJS_F32) $(LDFLAGS)

# Float64 build
DSP_OBJS_F64 = dsp_biquad_f64.o dsp_gate_f64.o dsp_limiter_f64.o dsp_chain_f64.o dsp_feedback_f64.o

dsp_%_f64.o: ../dsp/dsp_%.c ../dsp/dsp_coeffs.h
	$(CC) $(CFLAGS) -DDSP_REAL_DOUBLE -c -o $@ $<

run_chain_f64.exe: run_chain.cpp $(WAV_OBJ) $(DSP_OBJS_F64)
	$(CXX) $(CXXFLAGS) -DDSP_REAL_DOUBLE -o $@ run_chain.cpp $(WAV_OBJ) $(DSP_OBJS_F64) $(LDFLAGS)

compare.exe: compare.cpp $(WAV_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ compare.cpp $(WAV_OBJ) $(LDFLAGS)

analyze_snr.exe: analyze_snr.cpp fft.hpp $(DSP_OBJS_F32) ../dsp/dsp_coeffs.h
	$(CXX) $(CXXFLAGS) -o $@ analyze_snr.cpp $(DSP_OBJS_F32) $(LDFLAGS)

wav.o: wav.c wav.h
	$(CC) $(CFLAGS) -c -o $@ wav.c

freq_response: freq_response.exe
gen_signal: gen_signal.exe
run_chain: run_chain_f32.exe
compare: compare.exe
analyze_snr: analyze_snr.exe

test: all
	@echo "=== gen_coeffs ==="
	./gen_coeffs.exe ../dsp/dsp_coeffs.h
	@echo "=== freq_response ==="
	./freq_response.exe > out/freq_response.csv
	@echo "=== gen_signal ==="
	./gen_signal.exe out
	@echo "=== run_chain f32/f64 ==="
	./run_chain_f32.exe out/tone_80Hz.wav out/out_80_f32.wav
	./run_chain_f64.exe out/tone_80Hz.wav out/out_80_f64.wav
	./run_chain_f32.exe out/tone_1kHz.wav out/out_1k_f32.wav
	./run_chain_f32.exe out/burst_0dBFS.wav out/out_burst_f32.wav
	@echo "=== compare ==="
	./compare.exe out/out_80_f32.wav out/out_80_f64.wav
	@echo "=== analyze_snr ==="
	./analyze_snr.exe > out/snr_report.csv
	@echo "=== ALL TESTS PASSED ==="

clean:
	rm -f *.o *.exe ../dsp/*.o out/*.wav out/*.csv 2>/dev/null || true
	rm -f dsp_*_f32.o dsp_*_f64.o 2>/dev/null || true
