# Firmware and local server — revised package

Baseline ini menyinkronkan firmware node, gateway, dan server dengan desain vibration + temperature monitoring. Perubahan penting: siklus 4 jam, ACK/retry, payload schema 1.0, temperatur MLX90614, DSP terkalibrasi secara struktural, register FIFO IIS3DWB yang benar, dan power management sensor.

## Batasan
- Belum dikompilasi pada board fisik dalam paket ini; lakukan PlatformIO build dan hardware-in-loop test.
- `N_SAMPLES=8192` memerlukan verifikasi heap pada ESP32 DevKit aktual.
- Ambang alarm tetap belum dikonfigurasi sampai baseline mesin dan kelas evaluasi ditetapkan.
- Secret harus diganti sebelum deployment.
