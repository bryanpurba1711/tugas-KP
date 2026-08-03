#include "dsp_processing.h"
#include <cmath>
#include <cstdlib>

namespace {
constexpr float PI_F = 3.14159265358979323846f;
constexpr float G0 = 9.80665f;
}

bool DspProcessor::isPowerOfTwo(uint16_t n) { return n && !(n & (n - 1)); }

void DspProcessor::fft(float *real, float *imag, uint16_t n) {
    for (uint16_t i = 1, j = 0; i < n; ++i) {
        uint16_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            const float tr = real[i]; real[i] = real[j]; real[j] = tr;
            const float ti = imag[i]; imag[i] = imag[j]; imag[j] = ti;
        }
    }
    for (uint16_t len = 2; len <= n; len <<= 1) {
        const float angle = -2.0f * PI_F / static_cast<float>(len);
        const float wLenR = cosf(angle);
        const float wLenI = sinf(angle);
        for (uint16_t i = 0; i < n; i += len) {
            float wr = 1.0f, wi = 0.0f;
            for (uint16_t j = 0; j < len / 2; ++j) {
                const uint16_t u = i + j;
                const uint16_t v = i + j + len / 2;
                const float vr = real[v] * wr - imag[v] * wi;
                const float vi = real[v] * wi + imag[v] * wr;
                const float ur = real[u], ui = imag[u];
                real[u] = ur + vr; imag[u] = ui + vi;
                real[v] = ur - vr; imag[v] = ui - vi;
                const float nextWr = wr * wLenR - wi * wLenI;
                wi = wr * wLenI + wi * wLenR;
                wr = nextWr;
            }
        }
    }
}

AxisFeatures DspProcessor::process(const int16_t *raw, uint16_t n, float fs,
                                   float sensitivityGPerLsb) {
    AxisFeatures out;
    if (!raw || !isPowerOfTwo(n) || n < 16) return out;

    float *real = static_cast<float*>(malloc(sizeof(float) * n));
    float *imag = static_cast<float*>(calloc(n, sizeof(float)));
    if (!real || !imag) {
        free(real); free(imag);
        return out;
    }

    double mean = 0.0;
    for (uint16_t i = 0; i < n; ++i) mean += raw[i] * sensitivityGPerLsb;
    mean /= static_cast<double>(n);

    double sum2 = 0.0, sum4 = 0.0;
    float peak = 0.0f;
    float coherentGain = 0.0f;
    for (uint16_t i = 0; i < n; ++i) {
        const float centered = raw[i] * sensitivityGPerLsb - static_cast<float>(mean);
        const float a = fabsf(centered);
        if (a > peak) peak = a;
        const double sq = static_cast<double>(centered) * centered;
        sum2 += sq; sum4 += sq * sq;
        const float w = 0.5f * (1.0f - cosf(2.0f * PI_F * i / (n - 1))); // Hann
        coherentGain += w;
        real[i] = centered * w;
    }
    coherentGain /= n;
    out.acceleration_rms_g = sqrtf(static_cast<float>(sum2 / n));
    out.peak_g = peak;
    out.crest_factor = out.acceleration_rms_g > 1e-9f ? peak / out.acceleration_rms_g : 0.0f;
    out.kurtosis = sum2 > 1e-18 ? static_cast<float>((n * sum4) / (sum2 * sum2)) : 0.0f;

    fft(real, imag, n);
    const float binHz = fs / n;
    float maxAmplitude = 0.0f;
    double velocitySumSq = 0.0;
    double bandSumSq[FFT_BAND_COUNT] = {0};

    for (uint16_t k = 1; k < n / 2; ++k) {
        const float f = k * binHz;
        const float mag = hypotf(real[k], imag[k]);
        const float peakAmplitudeG = 2.0f * mag / (n * coherentGain);
        const float rmsAmplitudeG = peakAmplitudeG / sqrtf(2.0f);
        if (f >= 10.0f && f <= 6000.0f && peakAmplitudeG > maxAmplitude) {
            maxAmplitude = peakAmplitudeG;
            out.dominant_freq_hz = f;
        }
        for (uint8_t b = 0; b < FFT_BAND_COUNT; ++b) {
            if (f >= FFT_BAND_LOW_HZ[b] && f < FFT_BAND_HIGH_HZ[b]) {
                bandSumSq[b] += static_cast<double>(rmsAmplitudeG) * rmsAmplitudeG;
            }
        }
        if (f >= VELOCITY_LOW_HZ && f <= VELOCITY_HIGH_HZ) {
            const float velocityRmsMmS = (rmsAmplitudeG * G0 / (2.0f * PI_F * f)) * 1000.0f;
            velocitySumSq += static_cast<double>(velocityRmsMmS) * velocityRmsMmS;
        }
    }
    for (uint8_t b = 0; b < FFT_BAND_COUNT; ++b) out.band_rms_g[b] = sqrtf(bandSumSq[b]);
    out.velocity_rms_mm_s = sqrtf(velocitySumSq);
    free(real); free(imag);
    return out;
}
