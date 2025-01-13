# Electrochemical Impedance Fitting Embedded

**Estimation of EIS Parameters using Randle Cell Model with Warburg, and Embedded C Implementation on ESP32.**  

## Aim
We want to use EIS to measure cell impedance and model cell parameters using Least Squares Estimation to pinpoint what component of the cell is affecting cell impedance.
If we are able to estimate cell paramerts in car, 

## Description

We simulate Electrochemical Impedance Spectroscopy (EIS) for a cell using the **Randles Cell Model**, and extend it to include the **Randle Cell Model with Warburg** coefficient.  
Cell parameter estimation is performed using **Least Squares Estimation** to estimate the cell parameters from simulated impedance measurements, based on an initial guess.  
The same cell model parameter estimation is replicated on an embedded platform (Arduino ESP32) using the **Nelder-Mead Method** of Least Squares. The results are matched, and the approximate performance is measured (Randle's model with Warburg).

## Modelled Parameters and Physical Interpretation

| **Parameter**        | **Physical Interpretation**                                                                                                                                                  | **Increase in Parameter**                                                                                               | **Decrease in Parameter**                                                                                              |
|----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------|
| **Rs (Solution Resistance)** | Resistance of the electrolyte between the electrodes.                                                                                                                   | - Low temperature (reduces ionic mobility).<br>- Electrolyte leakage (reduces ionic path).<br>- Contamination or electrolyte depletion (reduces ionic concentration). | - Higher temperature (increases ionic mobility).<br>- Improved electrolyte composition (more conductive).               |
| **Rp/Rct (Polarization Resistance)** | Resistance at the electrode-electrolyte interface, linked to charge transfer resistance.                                                                           | - Surface passivation (formation of insulating layers).<br>- Reduced catalyst activity (slower electrochemical reactions).<br>- Electrochemical degradation (corrosion or fouling of electrodes). | - Improved electrode surface (more active material).<br>- Higher temperature (increased reaction rates).               |
| **Cdl (Double Layer Capacitance)** | Capacitance of the electrochemical double layer at the electrode-electrolyte interface.                                                                           | - Larger electrode surface area (more active sites).<br>- Higher dielectric constant in electrolyte (improves charge storage). | - Electrode surface degradation (loss of active sites).<br>- Lower dielectric constant in electrolyte (reduced charge storage). |
| **Zw (Warburg Coefficient)** | Diffusion impedance, indicating how ion diffusion affects impedance.                                                                                                  | - Poor ion diffusion (thicker diffusion layers, lower temperature, or high-viscosity electrolytes).<br>- Electrode blockage (byproducts hindering diffusion). | - Improved ion mobility (better electrolyte composition, higher temperature).<br>- Thinner diffusion layer (optimized stirring or electrode design). |

## Cell model
Reference : https://upload.wikimedia.org/wikipedia/commons/thumb/9/93/Randles_circuit.png/220px-Randles_circuit.png
![Randles Circuit](media/Randles_circuit.png)
The simplified Randles model does not include Warberg impedance. 

## Matlab Simulation
We simulate impedance values referencing cell parameter values first using a simple Randles model ( No Warberg ), and then including Warberg.

Rs = 20 ohm
Rp = 250 ohm
Cdl = 40e uF
sigma = 150 ( warberg coeff.)

I have tried to match result in reference : https://www.gamry.com/application-notes/EIS/basics-of-electrochemical-impedance-spectroscopy/

### Simplifed model
[Simple model code Matlab](randles_simplified_model.m)
![Simplifed model](media/randles_simple_impedance.png)

### Randles with Warberg
[Warber added model code Matlab](randles_with_warberg.m)
![Randles with Warberg](media/randles_warberg_impedance.png)

## Cell parameter estimation
Simulate Electrochemical Impedance Spectroscopy (EIS) data using the Randle Cell model with Warburg impedance. We use a freqeuncy sweep from 0.1 to 10khz AC stimulus and impedance measuremnt (10 measurements logspace spread).
Define true values for the model parameters: Rs (Solution Resistance), Rp (Polarization Resistance), Cdl (Double Layer Capacitance), and sigma (Warburg Coefficient).
Generate noisy simulated impedance data for fitting.
Use Least Squares Fitting (via fminsearch) to estimate the model parameters by minimizing the difference between noisy data and model predictions.
The estimation process adjusts the parameters to best match the simulated data.
Output estimated values for Rs, Rp, Cdl, and sigma after optimization.


[lsqnonlin method MATLAB](lss_estimation_lsqnonlin.m)

Estimated Parameters:
Rs: 19.928
Rp: 248.421
Cdl: 3.9958e-05
Sigma: 151.7565

[NelderMead method MATLAB](lss_estimation_NelderMead.m)

Estimated Parameters:
Rs: 21.7962
Rp: 270.9082
Cdl: 4.0415e-05
Sigma: 147.6066

There will be run to run variance due to artificially added noise to impedance measurments. This is done on purpose, measurements arent always perfect.

## Embedded platform