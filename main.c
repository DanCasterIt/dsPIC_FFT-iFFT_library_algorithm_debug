/*
 * File:   main.c
 * Author: daniele
 *
 * Created on 17 giugno 2017, 20.21
 */

/* DSPIC33FJ128GP802 Configuration Bit Settings */
// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Mode (Internal Fast RC (FRC))
// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Source (Primary Oscillator Disabled)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)
// FWDT
#pragma config FWDTEN = OFF             // Watchdog Timer Enabled/disabled by user software
// (LPRC can be disabled by clearing SWDTEN bit in RCON register

#include "main.h"
#include <dsp.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include <stdio.h>
#include <p33FJ128GP802.h>
#include <stdint.h>

/* Global Definitions */
extern fractcomplex sigCmpx[BUFFER_LENGHT] 		/* Typically, the input signal to an FFT  */
__attribute__((space(ymemory),far,aligned(BUFFER_LENGHT*2*2)));

#if HANNING_WINDOW
fractional window[BUFFER_LENGHT] __attribute__((space(ymemory), aligned(BUFFER_LENGHT*2)));
#endif

#if FFTTWIDCOEFFS_IN_PROGMEM
fractcomplex twiddleFactors_FFT[BUFFER_LENGHT/2] 	/* Declare Twiddle Factor array in X-space*/
__attribute__ ((section (".xbss, bss, xmemory"), aligned (BUFFER_LENGHT*2)));
fractcomplex twiddleFactors_IFFT[BUFFER_LENGHT/2] 	/* Declare Twiddle Factor array in X-space*/
__attribute__ ((section (".xbss, bss, xmemory"), aligned (BUFFER_LENGHT*2)));
#else
extern const fractcomplex twiddleFactors_FFT[BUFFER_LENGHT/2]	/* Twiddle Factor array in Program memory */
__attribute__ ((space(auto_psv), aligned (BUFFER_LENGHT*2)));
extern const fractcomplex twiddleFactors_IFFT[BUFFER_LENGHT/2]	/* Twiddle Factor array in Program memory */
__attribute__ ((space(auto_psv), aligned (BUFFER_LENGHT*2)));
#endif

void oscillator_setup(void);
void uart_setup(void);
void process_1khz_square_wave(void);
void transform(void);

int	peakFrequencyBin = 0;				/* Declare post-FFT variables to compute the */
unsigned long peakFrequency = 0;			/* frequency of the largest spectral component */

int main(void) {
    oscillator_setup();
    uart_setup();
    printf("RESET\n\r");
#if HANNING_WINDOW
    printf("Hanning windowing enabled\n\r");
#endif
    printf("Twiddle Factors ");
#if FFTTWIDCOEFFS_IN_PROGMEM
    printf("in RAM X-space\n\r");
#else
    printf("in program memory\n\r");
#endif

#if FFTTWIDCOEFFS_IN_PROGMEM					/* Generate TwiddleFactor Coefficients */
	TwidFactorInit (BUFFER_LENGHT_LOG, &twiddleFactors_FFT[0], 0);	/* We need to do this only once at start-up */
	TwidFactorInit (BUFFER_LENGHT_LOG, &twiddleFactors_IFFT[0], 1);	/* We need to do this only once at start-up */
#endif
#if HANNING_WINDOW
    HanningInit(BUFFER_LENGHT, &window[0]);
#endif
    
    process_1khz_square_wave();
    transform();

    while (1);
}

void process_1khz_square_wave(void) {
    int i, e;
	for (i = BUFFER_LENGHT-1, e = ((BUFFER_LENGHT/2)-1); i > 0; i-=2, e--)	{
		sigCmpx[i].real = sigCmpx[e].real >> 1;
		sigCmpx[i].imag = 0;
		sigCmpx[i-1].real = sigCmpx[e-1].imag >> 1;
		sigCmpx[i-1].imag = 0;
	}
}

void transform() {
#if TRANSFORM_FUNCTION_NO_TEST
    int i;
    fractcomplex buff[BUFFER_LENGHT];
    for (i = 0; i < BUFFER_LENGHT; i++)  printf("A - [%03u].R = %04X = % 5d\n\r", i, sigCmpx[i].real, sigCmpx[i].real);
#endif
#if HANNING_WINDOW
    VectorWindow(BUFFER_LENGHT,&sigCmpx[BUFFER_LENGHT].real,&sigCmpx[BUFFER_LENGHT].real,(fractional*)&window[0]);
#endif
	/* Perform FFT operation */
#if FFTTWIDCOEFFS_IN_PROGMEM
	FFTComplexIP (BUFFER_LENGHT_LOG, &sigCmpx[0], &twiddleFactors_FFT[0], COEFFS_IN_DATA);
#else
	FFTComplexIP (BUFFER_LENGHT_LOG, &sigCmpx[0], (fractcomplex *) __builtin_psvoffset(&twiddleFactors_FFT[0]), (int) __builtin_psvpage(&twiddleFactors_FFT[0]));
#endif
    
	/* Store output samples in bit-reversed order of their addresses */
	BitReverseComplex (BUFFER_LENGHT_LOG, &sigCmpx[0]);
    
	/* Compute the square magnitude of the complex FFT output array so we have a Real output vetor */
	SquareMagnitudeCplx(BUFFER_LENGHT, &sigCmpx[0], &buff[0].real);

	/* Find the frequency Bin ( = index into the SigCmpx[] array) that has the largest energy*/
	/* i.e., the largest spectral component */
	VectorMax(BUFFER_LENGHT/2, &buff[0].real, &peakFrequencyBin);

	/* Compute the frequency (in Hz) of the largest spectral component */
	peakFrequency = peakFrequencyBin*(SAMPLING_RATE/BUFFER_LENGHT);
#if TRANSFORM_FUNCTION_NO_TEST
    for (i = 0; i < BUFFER_LENGHT; i++) printf("B - [%03u].R = %04X = % 5d\n\r", i, sigCmpx[i].real, sigCmpx[i].real);
#endif
	/* Perform IFFT operation */
#if FFTTWIDCOEFFS_IN_PROGMEM
	IFFTComplexIP (BUFFER_LENGHT_LOG, &sigCmpx[0], &twiddleFactors_IFFT[0], COEFFS_IN_DATA);
#else
	IFFTComplexIP (BUFFER_LENGHT_LOG, &sigCmpx[0], (fractcomplex *) __builtin_psvoffset(&twiddleFactors_IFFT[0]), (int) __builtin_psvpage(&twiddleFactors_IFFT[0]));
#endif
    
    //FFTComplexIP() scales the signal by 1/N. This N has to be re-multiplied.
    for (i = 0; i < BUFFER_LENGHT; i++)  sigCmpx[i].real = sigCmpx[i].real * BUFFER_LENGHT;
    
#if TRANSFORM_FUNCTION_NO_TEST
    for (i = 0; i < BUFFER_LENGHT; i++)  printf("C - [%03u].R = %04X = % 5d\n\r", i, sigCmpx[i].real, sigCmpx[i].real);
    printf("\n\rpeakFrequencyBin = %d\n\r", peakFrequencyBin);
    printf("peakFrequency = %d\n\r", (int)peakFrequency);
#endif
}

void oscillator_setup(void) {
    // Configure PLL prescaler, PLL postscaler, PLL divisor
    PLLFBD = 41; // M = 43
    CLKDIVbits.PLLPOST = 0; // N2 = 2
    CLKDIVbits.PLLPRE = 0; // N1 = 2
    OSCTUN = 0; // Tune FRC oscillator, if FRC is used
    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;
    // Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b001);
    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1) {
    };
}

void uart_setup(void) {
    AD1PCFGL = 0xFFFF; //all pins as digital
    TRISBbits.TRISB3 = 0; // TX as output
    TRISBbits.TRISB2 = 1; // RX as input

    RPINR18bits.U1RXR = 2; //U1RX on RP2 pin
    RPOR1bits.RP3R = 0b00011; //U1TX on RP3 pin

    U1MODEbits.STSEL = 0; // 1-stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 0; // Standard-Speed mode
    U1BRG = BRGVAL; // Baud Rate setting for 9600
    U1MODEbits.UARTEN = 1; // Enable 
    U1STAbits.UTXEN = 1; // Enable UART TX
    __C30_UART = 1;
}