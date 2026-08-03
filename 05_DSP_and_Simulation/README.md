# DSP and simulation

- `Konsep_algoritma_DSP_Revisi.m`: reference extraction with DC removal, Hann correction, velocity RMS, and fixed bands.
- `Konsep_trending_dan_alarm_Revisi.m`: 4-hour trending and baseline persistence logic.
- `Konsep_anomali_multivariat_Revisi.m`: exploratory multivariate anomaly score; not a production classifier.
- `Power_Transient_Realistic.cir`: LTspice-compatible nonideal battery/regulator/supercap/load model.
- `dsp_validation_revised.py`: executable desktop reference used for static numerical verification.

No universal alarm limit is embedded. Thresholds remain a commissioning output.
