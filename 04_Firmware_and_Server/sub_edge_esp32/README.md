# Sub-edge ESP32 — revision baseline 1.0

Firmware sensor node ESP32-WROOM-32D untuk akuisisi IIS3DWB dan MLX90614-BCC, pemrosesan fitur lokal, pengiriman berkala melalui painlessMesh, ACK/retry, lalu deep sleep selama 14.400 detik.

## Perubahan utama
- Interval tidur 4 jam.
- FIFO register IIS3DWB dikoreksi ke `0x3A/0x3B`; batching memakai `FIFO_CTRL3`.
- 8.192 sampel pada 26.667 kHz; DC removal, Hann window, FFT, RMS percepatan, velocity RMS, crest factor, kurtosis, dominant frequency, dan 5 band RMS.
- MLX90614 object/ambient/delta temperature.
- Load-switch sensor dan battery-divider switchable.
- ACK/retry sebelum tidur.

## Hardware wajib
GPIO `PIN_SENSOR_POWER_EN` hanya mengendalikan EN load switch/MOSF; jangan menyuplai sensor langsung dari GPIO. Keluaran HT7833-A 3,3 V masuk ke rail 3V3 board, bukan pin VIN/5V. Arus deep sleep wajib diukur pada assembly final.

## Konfigurasi
Ubah `NODE_ID`, password mesh, rasio divider, dan pin sesuai PCB. Untuk simulasi tambahkan build flag `-D USE_SIMULATED_SENSOR=1`.
