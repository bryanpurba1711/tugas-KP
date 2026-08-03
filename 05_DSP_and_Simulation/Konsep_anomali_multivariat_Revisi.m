%% Simple multivariate anomaly score for prototype validation
clear; clc; close all; rng(5);
N = 120;
features = [1.5+0.12*randn(N,1), 2.6+0.2*randn(N,1), ...
            3.0+0.25*randn(N,1), 18+1.5*randn(N,1)];
features(101:end,:) = features(101:end,:) + ...
    [linspace(0,1.8,20)' linspace(0,2.5,20)' linspace(0,2,20)' linspace(0,12,20)'];
baseline = features(1:60,:);
mu = mean(baseline); sigma = std(baseline);
z = (features-mu)./sigma;
score = sqrt(sum(z.^2,2));
threshold = prctile(sqrt(sum(((baseline-mu)./sigma).^2,2)),99);
figure; plot(score,'LineWidth',1.2); hold on; yline(threshold,'--'); grid on;
xlabel('4-hour sample'); ylabel('Standardized anomaly score');
title('Prototype anomaly score — requires field baseline and validation');
