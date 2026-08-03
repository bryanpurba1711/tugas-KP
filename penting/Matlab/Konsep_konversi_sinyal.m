% =========================================================================
% SIMULASI HMI GRAFANA: TREN KONDISI MOTOR (STANDAR ISO 20816)
% =========================================================================

%% 1. Parameter Simulasi (Waktu & Data)
% Pengambilan sampel setiap 4 jam dalam sehari (24 jam)
waktu_jam = [0, 4, 8, 12, 16, 20, 24];

% Skenario Data Velocity RMS (mm/s) dari ESP32
% Asumsi: Pagi hari mesin sehat, siang mulai degradasi, malam masuk zona bahaya
v_rms = [1.5, 1.8, 2.2, 3.5, 4.4, 6.2, 7.8]; 

%% 2. Pembuatan Kanvas Dashboard HMI
figure('Name', 'Dashboard HMI - Tren Getaran Motor', 'NumberTitle', 'off', 'Position', [100, 100, 800, 500]);
hold on;

% -------------------------------------------------------------------------
% 3. PEMBUATAN ZONA WARNA STANDAR ISO 20816
% Menggunakan patch untuk membuat blok warna di latar belakang
% -------------------------------------------------------------------------
% Zona A (Hijau) - Kondisi Sangat Baik (< 2.3 mm/s)
patch([0 24 24 0], [0 0 2.3 2.3], 'g', 'FaceAlpha', 0.2, 'EdgeColor', 'none', 'DisplayName', 'Zona A (Baik)');

% Zona B (Kuning) - Kondisi Normal (2.3 - 4.5 mm/s)
patch([0 24 24 0], [2.3 2.3 4.5 4.5], 'y', 'FaceAlpha', 0.2, 'EdgeColor', 'none', 'DisplayName', 'Zona B (Normal)');

% Zona C (Oranye) - Kondisi Peringatan (4.5 - 7.1 mm/s)
patch([0 24 24 0], [4.5 4.5 7.1 7.1], [1 0.5 0], 'FaceAlpha', 0.2, 'EdgeColor', 'none', 'DisplayName', 'Zona C (Alert)');

% Zona D (Merah) - Kondisi Bahaya (> 7.1 mm/s)
patch([0 24 24 0], [7.1 7.1 10 10], 'r', 'FaceAlpha', 0.2, 'EdgeColor', 'none', 'DisplayName', 'Zona D (Danger/Trip)');

% -------------------------------------------------------------------------
% 4. PLOT DATA TREN GETARAN MOTOR
% -------------------------------------------------------------------------
plot(waktu_jam, v_rms, '-ko', 'LineWidth', 2, 'MarkerSize', 8, 'MarkerFaceColor', 'k', 'DisplayName', 'Nilai Getaran Aktual');

%% 5. Formatting Tampilan HMI
title('Simulasi HMI: Tren Getaran Motor (ISO 20816)', 'FontSize', 14, 'FontWeight', 'bold');
xlabel('Waktu Operasional (Jam)', 'FontSize', 12);
ylabel('Velocity RMS (mm/s)', 'FontSize', 12);

% Pengaturan Sumbu
xlim([0 24]);
ylim([0 9]);
xticks(0:4:24);

% Menampilkan Legenda dan Grid
legend('Location', 'northwest');
grid on;
hold off;