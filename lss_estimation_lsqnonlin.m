clc
clear all 
close all

% True data
Rs_true = 20;
Rp_true = 250;
Cdl_true = 40e-6;
sigma_true = 150;

% Initial guess
initial_guess = [10, 150, 20e-6, 100];  % [Rs, Rp, Cdl, sigma]

freq = logspace(-1, 4, 500);
omega = 2 * pi * freq;


Z_Rs = Rs_true;                    % electrolyte resistance (Rs)
Z_Cdl = 1 ./ (1j * omega * Cdl_true);  % capacitance impedance
Z_W = sigma_true * (1j * omega).^(-1/2);  % Warburg impedance
Z_Rp = Rp_true + Z_W;              % Polarization resistance (Rp)
Z_parallel = 1 ./ (1 ./ Z_Rp + 1 ./ Z_Cdl);
Z_total_true = Z_Rs + Z_parallel;

% Simulated data (reference for fitting)
Z_real = real(Z_total_true);
Z_imag = imag(Z_total_true);

%%%%%%%%%%%%%%%%%%%% Adding noise %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
noise_power = 0.05;  % 5% noise

Z_real_noisy = Z_real + noise_power * Z_real .* randn(size(Z_real));
Z_imag_noisy = Z_imag + noise_power * Z_imag .* randn(size(Z_imag));

Z_data = [Z_real_noisy, Z_imag_noisy];


%%%%%%%%%%% least squarese fit %%%%%%%%%%%%%%%%%%%

residual = @(params) calculate_residual(params, omega, Z_data);

options = optimset('Display', 'iter', 'TolFun', 1e-6);

% least square fit non linear
[params_estimated, resnorm, residual_output] = lsqnonlin(residual, initial_guess, [], [], options);

disp('Params:');
disp(['Rs: ', num2str(params_estimated(1))]);
disp(['Rp: ', num2str(params_estimated(2))]);
disp(['Cdl: ', num2str(params_estimated(3))]);
disp(['Sigma: ', num2str(params_estimated(4))]);

% Function to calculate the residuals (difference between measured and model data)
function res = calculate_residual(params, omega, Z_data)
    % Extract the parameters
    Rs = params(1);
    Rp = params(2);
    Cdl = params(3);
    sigma = params(4);
    
    Z_Rs = Rs;                    % electrolyte resistance (Rs)
    Z_Cdl = 1 ./ (1j * omega * Cdl);  % capacitance impedance
    Z_W = sigma * (1j * omega).^(-1/2);  % Warburg impedance
    Z_Rp = Rp + Z_W;              % Polarization resistance (Rp)
    Z_parallel = 1 ./ (1 ./ Z_Rp + 1 ./ Z_Cdl);
    Z_total = Z_Rs + Z_parallel;
    
    Z_real_model = real(Z_total);
    Z_imag_model = imag(Z_total);
    
    Z_model = [Z_real_model, Z_imag_model];
    
    % residual
    res = Z_data(:) - Z_model(:);
end
