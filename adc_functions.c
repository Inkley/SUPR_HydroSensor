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
#include "uart_functions.h"

//*****************************************************************************/
// Configure ADC0 for differential sampling
//*****************************************************************************/
void configureADC(void)
{
    // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Set the clocking to run at 20 MHz (200 MHz / 10) using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source. System clock is set to use the PLL with a division factor of 10,
    // and the external crystal oscillator is set to 16 MHz.
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // GPIO port E needs to be enabled so these pins can be used.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Select the analog ADC function for these pins.
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3. SS3: Number of Samples = 1, Depth of FIFO = 1
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

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

    // Configure the ADC to use PLL at 480 MHz divided by 24 to get an ADC clock of 20 MHz.
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 24);

    // Display the setup on the console.
    UARTprintf("ADC ->\n");
    UARTprintf("    Type: Differential\n");
    UARTprintf("    Input Pin: (AIN0/PE3 - AIN1/PE2)\n");
    UARTprintf("    System Clock: %d MHz\n", SysCtlClockGet() / 1000000);
    UARTprintf("    Delay: %d CPU ticks\n\n", SysCtlClockGet() / 1000);

    // Read the current ADC clock configuration.
    uint32_t ui32Config, ui32ClockDiv;
    ui32Config = ADCClockConfigGet(ADC0_BASE, &ui32ClockDiv);
    UARTprintf("ADC Clock Configuration: %d Hz\n\n", ui32Config);
}

//*****************************************************************************/
// Perform ADC sampling and data acquisition
//*****************************************************************************/
int getADC(void)
{
    // Array for storing  data read from  ADC FIFO
    uint32_t pui32ADC0Value[0];
    //TODO: Change to below format (move away from single element array?)
    //uint32_t pui32ADC0Value[SIZE];

    // Add a loop counter
    uint32_t loopCounter = 0;

    // Define sample variables and set a maximum number of samples
    #define MAX_SAMPLE_NUM 1000
    uint32_t sample_num;

    // Call user input function for number of samples
    sample_num = GetUserInput();

    // Validate user input
    if (sample_num <= 0 || sample_num > MAX_SAMPLE_NUM) {
        UARTprintf("Invalid number of samples. Exiting.\n");
        return 1;
    }

    // Set the limit for the number of loops
    const uint32_t LOOP_LIMIT = loopCounter + sample_num;

    // Add this variable to represent the text file
    FILE * file;

    // Open the file for writing
    file = fopen("/Users/Tyler/Desktop/RESEARCH/1 Sensor Module/Electronics/adc_data.txt", "w+");

    // Check if the file was opened successfully
    if (file == NULL) {
        UARTprintf("Error opening file for writing.\n");
        return 1; // Exit the program with an error code
    }

    while(1)
    {
        // Turn on the blue LED
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

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

        // Display the [AIN0(PE3) -  AIN1(PE2)] digital value on the console
        // UART is a time based peripheral - and depends on a particular input clock rate
        UARTprintf("\nLoop # = %d, Timestamp = %d, AIN0 - AIN1 = %4d\r", loopCounter+1, clock(), pui32ADC0Value[0]);

        // TODO: Try to get this method to work below - Abby had something working before, ask if she still has file perhaps?
        //time_t seconds;
        //seconds = time(NULL);
        //UARTprintf("\nLoop # = %d, Timestamp = %d, AIN0 - AIN1 = %4d\r", loopCounter+1, seconds, pui32ADC0Value[0]);

        // Delay loop after each ADC conversion
        SysCtlDelay(SysCtlClockGet() / 1000);

        // Increment the loop counter
        loopCounter++;

        // Write the ADC value to the file and flush the buffer
        fprintf(file, "%d\t%d\t%4d\n", loopCounter, clock(), pui32ADC0Value[0]);
        fflush(file);

        // Check for an exit condition
        // TODO: Change exit condition to exceedance of timestamp value (vs. loop limit)
        if (loopCounter >= LOOP_LIMIT) {
            break;  // Exit the loop and program
            }
    }
    // Turn off the blue LED
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

    // Succes Statement
    UARTprintf("\n\nSampling Completed");

    // Close the file before exiting
    fclose(file);
    return(0);
}
