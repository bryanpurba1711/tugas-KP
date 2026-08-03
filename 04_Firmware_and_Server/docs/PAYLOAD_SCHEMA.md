# Payload schema 1.0

Topic: `kp/edge/<node_id>/features`

```json
{
  "schema_version": "1.0",
  "firmware_version": "1.0.0-revA",
  "type": "condition_features",
  "node_id": "edge01",
  "seq": 1,
  "battery_mv": 3560,
  "sample_rate_hz": 26667,
  "sample_count": 8192,
  "acquisition_valid": true,
  "temperature": {"valid": true, "object_c": 54.2, "ambient_c": 31.0, "delta_c": 23.2},
  "vibration": {
    "x": {"accel_rms_g": 0.1, "peak_g": 0.3, "crest_factor": 3.0, "kurtosis": 3.2, "dominant_freq_hz": 49.0, "velocity_rms_mm_s": 2.1, "band_rms_g": [0.1,0.02,0.01,0.01,0.005]},
    "y": {}, "z": {}
  },
  "alarm_state": "UNCONFIGURED_BASELINE"
}
```

`alarm_state` tetap `UNCONFIGURED_BASELINE` sampai baseline motor, kelas mesin, lokasi pengukuran, dan ambang yang disetujui tersedia.
