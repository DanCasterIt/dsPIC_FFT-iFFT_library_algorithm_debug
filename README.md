# dsPIC_FFT-iFFT_library_algorithm_debug

This is a signal playback project: It simply transforms a signal using the FFT and inverse transforms it using the iFFT.

Tested with MPLAB X v5.15 and XC16 v1.36 on a dsPIC33FJ128GP802

This code is directly derived from the Microchip's "CE018 - Using the Fast Fourier Transform (FFT) for Frequency Detection" with few modification:

1) The processing algorithm of the input signal has been modified to make it more understandable (process_1khz_square_wave() function)
2) The output is directly printed through the serial port
3) The iFFT function has been added
4) Hanning function has been added

This project runs and compiles properly with modern versions of MPLAB X and XC16 compilers.

## How to compile and run

1) Clone this project
2) Rename this project's folder from "dsPIC_FFT-iFFT_library_algorithm_debug" to "dsPIC_FFT-iFFT_library_algorithm_debug.X"
3) Open it with MPLAB X
4) If it does not compile already, right-click on the project name -> properties -> Conf:[default] -> XC16-> xc16-ld -> in "Additional options:" put the string: "--library "dsp""

## Workaround for TwidFactorInit() function

TwidFactorInit() works properly ONLY if I write a value into TRISB register or if I call a printf() function. Still trying to find out the reason why of this behaviour