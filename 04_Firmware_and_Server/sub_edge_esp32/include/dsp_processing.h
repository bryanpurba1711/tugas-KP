#pragma once
#include <Arduino.h>
#include "config.h"

struct AxisFeatures {
    float acceleration_rms_g = 0.0f;
    float peak_g = 0.0f;
    float crest_factor = 0.0f;
    float kurtosis = 0.0f;
    float dominant_freq_hz = 0.0f;
    float velocity_rms_mm_s = 0.0f;
    float band_rms_g[FFT_BAND_COUNT] = {0};
};

class DspProcessor {
public:
    AxisFeatures process(const int16_t *raw, uint16_t n, float fs,
                         float sensitivityGPerLsb);
private:
    static bool isPowerOfTwo(uint16_t n);
    static void fft(float *real, float *imag, uint16_t n);
};
