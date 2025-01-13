% Simplifed Randles model with wahburg coeff

freq = logspace(-1, 5, 500);
omega = 2 * pi * freq;

Rs = 20;
Rp = 250;
Cdl = 40e-6;
sigma = 150;

Z_Rs = Rs;
Z_Cdl = 1 ./ (1j*omega*Cdl);
Z_W = sigma * (1j * omega).^(-1/2)*(1 - 1j);
Z_Rp = Rp + Z_W;

Z_parallel = 1 ./ (1 ./ Z_Rp + 1 ./ Z_Cdl);

Z_total = Z_Rs + Z_parallel;

figure;
subplot(3,1,1);
plot(real(Z_total), -imag(Z_total), 'r', 'LineWidth', 1.5);
xlabel('Real(Z) [Ohms]');
ylabel('Imag(Z) [Ohms]');
title('Nyquist Plot');
grid on;
axis equal;

subplot(3,1,2);
semilogx(freq, 20*log10(abs(Z_total)), 'r', 'LineWidth', 1.5);
xlabel('Frequency [Hz]');
ylabel('Magnitude [dB]');
title('Bode Plot: Magnitude (Simplified Randles Cell)');
grid on;

subplot(3,1,3);
semilogx(freq, rad2deg(angle(Z_total)), 'r', 'LineWidth', 1.5);
xlabel('Frequency [Hz]');
ylabel('Phase [Degrees]');
title('Bode Plot: Phase (Simplified Randles Cell)');
grid on;
