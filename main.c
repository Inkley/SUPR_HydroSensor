/*
Tyler J. Inkley
Hydrodynamic Sensor Module Sampling

Setup ADC0 as a differential input and take samples between AIN0 and AIN1.
ADC data is read and printed to the serial port and a text file.

Uses the following peripherals and I/O signals:
    ADC0 peripheral
    GPIO Port E peripheral (for ADC0 pins)
    AIN0 - PE3
    AIN1 - PE2

The following UART signals are configured for displaying console messages:
    UART0 peripheral
    GPIO Port A peripheral (for UART0 pins)
    UART0RX - PA0
    UART0TX - PA1
*/

// Standard C libraries
#include <stdint.h>

// Custom project-specific headers
#include "adc_functions.h"
#include "data_transfer_functions.h"
#include "uart_functions.h"

int main(void)
{
    // Sets up UART0 to display information to console
    configureUART();

    // Configure ADC0 for differential sampling, Trigger Timer - 1 kHz
    configureADC1();

    // Perform ADC sampling and data acquisition
    startADC1();
}
