%% Monitoring vibrasi motor — konsep DSP revisi DB-001 Rev.1.0
clear; clc; close all;
Fs = 26667; N = 8192; t = (0:N-1)'/Fs;
rpm = 1470; fr = rpm/60;
% Sinyal contoh: 1X, 2X, 50 Hz, komponen bearing, noise, dan impact
rng(7);
a_g = 0.10*sin(2*pi*fr*t) + 0.04*sin(2*pi*2*fr*t) + ...
      0.03*sin(2*pi*50*t) + 0.015*sin(2*pi*1500*t) + 0.01*randn(size(t));
a_g(1:round(Fs/20):end) = a_g(1:round(Fs/20):end) + 0.25;

% DC/gravity removal sebelum fitur time-domain
x = detrend(a_g,'constant');
accel_rms_g = rms(x);
peak_g = max(abs(x));
crest_factor = peak_g/accel_rms_g;
kurtosis_value = kurtosis(x);

% Hann window + coherent-gain correction
w = hann(N,'periodic'); cg = mean(w);
X = fft(x.*w);
f = (0:N/2)'*Fs/N;
A_peak_g = 2*abs(X(1:N/2+1))/(N*cg);
A_peak_g([1 end]) = A_peak_g([1 end])/2;
A_rms_g = A_peak_g/sqrt(2);

% Dominant frequency pada bandwidth evaluasi sensor
valid = f >= 10 & f <= 6000;
[~,idx] = max(A_peak_g(valid));
f_valid = f(valid); dominant_freq_hz = f_valid(idx);

% Velocity RMS dari acceleration spectrum, 10–1000 Hz
v_rms_bin_mm_s = zeros(size(f));
nonzero = f > 0;
v_rms_bin_mm_s(nonzero) = A_rms_g(nonzero)*9.80665./(2*pi*f(nonzero))*1000;
vband = f >= 10 & f <= 1000;
velocity_rms_mm_s = sqrt(sum(v_rms_bin_mm_s(vband).^2));

% Fixed engineering bands, bukan pembagian linear Nyquist
bands = [10 100; 100 500; 500 1000; 1000 3000; 3000 6000];
band_rms_g = zeros(size(bands,1),1);
for k = 1:size(bands,1)
    b = f >= bands(k,1) & f < bands(k,2);
    band_rms_g(k) = sqrt(sum(A_rms_g(b).^2));
end

fprintf('Acceleration RMS = %.4f g\n',accel_rms_g);
fprintf('Peak = %.4f g; Crest = %.2f; Kurtosis = %.2f\n',peak_g,crest_factor,kurtosis_value);
fprintf('Dominant frequency = %.2f Hz\n',dominant_freq_hz);
fprintf('Velocity RMS 10-1000 Hz = %.3f mm/s\n',velocity_rms_mm_s);

figure('Name','DSP revisi');
subplot(2,1,1); plot(t,x); xlim([0 0.2]); grid on;
xlabel('Time (s)'); ylabel('Acceleration (g)'); title('DC-removed vibration');
subplot(2,1,2); semilogy(f,A_rms_g+eps); xlim([0 6000]); grid on;
xlabel('Frequency (Hz)'); ylabel('RMS amplitude (g)'); title('Hann-corrected spectrum');

% Alarm intentionally not assigned here. Configure from machine baseline,
% measurement point, machine class, and approved evaluation standard.
