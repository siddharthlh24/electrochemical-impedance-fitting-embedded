#include <Arduino.h>
#include "Complex.h"

// Constants and initial conditions
const float Rs_true = 20.0;
const float Rp_true = 250.0;
const float Cdl_true = 40;  // nothe that this is micro farads ( the impedance calculator considers this)
const float sigma_true = 150.0;
float initial_guess[] = {10.0, 150.0, 60, 100.0};
const int num_points = 10;
const int max_iterations = 200;
const float tol = 1e-6;

float freq[num_points];
float omega[num_points];
Complex Z_total_true[num_points];
float Z_real_noisy[num_points];
float Z_imag_noisy[num_points];

// Generate logarithmically spaced frequency values
void generateLogspace(float start, float end, int num, float *arr) {
    float log_start = log10(start);
    float log_end = log10(end);
    float delta = (log_end - log_start) / (num - 1);
    for (int i = 0; i < num; i++) {
        arr[i] = pow(10, log_start + i * delta);
    }
}

// Calculate total impedance based on parameters
void calculateImpedance(float Rs, float Rp, float Cdl, float sigma, Complex *Z_total) {
    for (int i = 0; i < num_points; i++) {
        Complex Z_Rs(Rs, 0);
        Complex Z_Cdl(0, -1 / (omega[i] * Cdl * 1e-6));
        Complex Z_W(0, sigma * sqrt(omega[i]));
        Complex Z_Rp(Rp, 0);
        Z_Rp = Z_Rp + Z_W;

        Complex Z_parallel = Complex(1, 0) / (Complex(1, 0) / Z_Rp + Complex(1, 0) / Z_Cdl);
        Z_total[i] = Z_Rs + Z_parallel;
    }
}

// Add noise to real and imaginary parts
void addNoise(float *real, float *imag) {
    float noise_power = 0.05;  // 5% noise
    for (int i = 0; i < num_points; i++) {
        Z_real_noisy[i] = real[i] + noise_power * real[i] * ((float)rand() / RAND_MAX - 0.5);
        Z_imag_noisy[i] = imag[i] + noise_power * imag[i] * ((float)rand() / RAND_MAX - 0.5);
    }
}

// Objective function to minimize
float objectiveFunction(float *params) {
    float Rs = params[0];
    float Rp = params[1];
    float Cdl = params[2];
    float sigma = params[3];
    
    Complex Z_model[num_points];
    calculateImpedance(Rs, Rp, Cdl, sigma, Z_model);

    float residual = 0.0;
    // Sweep over muliple frequencies for diversity
    for (int i = 0; i < num_points; i++) {
        float real_residual = Z_real_noisy[i] - Z_model[i].real();
        float imag_residual = Z_imag_noisy[i] - Z_model[i].imag();
        residual += real_residual * real_residual + imag_residual * imag_residual;
    }
    return residual;
}

// Perform the Nelder-Mead optimization
void nelderMeadOptimization(float *start, int n, int max_iter, float tol) {
    const float alpha = 1.0;    // Reflection coefficient
    const float gamma = 2.0;    // Expansion coefficient
    const float rho = 0.5;      // Contraction coefficient
    const float sigma = 0.5;    // Shrink coefficient

    float simplex[n+1][n];
    float f_values[n+1];

    // Initialize simplex points and their function values
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j < n; j++) {
            simplex[i][j] = start[j];
        }
        if (i > 0) {
            simplex[i][i-1] = start[i-1] * 1.05; // Perturbation to form the simplex
        }
        f_values[i] = objectiveFunction(simplex[i]);
    }

    for (int iter = 0; iter < max_iter; iter++) {
        // Sort simplex vertices by their function values
        for (int i = 0; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                if (f_values[j] < f_values[i]) {
                    // Swap f_values
                    float temp_f = f_values[i];
                    f_values[i] = f_values[j];
                    f_values[j] = temp_f;

                    // Swap simplex points
                    for (int k = 0; k < n; k++) {
                        float temp = simplex[i][k];
                        simplex[i][k] = simplex[j][k];
                        simplex[j][k] = temp;
                    }
                }
            }
        }

        // Calculate centroid of all points except the worst
        float centroid[n];
        for (int j = 0; j < n; j++) {
            centroid[j] = 0.0;
            for (int i = 0; i < n; i++) {
                centroid[j] += simplex[i][j];
            }
            centroid[j] /= n;
        }

        // Reflection
        float reflected[n];
        for (int j = 0; j < n; j++) {
            reflected[j] = centroid[j] + alpha * (centroid[j] - simplex[n][j]);
        }
        float f_reflected = objectiveFunction(reflected);

        if (f_reflected < f_values[0]) {
            // Expansion
            float expanded[n];
            for (int j = 0; j < n; j++) {
                expanded[j] = centroid[j] + gamma * (reflected[j] - centroid[j]);
            }
            float f_expanded = objectiveFunction(expanded);

            if (f_expanded < f_reflected) {
                for (int j = 0; j < n; j++) {
                    simplex[n][j] = expanded[j];
                }
                f_values[n] = f_expanded;
            } else {
                for (int j = 0; j < n; j++) {
                    simplex[n][j] = reflected[j];
                }
                f_values[n] = f_reflected;
            }
        } else if (f_reflected < f_values[n-1]) {
            // Accept the reflected point
            for (int j = 0; j < n; j++) {
                simplex[n][j] = reflected[j];
            }
            f_values[n] = f_reflected;
        } else {
            // Contraction
            float contracted[n];
            for (int j = 0; j < n; j++) {
                contracted[j] = centroid[j] + rho * (simplex[n][j] - centroid[j]);
            }
            float f_contracted = objectiveFunction(contracted);

            if (f_contracted < f_values[n]) {
                for (int j = 0; j < n; j++) {
                    simplex[n][j] = contracted[j];
                }
                f_values[n] = f_contracted;
            } else {
                // Shrink
                for (int i = 1; i <= n; i++) {
                    for (int j = 0; j < n; j++) {
                        simplex[i][j] = simplex[0][j] + sigma * (simplex[i][j] - simplex[0][j]);
                    }
                    f_values[i] = objectiveFunction(simplex[i]);
                }
            }
        }

        // Check convergence
        float max_diff = 0.0;
        for (int i = 0; i < n; i++) {
            max_diff = max(max_diff, abs(f_values[0] - f_values[i]));
        }
        if (max_diff < tol) {
            Serial.print("Reached convergence in ");
            Serial.print(iter);
            Serial.println("iterations");
            break;
        }
    }

    // Print the estimated parameters
    Serial.println("Estimated Parameters:");
    for (int j = 0; j < n; j++) {
        Serial.print("Param ");
        Serial.print(j);
        Serial.print(": ");
        Serial.println(simplex[0][j]);
    }
}

void optimizationTask(void *pvParameters) {
    generateLogspace(0.1, 10000, num_points, freq);
    for (int i = 0; i < num_points; i++) {
        omega[i] = 2 * PI * freq[i];
    }

    // Calculate true impedance
    calculateImpedance(Rs_true, Rp_true, Cdl_true, sigma_true, Z_total_true);

    // Extract real and imaginary parts
    float Z_real[num_points];
    float Z_imag[num_points];
    for (int i = 0; i < num_points; i++) {
        Z_real[i] = Z_total_true[i].real();
        Z_imag[i] = Z_total_true[i].imag();
    }

    // Add noise
    addNoise(Z_real, Z_imag);

    // Measure time before Nelder-Mead optimization
    unsigned long startTime = millis();  // Store the start time in milliseconds

    // Start Nelder-Mead optimization
    nelderMeadOptimization(initial_guess, 4, max_iterations, tol);

    // Measure time after Nelder-Mead optimization
    unsigned long endTime = millis();  // Store the end time in milliseconds

    // Calculate the time taken
    unsigned long duration = endTime - startTime;

    // Print the time taken for the optimization
    Serial.print("Nelder-Mead optimization took: ");
    Serial.print(duration);  // Duration in milliseconds
    Serial.println(" milliseconds");

    vTaskDelete(NULL);
}


void setup() {
    Serial.begin(115200);
    vTaskDelay(1000u);

    // Create a task with a large stack for optimization
    xTaskCreatePinnedToCore(
        optimizationTask,       // Task function
        "OptimizationTask",     // Task name
        16384,                   // Stack size (adjustable)
        NULL,                   // Parameters to pass to the task
        1,                      // Task priority
        NULL,                   // Task handle (not needed)
        1                       // Core to run the task on (0 or 1)
    );
    Serial.println("Optimization task created!");
}

void loop() {
    // Placeholder loop
}
