"""Desktop reference matching the revised embedded DSP definitions."""
from __future__ import annotations
import json
import numpy as np

FS = 26667.0; N = 8192
BANDS = [(10,100),(100,500),(500,1000),(1000,3000),(3000,6000)]

def features(x: np.ndarray) -> dict:
    x = np.asarray(x, dtype=float) - np.mean(x)
    rms = float(np.sqrt(np.mean(x*x))); peak = float(np.max(np.abs(x)))
    crest = peak/rms if rms else 0.0
    kurt = float(np.mean(x**4)/(np.mean(x*x)**2)) if rms else 0.0
    w = np.hanning(len(x)); cg = np.mean(w)
    spec = np.fft.rfft(x*w)
    f = np.fft.rfftfreq(len(x),1/FS)
    peak_g = 2*np.abs(spec)/(len(x)*cg); peak_g[[0,-1]] /= 2
    rms_g = peak_g/np.sqrt(2)
    valid = (f>=10)&(f<=6000); dom = float(f[valid][np.argmax(peak_g[valid])])
    v = np.zeros_like(f); nz=f>0; v[nz]=rms_g[nz]*9.80665/(2*np.pi*f[nz])*1000
    velocity=float(np.sqrt(np.sum(v[(f>=10)&(f<=1000)]**2)))
    band=[float(np.sqrt(np.sum(rms_g[(f>=lo)&(f<hi)]**2))) for lo,hi in BANDS]
    return {"acceleration_rms_g":rms,"peak_g":peak,"crest_factor":crest,
            "kurtosis":kurt,"dominant_freq_hz":dom,
            "velocity_rms_mm_s":velocity,"band_rms_g":band}

def main() -> None:
    rng=np.random.default_rng(12); t=np.arange(N)/FS
    x=.1*np.sin(2*np.pi*24.5*t)+.03*np.sin(2*np.pi*50*t)+.01*rng.normal(size=N)
    print(json.dumps(features(x),indent=2))
if __name__ == "__main__": main()
