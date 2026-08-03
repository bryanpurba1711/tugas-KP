% =========================================================================
% FASE 2 - STEP 3: ANALISIS DINAMIKA SISTEM ORDE-6 (WORST-CASE SCENARIO)
% =========================================================================

%% 1. Pemodelan Motor Induksi Kompleks (Orde-6)
% Persamaan denominator didapat dari konvolusi 3 subsistem:
% Elektrik x Inersia Rotor x Resonansi Kopling Fleksibel
num_wc = [56250000];
den_wc = [1 83 3667 83485 2486000 6362500 56250000]; 
sys_worst_case = tf(num_wc, den_wc);

%% 2. Pengaturan Kanvas Grafis Analitik (Dashboard Insinyur)
figure('Name', 'Analisis Worst-Case: Sistem Elektromekanis Orde-6', 'NumberTitle', 'off', 'Position', [100, 100, 900, 700]);

% Membuat visualisasi Pole-Zero Map 
pzmap(sys_worst_case);
title('Peta Pole-Zero: Kondisi Worst-Case Motor Induksi (Orde-6)', 'FontSize', 14, 'FontWeight', 'bold');
xlabel('Real Axis (Stabilitas)');
ylabel('Imaginary Axis (Frekuensi Osilasi)');

% -------------------------------------------------------------------------
% HIERARKI SINTAKSIS KRUSIAL: PENCEGAHAN GLITCH ZOOM LEVEL
% Pada sistem orde tinggi, titik pole saling tumpang tindih dengan grid.
% Urutan eksekusi ini mutlak dipatuhi agar kanvas tidak me-reset paksa.
% -------------------------------------------------------------------------
sgrid;                  % URUTAN 1: Eksekusi grid rasio redaman dan frekuensi alami
axis([-50 5 -40 40]);   % URUTAN 2: Kunci koordinat layar untuk menampung 6 kutub ekstrem
% -------------------------------------------------------------------------

disp('======================================================');
disp('Status: Sistem WORST-CASE Orde-6 Berhasil Dirender!');
disp('Proteksi Glitch Visual: AKTIF (Koordinat Axis Terkunci)');
disp('======================================================');