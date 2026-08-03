# Tools — validasi tanpa hardware

## `dsp_validation.py`
Menjalankan ulang logika DSP yang sama persis strukturnya dengan
`sub_edge_esp32/src/dsp_processing.cpp` (remove DC -> Hamming window ->
FFT -> RMS/peak/crest/dominant frequency/band energy), tapi di Python
dengan sinyal getaran sintetis (bukan data dari ESP32).

Gunanya: membuktikan rumus DSP benar SEBELUM repot compile ke ESP32.
Membandingkan 2 skenario — motor sehat vs motor dengan indikasi cacat
bearing (impact periodik) — dan menunjukkan bagaimana crest factor dan
distribusi band energy berubah.

```bash
pip install numpy matplotlib
python dsp_validation.py
```

Akan mencetak angka fitur ke terminal dan menyimpan plot
`dsp_validation_output.png` (time-domain + spektrum FFT untuk tiap
skenario).
