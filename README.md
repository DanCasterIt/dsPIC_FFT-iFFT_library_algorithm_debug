# dsPIC_FFT-iFFT_library_algorithm_debug
This is an example code I wrote to try to understand why the Microchip XC16's iFFT library function introduces too much noise the output. It introduces so much noise that is not usable.

Tested with MPLAB X v5.15 and XC16 v1.36 on a dsPIC33FJ128GP802

This code is directly derived from the Microchip's "CE018 - Using the Fast Fourier Transform (FFT) for Frequency Detection" with few modification:

1) The processing algorithm of the input signal has been modified to make it more understandable (process_1khz_square_wave() function)
2) The output is directly printed through the serial port
3) the iFFT function has been added

This project runs and compiles properly with modern versions of MPLAB X and XC16 compilers.

## How to compile and run

clone this project folder, open it with MPLAB X, if it does not compile already, right-click on the project name -> properties -> Conf:[default] -> XC16-> xc16-ld -> in "Additional options:" put the string: "--library "dsp""

## Workaround for TwidFactorInit() function

TwidFactorInit() works properly ONLY if I write a value into TRISB register or if I call a printf() function. Still trying to find out the reason why of this behaviour