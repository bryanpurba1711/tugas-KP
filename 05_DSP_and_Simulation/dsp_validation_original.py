#!/usr/bin/env python3
# =====================================================================
# Validasi algoritma DSP (RMS, peak, crest factor, dominant frequency,
# band energy) TANPA hardware — pakai sinyal getaran sintetis.
#
# Logika di sini SENGAJA ditulis mengikuti struktur yang sama dengan
# sub_edge_esp32/src/dsp_processing.cpp (remove DC -> windowing Hamming
# -> FFT -> ekstraksi fitur), supaya kamu bisa membandingkan angka yang
# dihasilkan di sini dengan angka yang nanti muncul di Serial Monitor
# ESP32 (baik lewat Wokwi maupun hardware asli).
#
# Cara pakai:
#   pip install numpy matplotlib
#   python dsp_validation.py
# =====================================================================

import numpy as np
import matplotlib.pyplot as plt

# ---- Parameter, samakan dengan sub_edge_esp32/include/config.h ----
N_SAMPLES = 1024
FS_HZ = 26700.0
FFT_BAND_COUNT = 6


def generate_synthetic_vibration(fs, n, base_freq_hz=150.0, harmonics=(1, 2, 3),
                                  noise_std=0.02, impact_every_n=None, seed=42):
    """
    Membuat sinyal getaran buatan yang meniru motor industri:
    - base_freq_hz: frekuensi putaran dasar motor (mis. ~150 Hz -> motor ~9000rpm/pole,
      sesuaikan dengan spesifikasi motor nyata yang mau dipantau)
    - harmonics: motor sehat biasanya punya energi di harmonik ke-1,2,3 dari base_freq
    - noise_std: noise sensor + getaran lingkungan
    - impact_every_n: jika diisi, tambahkan "impact" periodik (mensimulasikan
      cacat bearing) setiap n sampel -> menaikkan crest factor
    """
    rng = np.random.default_rng(seed)
    t = np.arange(n) / fs
    signal = np.zeros(n)

    for h in harmonics:
        amplitude = 1.0 / h  # harmonik lebih tinggi biasanya lebih lemah
        signal += amplitude * np.sin(2 * np.pi * base_freq_hz * h * t)

    signal += rng.normal(0, noise_std, n)

    if impact_every_n:
        for i in range(0, n, impact_every_n):
            width = 5
            if i + width < n:
                signal[i:i + width] += np.hanning(width) * 3.0  # lonjakan impact

    return signal


def compute_axis_features(x, fs, band_count=FFT_BAND_COUNT):
    """Mengikuti struktur AxisFeatures di firmware ESP32."""
    n = len(x)

    # ---- Time-domain features ----
    rms = np.sqrt(np.mean(x ** 2))
    peak = np.max(np.abs(x))
    crest_factor = peak / rms if rms > 1e-9 else 0.0

    # ---- Frequency-domain features ----
    x_dc_removed = x - np.mean(x)
    window = np.hamming(n)
    x_windowed = x_dc_removed * window

    spectrum = np.abs(np.fft.rfft(x_windowed))  # magnitude spectrum, setengah pertama
    freqs = np.fft.rfftfreq(n, d=1.0 / fs)

    # Dominant frequency, skip bin 0 (DC)
    dom_bin = np.argmax(spectrum[1:]) + 1
    dominant_freq_hz = freqs[dom_bin]

    # Band energy: bagi spektrum jadi band_count pita linear-spaced
    half = len(spectrum)
    bins_per_band = half // band_count
    band_energy = []
    for b in range(band_count):
        start = b * bins_per_band + (1 if b == 0 else 0)  # skip DC di band pertama
        end = half if b == band_count - 1 else start + bins_per_band
        energy = np.sqrt(np.sum(spectrum[start:end] ** 2))
        band_energy.append(energy)

    return {
        "rms": rms,
        "peak": peak,
        "crest_factor": crest_factor,
        "dominant_freq_hz": dominant_freq_hz,
        "band_energy": band_energy,
    }, freqs, spectrum


def main():
    print(f"Parameter: N_SAMPLES={N_SAMPLES}, FS_HZ={FS_HZ}\n")

    scenarios = {
        "Motor sehat (150 Hz)": generate_synthetic_vibration(
            FS_HZ, N_SAMPLES, base_freq_hz=150.0, noise_std=0.02),
        "Motor dengan indikasi cacat bearing (impact periodik)": generate_synthetic_vibration(
            FS_HZ, N_SAMPLES, base_freq_hz=150.0, noise_std=0.02, impact_every_n=180),
    }

    fig, axes = plt.subplots(len(scenarios), 2, figsize=(11, 4 * len(scenarios)))

    for row, (label, sig) in enumerate(scenarios.items()):
        feat, freqs, spectrum = compute_axis_features(sig, FS_HZ)

        print(f"--- {label} ---")
        print(f"  RMS            : {feat['rms']:.4f} g")
        print(f"  Peak           : {feat['peak']:.4f} g")
        print(f"  Crest factor   : {feat['crest_factor']:.2f}  "
              f"({'indikasi impact/cacat' if feat['crest_factor'] > 4 else 'normal'})")
        print(f"  Dominant freq  : {feat['dominant_freq_hz']:.1f} Hz")
        print(f"  Band energy    : {[round(b, 3) for b in feat['band_energy']]}")
        print()

        t = np.arange(N_SAMPLES) / FS_HZ
        axes[row, 0].plot(t[:500], sig[:500])
        axes[row, 0].set_title(f"{label} — time domain")
        axes[row, 0].set_xlabel("waktu (s)")
        axes[row, 0].set_ylabel("amplitudo (g)")

        axes[row, 1].plot(freqs[:len(freqs)//4], spectrum[:len(spectrum)//4])
        axes[row, 1].set_title(f"{label} — spektrum FFT")
        axes[row, 1].set_xlabel("frekuensi (Hz)")
        axes[row, 1].set_ylabel("magnitude")

    plt.tight_layout()
    plt.savefig("dsp_validation_output.png", dpi=120)
    print("Plot disimpan ke dsp_validation_output.png")


if __name__ == "__main__":
    main()
