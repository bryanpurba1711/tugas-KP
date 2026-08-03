%% Trending 4-hourly — baseline-based alarm philosophy
clear; clc; close all;
steps = 6*14; time_h = (0:steps-1)*4; rng(11);
velocity = 1.8 + 0.12*randn(1,steps);
temp_obj = 48 + 0.8*randn(1,steps);
% Simulated degradation in final four days
idx = time_h >= 10*24;
velocity(idx) = velocity(idx) + linspace(0,2.0,sum(idx));
temp_obj(idx) = temp_obj(idx) + linspace(0,12,sum(idx));

baseline_n = 6*3; % first three days
v_mu = mean(velocity(1:baseline_n)); v_sd = std(velocity(1:baseline_n));
t_mu = mean(temp_obj(1:baseline_n)); t_sd = std(temp_obj(1:baseline_n));
v_warning = v_mu + 3*v_sd; t_warning = t_mu + 3*t_sd;

raw_alarm = velocity > v_warning | temp_obj > t_warning;
persistent_alarm = movsum(raw_alarm,[2 0]) >= 2; % >=2 of last 3 samples

figure('Name','Condition trending');
subplot(2,1,1); plot(time_h/24,velocity,'-o'); hold on; yline(v_warning,'--');
scatter(time_h(persistent_alarm)/24,velocity(persistent_alarm),30,'filled'); grid on;
ylabel('Velocity RMS (mm/s)'); title('Baseline-derived warning; verify against machine class');
subplot(2,1,2); plot(time_h/24,temp_obj,'-o'); hold on; yline(t_warning,'--');
scatter(time_h(persistent_alarm)/24,temp_obj(persistent_alarm),30,'filled'); grid on;
xlabel('Day'); ylabel('Object temperature (°C)');
