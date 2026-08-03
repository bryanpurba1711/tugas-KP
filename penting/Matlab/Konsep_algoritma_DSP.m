% =========================================================================
% FASE 2: SIMULASI FFT, KONVERSI VELOCITY (mm/s), & KALKULASI RMS ISO 20816
% =========================================================================

%% 1. Parameter Akuisisi Data (Domain Waktu)
Fs = 4000;              % Frekuensi sampling 4 kHz
t = 0:1/Fs:2-1/Fs;      % Vektor durasi 2 detik

% Parameter Frekuensi Mesin Aktual (Asumsi: Motor 4-Pole, ~1470 RPM)
f_1x = 24.5;            % 1X RPM (Fundamental putaran mekanis)
f_2x = f_1x * 2;        % Harmonisa 2X
f_3x = f_1x * 3;        % Harmonisa 3X
f_elec = 50.0;          % Tarikan elektromagnetik frekuensi listrik
f_bpfo = f_1x * 3.58;   % Cacat Bearing (Outer)
f_bpfi = f_1x * 5.43;   % Cacat Bearing (Inner)

%% 2. Pembangkitan Sinyal Campuran (Percepatan dalam 'g') - KALIBRASI REALISTIS
% Amplitudo diturunkan drastis menyesuaikan getaran riil motor industri
x_mech = 0.09*sin(2*pi*f_1x*t) + 0.05*sin(2*pi*f_2x*t) + 0.02*sin(2*pi*f_3x*t);
x_elec = 0.03*sin(2*pi*f_elec*t);

% Frekuensi tinggi (Bearing) biasanya bisa memiliki nilai 'g' yang sedikit lebih 
% besar namun tidak akan menghasilkan mm/s yang tinggi
x_bear = 0.05*sin(2*pi*f_bpfo*t) + 0.03*sin(2*pi*f_bpfi*t);

% Latar belakang getaran pabrik (White Noise)
noise = 0.02*randn(size(t)); 

% Total Sinyal Percepatan (g) mentah
x_total = x_mech + x_elec + x_bear + noise;

%% 3. Komputasi Edge (Algoritma FFT Percepatan)
L = length(x_total);
Y = fft(x_total);
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f_spectrum = Fs*(0:(L/2))/L;

%% 4. KONVERSI KECEPATAN (VELOCITY mm/s) & OVERALL RMS ISO 20816
% Inisialisasi array untuk spektrum kecepatan
v_spectrum = zeros(size(P1)); 

% Mengonversi setiap titik frekuensi dari g ke mm/s
% (Dimulai dari index 2 untuk menghindari pembagian dengan 0 Hz / DC Offset)
for i = 2:length(f_spectrum)
    % Rumus: V_peak = (A_peak * 9810) / (2 * pi * frekuensi)
    v_spectrum(i) = (P1(i) * 9810) / (2 * pi * f_spectrum(i));
end

% Kalkulasi Overall RMS Velocity dari spektrum
% Mengingat v_spectrum adalah amplitudo puncak (peak), kita bagi sqrt(2) untuk mendapat RMS
% Lalu dikuadratkan, dijumlahkan (sum), dan diakarkan (sqrt)
rms_velocity_mms = sqrt(sum((v_spectrum / sqrt(2)).^2));

% ---> TAMBAHKAN BARIS INI UNTUK MENGHITUNG rms_g <---
rms_g = sqrt(mean(x_total.^2)); 
% (Atau jika menggunakan Signal Processing Toolbox, cukup ketik: rms_g = rms(x_total);)

% Menampilkan hasil di Command Window
disp('======================================================');
disp(['Nilai RMS Percepatan (Raw)   : ', num2str(rms_g, '%.4f'), ' g']);
disp(['Nilai Overall RMS Velocity   : ', num2str(rms_velocity_mms, '%.2f'), ' mm/s  <-- (Standar ISO)']);
disp('======================================================');

%% 5. Visualisasi Analitik
figure('Name', 'Analisis Spektrum & RMS Velocity', 'NumberTitle', 'off', 'Position', [100, 100, 900, 700]);

% Subplot 1: Sinyal Mentah (g)
subplot(3,1,1);
plot(t, x_total, 'b');
title(['Domain Waktu: Sinyal Percepatan (RMS: ', num2str(rms_g, '%.4f'), ' g)']);
xlabel('Waktu (detik)'); ylabel('Amplitudo (g)');
xlim([0 0.5]); grid on;

% Subplot 2: Spektrum Percepatan (FFT g)
subplot(3,1,2);
plot(f_spectrum, P1, 'r');
title('Domain Frekuensi: Spektrum Percepatan (g)');
xlabel('Frekuensi (Hz)'); ylabel('Magnitudo (g)');
xlim([0 200]); grid on;

% Subplot 3: Spektrum Kecepatan (FFT mm/s)
subplot(3,1,3);
plot(f_spectrum, v_spectrum, 'k');
title(['Domain Frekuensi: Spektrum Kecepatan (Overall RMS: ', num2str(rms_velocity_mms, '%.2f'), ' mm/s)']);
xlabel('Frekuensi (Hz)'); ylabel('Kecepatan Puncak (mm/s)');
xlim([0 200]); grid on;

% -------------------------------------------------------------------------
% PROTOKOL KENDALI GRAFIS (PENCEGAHAN RESET ZOOM LEVEL)
% -------------------------------------------------------------------------
sgrid;                  % Langkah 1
axis auto;              % Langkah 2
% -------------------------------------------------------------------------