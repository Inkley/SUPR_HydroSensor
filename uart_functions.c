/*
 * uart_functions.c
 *
 *  Created on: Nov 14, 2023
 *      Author: Tyler
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
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// Custom project-specific headers (if any)
#include "driverlib/i2c.h"

//*****************************************************************************/
// Sets up UART0 to display information to console
//*****************************************************************************/
void initConsole(void)
{
        // Enable GPIO port A which is used for UART0 pins
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

        // Configure the pin muxing for UART0 functions on port A0 and A1
        GPIOPinConfigure(GPIO_PA0_U0RX);
        GPIOPinConfigure(GPIO_PA1_U0TX);

        // Enable UART0 so that we can configure the clock
        SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

        // Use the internal 16MHz oscillator as the UART clock source
        UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

        // Select the alternate (UART) function for these pins
        GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

        // Initialize the UART for console I/O
        UARTStdioConfig(0, 115200, 16000000);

        // Enable the GPIO port for the blue LED
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

        // Set the blue LED pin as an output
        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

        // Turn off the blue LED initially
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
}

//*****************************************************************************/
// User input function (uart polled)
//*****************************************************************************/
uint32_t GetUserInput(void)
{
    // Buffer to store user input as a string
    char userInput[16];

    // Variable to store the converted number of samples
    uint32_t numSamples;

    // Prompt the user to enter the number of samples
    UARTprintf("Enter the number of samples: ");

    // Read user input as a string using UART
    UARTgets(userInput, sizeof(userInput));

    // Convert the string to an integer
    numSamples = atoi(userInput);

    // Return the number of samples entered by the user
    return numSamples;
}
