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

The following UART signals are configured for displaying console messages
    UART0 peripheral
    GPIO Port A peripheral (for UART0 pins)
    UART0RX - PA0
    UART0TX - PA1
*/

// Standard C libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Tiva C Series libraries
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/flash.h"
#include "driverlib/eeprom.h"

// Custom project-specific headers
#include "utils/uartstdio.h"
#include "driverlib/i2c.h"
#include "uart_functions.h"
#include "adc_functions.h"
#include "storage_functions.h"

int main(void)
{
    // Set up the serial console
    initConsole();

    // Configure ADC0 for differential sampling
    configureADC();

    // Perform ADC sampling and data acquisition
    getADC();

    // FLASH STORAGE TESTS
        // Erase a block of flash and program a few words on TM4C123x device
        //writeFlash_test1();

        // Erase a block of flash and program a table on TM4C123x device
        //writeFlash_test2();

        // Calculates a checksum, appends it to a uint32_t array along with the input
        // string, erases a 16K Flash block, and programs the array into Flash memory.
        //writeFlash_test3();

        // Read Flash Test
        //readFlash_test1();

}
