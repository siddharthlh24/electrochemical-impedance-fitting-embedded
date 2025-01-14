# Electrochemical Impedance Fitting Embedded

**Estimation of EIS Parameters using Randle Cell Model with Warburg, and Embedded C Implementation on ESP32.**  

## Table of Contents
- [Aim](#aim)
- [Description](#description)
- [Modelled Parameters and Physical Interpretation](#modelled-parameters-and-physical-interpretation)
- [Cell Model / Equivalent Impedance Circuit](#cell-model--equivalent-impedance-circuit)
- [Matlab Impedance Simulation](#matlab-simulation)
- [Cell Parameter Estimation](#cell-parameter-estimation)
- [Embedded Platform Implementation](#embedded-platform-implementation)
- [Acknowledgments](#acknowledgments)

---

## Aim
This project uses Electrochemical Impedance Spectroscopy (EIS) with the Randles Cell Model (including Warburg impedance) to estimate cell parameters through Least Squares Estimation. The goal is to identify circuit components affecting impedance, aiding in the assessment of cell aging and degradation.

The estimation process is embedded on an ESP32 microcontroller using the Nelder-Mead optimization method. By performing real-time optimization on the edge, the system provides efficient, on-site monitoring of cell health and performance without the need for external computation. The embedded results are compared with simulated data to evaluate accuracy and performance.

---

## Description
We simulate Electrochemical Impedance Spectroscopy (EIS) for a cell using the **Randles Cell Model**, and extend it to include the **Randle Cell Model with Warburg** coefficient.  
Cell parameter estimation is performed using **Least Squares Estimation** in MATLAB to estimate the cell parameters from simulated impedance measurements, based on an 


## Aim
This project uses Electrochemical Impedance Spectroscopy (EIS) with the Randles Cell Model (including Warburg impedance) to estimate cell parameters through Least Squares Estimation. The goal is to identify circuit components affecting impedance, aiding in the assessment of cell aging and degradation.

The estimation process is embedded on an ESP32 microcontroller using the Nelder-Mead optimization method. By performing real-time optimization on the edge, the system provides efficient, on-site monitoring of cell health and performance without the need for external computation. The embedded results are compared with simulated data to evaluate accuracy and performance.

## Description

We simulate Electrochemical Impedance Spectroscopy (EIS) for a cell using the **Randles Cell Model**, and extend it to include the **Randle Cell Model with Warburg** coefficient.  
Cell parameter estimation is performed using **Least Squares Estimation** in MATLAB to estimate the cell parameters from simulated impedance measurements, based on an initial guess. 
The same cell model parameter estimation is replicated on an embedded platform (Arduino ESP32) using the **Nelder-Mead Method** of Least Squares. The results are matched, and the approximate performance is measured/timed (Randle's model with Warburg).

## Modelled Parameters and Physical Interpretation

| **Parameter**        | **Physical Interpretation**                                                                                                                                                  | **Increase in Parameter**                                                                                               | **Decrease in Parameter**                                                                                              |
|----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------|
| **Rs (Solution Resistance)** | Resistance of the electrolyte between the electrodes.                                                                                                                   | - Low temperature (reduces ionic mobility).<br>- Electrolyte leakage (reduces ionic path).<br>- Contamination or electrolyte depletion (reduces ionic concentration). | - Higher temperature (increases ionic mobility).<br>- Improved electrolyte composition (more conductive).               |
| **Rp/Rct (Polarization Resistance)** | Resistance at the electrode-electrolyte interface, linked to charge transfer resistance.                                                                           | - Surface passivation (formation of insulating layers).<br>- Reduced catalyst activity (slower electrochemical reactions).<br>- Electrochemical degradation (corrosion or fouling of electrodes). | - Improved electrode surface (more active material).<br>- Higher temperature (increased reaction rates).               |
| **Cdl (Double Layer Capacitance)** | Capacitance of the electrochemical double layer at the electrode-electrolyte interface.                                                                           | - Larger electrode surface area (more active sites).<br>- Higher dielectric constant in electrolyte (improves charge storage). | - Electrode surface degradation (loss of active sites).<br>- Lower dielectric constant in electrolyte (reduced charge storage). |
| **Zw (Warburg Coefficient)** | Diffusion impedance, indicating how ion diffusion affects impedance.                                                                                                  | - Poor ion diffusion (thicker diffusion layers, lower temperature, or high-viscosity electrolytes).<br>- Electrode blockage (byproducts hindering diffusion). | - Improved ion mobility (better electrolyte composition, higher temperature).<br>- Thinner diffusion layer (optimized stirring or electrode design). |

## Cell model / equivalent impedance circuit
Reference : https://upload.wikimedia.org/wikipedia/commons/thumb/9/93/Randles_circuit.png/220px-Randles_circuit.png
![Randles Circuit](media/Randles_circuit.png)
Int the above diagram, simple Randles model does not include Warberg impedance (Zw).
The Warburg infinite impedance (\( Z_W \)) is given by the equation:
$$
Z_W = \sigma \omega^{-1/2} (1 - j)
$$
Where:
- \( \sigma \) is the Warburg coefficient.
- \( \omega \) is the angular frequency, \( \omega = 2 \pi f \).
- \( f \) is the frequency.
Real life simulation use warberg finite impedance and depends on the geommetry of the cell and other factors. For our purposes, the infinite model is enough as we are not aiming to create a very accurate model, but rather focus on finding the circuital equivalents given measured impedance..


## Matlab Simulation
We simulate impedance values referencing cell parameter values first using a simple Randles model ( No Warberg ), and then including Warberg. We will later use the simulated impedance result to back estimate the 

Rs = 20 ohm
Rp = 250 ohm
Cdl = 40 uF
sigma = 150 ( warberg coeff.)

I have tried to match result in reference : https://www.gamry.com/application-notes/EIS/basics-of-electrochemical-impedance-spectroscopy/
We plot real vs complex impedance , and bode plot of real and imaginary components of impedance vs freqency. The Warberg model does not match up exactly as the referece, because I think refernce uses finite Warburg impedance instead of infinite. That is fine at this point in time.

### Simplifed model
[RANDLES SIMPLE CODE MATLAB](randles_simplified_model.m)
![Simplifed model](media/randles_simple_impedance.png)

### Randles with Warberg
[WARBERG ADDED CODE MATLAB](randles_with_warberg.m)
![Randles with Warberg](media/randles_warberg_impedance.png)

## Cell parameter estimation
Simulate Electrochemical Impedance Spectroscopy (EIS) data using the Randle Cell model with Warburg impedance. We use a freqeuncy sweep from 0.1 to 10khz AC stimulus and impedance measurement (10 measurements logspace spread).
Define true values for the model parameters: Rs (Solution Resistance), Rp (Polarization Resistance), Cdl (Double Layer Capacitance), and sigma (Warburg Coefficient).
Generate noisy simulated impedance data for fitting. This is done on purpose, measurements arent ever perfect.
Use Least Squares Fitting (via fminsearch) to estimate the model parameters by minimizing the difference between noisy data and model predictions.
The estimation process adjusts the parameters to best match the simulated data.
Output estimated values for Rs, Rp, Cdl, and sigma after optimization.

We tried 2 different algorithms/methods in MATLAB. The embedded C implementation follows from the NelderMead method.

[lsqnonlin method MATLAB](lss_estimation_lsqnonlin.m)

```
Estimated Parameters:
Rs: 19.928
Rp: 248.421
Cdl: 3.9958e-05
Sigma: 151.7565
```

[NelderMead method MATLAB](lss_estimation_NelderMead.m)

```
Estimated Parameters:
Rs: 21.7962
Rp: 270.9082
Cdl: 4.0415e-05
Sigma: 147.6066
```

There will be run to run variance due to artificially added noise to impedance measurments. 

## Embedded platform implementation

We implement Least Squares Estimation on the ESP32 using the Nelder-Mead optimization method for cell parameter estimation. The process involves:

- **Frequency Generation**: Logarithmic sweep from 0.1 Hz to 10 kHz.
- **Impedance Calculation**: Simulated using the Randle Cell model with Warburg impedance.
- **Noisy Data Simulation**: Adds noise to mimic real-world measurements.
- **Objective Function**: Minimizes the residual sum of squares between noisy data and model predictions.
- **Nelder-Mead Optimization**: Iteratively refines parameters (\(Rs, Rp, Cdl, \sigma\)) to fit the impedance data.

[Cell parameter estimation ESP32](randles_embedded/randles_embedded.ino)
The optimized parameters are outputted along with the time taken for the process.
Note : Stack/resoucre usage is still not optimized in anyway and the time taken is an approximate measure.
Note: The Code automatically accepts capacitance to be in uF. This is done so that the order magnitude of all the variables input to the NelderMead optimizer are similar.

```
12:42:02.671 -> Optimization task created!
12:42:02.752 -> Estimated Parameters:
12:42:02.752 -> Param 0: 19.84
12:42:02.752 -> Param 1: 251.20
12:42:02.752 -> Param 2: 40.19
12:42:02.752 -> Param 3: 149.16
12:42:02.752 -> Nelder-Mead optimization took: 41 milliseconds
```

## Acknowledgments

This project heavily leveraged AI-powered tools, such as ChatGPT, to assist with various aspects, including technical explanations, code optimization, and troubleshooting. The AI assistants were used to enhance the development process, offering insights and refining solutions to improve efficiency and accuracy. The assistance provided was crucial in enhancing the quality of the documentation and coding strategies.