/*
 * adc_functions.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Tyler
 */

// Standard C libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Custom project-specific headers
#include "uart_functions.h"

// Tiva C Series libraries
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "inc/hw_adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_udma.h"
#include "utils/uartstdio.h"

//*****************************************************************************/
// Configure ADC0 for differential sampling, Trigger Timer - 1 kHz
//*****************************************************************************/
void
configureADC1(void)
{
    // Set the clocking to run at 20 MHz (200 MHz / 10) using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source. System clock is set to use the PLL with a division factor of 10,
    // and the external crystal oscillator is set to 16 MHz.
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Enable necessary peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);   // Enable the clock for Timer 0 peripheral.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);     // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);    // GPIO port E needs to be enabled so these pins can be used.

    // Configure Timer 0 as a 16-bit periodic timer for ADC triggering at 1 kHz.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);

    // Set the timer load value for a 1 kHz sampling frequency.
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()/1000) - 1);

    // Enable the ADC trigger output for Timer A.
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    // Enable processor interrupts.
    IntMasterEnable();

    // Enable Timer 0 to start ADC triggering.
    TimerEnable(TIMER0_BASE, TIMER_A);

    // Select the analog ADC function for AIN0(PE3) and AIN1(PE2).
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2);

    // Use ADC0 sequence 0 to sample channel 0 once for each timer period.
    //ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_HALF, 1);

    // Read the current ADC clock configuration.
    //uint32_t ui32Config, ui32ClockDiv;
    //ui32Config = ADCClockConfigGet(ADC0_BASE, &ui32ClockDiv);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3. SS3: Number of Samples = 1, Depth of FIFO = 1
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Enable Timer A as the trigger source for ADC sequence 3.
    //ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);     // Change to this line for fs = 1kHz

    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // differential mode (ADC_CTL_D) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.   For more
    // information on the ADC sequences and steps, refer to the datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_D | ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag before starting the ADC sequence. This
    // ensures that there are no stale or previous interrupt flags set that could
    // interfere with the correct operation of the ADC
    ADCIntClear(ADC0_BASE, 3);

    // Display the setup on the console.
    UARTprintf("ADC ->\n");
    UARTprintf("    Type:           Differential\n");
    UARTprintf("    Input Pins:     AIN0/PE3 - AIN1/PE2\n");
    UARTprintf("    System Clock:   %d MHz\n", SysCtlClockGet() / 1000000);
    //UARTprintf("    ADC Clock:      %d Hz\n\n", ui32Config);
}

//*****************************************************************************/
// Perform ADC sampling and data acquisition
//*****************************************************************************/
int
startADC1(void)
{
    // Array for storing data read from ADC FIFO
    uint32_t pui32ADC0Value[1];

    // Add a loop counter
    uint32_t loopCounter = 0;

    // Define sample variables and set a maximum number of samples
    #define MAX_SAMPLE_NUM 1000
    uint32_t sample_num;

    // Call user input function for the number of samples
    sample_num = getUserInput();

    // Validate user input
    if (sample_num <= 0 || sample_num > MAX_SAMPLE_NUM) {
        UARTprintf("Invalid number of samples. Exiting.\n");
        return 1;
    }

    // Add this variable to represent the text file
    FILE *file;

    // Open the file for writing
    file = fopen("/Users/Tyler/Desktop/RESEARCH/1 Sensor Module/Electronics/adc_data.txt", "w+");

    // Check if the file was opened successfully
    if (file == NULL) {
        UARTprintf("Error opening file for writing.\n");
        return 1; // Exit the program with an error code
    }

    // Turn on the blue LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

    // Loop for the specified number of samples
    for (loopCounter = 0; loopCounter < sample_num; loopCounter++)
    {
        // Causes a processor trigger for a sample sequence
        ADCProcessorTrigger(ADC0_BASE, 3);

        // Wait until the sample sequence has completed
        while (!ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        // Used after the ADC conversion is complete to clear the interrupt status flag.
        // This is necessary to acknowledge that the ADC conversion has been processed,
        // and the ADC is ready for the next trigger.
        ADCIntClear(ADC0_BASE, 3);

        // Read ADC Value
        ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);

        // Display the [AIN0(PE3) - AIN1(PE2)] digital value on the console
        UARTprintf("\nLoop # = %d, Timestamp = %d, AIN0 - AIN1 = %4d\r", loopCounter + 1, clock(), pui32ADC0Value[0]);

        // Write the ADC value to the file and flush the buffer
        fprintf(file, "%d\t%d\t%4d\n", loopCounter + 1, clock(), pui32ADC0Value[0]);
        fflush(file);
    }

    // Turn off the blue LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

    // Success Statement
    UARTprintf("\n\nSampling Completed");

    // Close the file before exiting
    fclose(file);
    return 0;
}
