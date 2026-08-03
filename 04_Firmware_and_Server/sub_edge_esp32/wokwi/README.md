# Cara mensimulasikan sub-edge node TANPA hardware sama sekali

## Opsi A — Wokwi lewat browser (paling gampang, tidak perlu install apapun)

1. Build dulu firmware-nya secara lokal supaya ada file `.bin`/`.elf`:
   ```bash
   cd sub_edge_esp32
   pio run
   ```
2. Buka [wokwi.com](https://wokwi.com), buat project baru **ESP32**.
3. Upload isi `wokwi/diagram.json` ke project Wokwi (menu "Diagram" ->
   paste/import), lalu upload file `firmware.bin`/`firmware.elf` hasil
   build ke tab "Firmware".
4. Klik tombol Play (▶) di Wokwi. Buka panel Serial Monitor di sana.

## Opsi B — Wokwi lewat VS Code (lebih nyaman untuk development)

1. Install ekstensi **Wokwi Simulator** di VS Code (perlu akun Wokwi,
   gratis untuk simulasi ESP32 dasar).
2. Pastikan sudah `pio run` (lihat langkah 1 di atas).
3. Buka folder `sub_edge_esp32` di VS Code, tekan `F1` -> "Wokwi: Start
   Simulator". Konfigurasi `wokwi/wokwi.toml` sudah menunjuk ke hasil
   build PlatformIO secara otomatis.

## Apa yang akan terlihat di Serial Monitor

Karena `USE_SIMULATED_SENSOR` sudah di-set `1` di `include/config.h`,
firmware TIDAK akan mencoba membaca sensor IIS3DWB sama sekali —
melainkan membangkitkan sinyal getaran buatan (`generateSimulatedSamples()`
di `src/main.cpp`). Urutan yang akan muncul:

```
[BOOT] edge01 | seq=0
[SENSOR-SIM] Sinyal getaran simulasi dibuat (sensor IIS3DWB tidak dipakai).
[MAIN] Akuisisi sensor selesai, memulai mesh...
...(menunggu ~8 detik, karena tidak ada node lain untuk join mesh)...
[MESH] Payload terkirim:
{"node_id":"edge01","seq":0,...,"x":{"rms":...,"dom_freq":150.x,...},...}
[SLEEP] Deep sleep selama 60 detik...
```

Bandingkan angka `rms`, `dom_freq`, dan `band` di JSON tersebut dengan
output `tools/dsp_validation.py` — seharusnya polanya konsisten (dominant
frequency mendekati `SIM_BASE_FREQ_HZ` = 150 Hz).

## Yang TIDAK tervalidasi lewat cara ini
- Koneksi ESP-MESH sungguhan antar node (di sini node akan mencoba join,
  timeout karena sendirian, lalu tetap lanjut kirim broadcast ke "ruang
  kosong" — ini sudah diantisipasi oleh logika `taskSendAndSleep`).
- Pembacaan SPI sensor IIS3DWB yang sesungguhnya.
- Konsumsi daya deep sleep yang sebenarnya (Wokwi tidak mensimulasikan
  arus listrik).

Ketiga hal ini baru bisa diuji setelah ada ESP32 fisik (dan untuk yang
terakhir, sensor + adapter STEVAL-MKI208V1K asli).

## Mengembalikan ke mode sensor asli
Setelah hardware IIS3DWB tersedia, ubah kembali di `include/config.h`:
```cpp
#define USE_SIMULATED_SENSOR   0
```
Seluruh kode lain (DSP, mesh, deep sleep) tidak perlu diubah sama sekali
— hanya sumber datanya yang berpindah dari simulasi ke sensor asli.
