//*****************************************************************************
//  PROJECT DESCRIPTION
/*
Tyler Inkley
Hydrodynamic Sensor Module - Sampling Protocol

Setup ADC0 as a differential input and take samples between AIN0 and AIN1.

The value of the ADC is read and printed to the serial port.

Uses the following peripherals and I/O signals:
    ADC0 peripheral
    GPIO Port E peripheral (for ADC0 pins)
    AIN0 - PE3
    AIN1 - PE2

The following UART signals are configured only for displaying console messages
*** These are NOT required for operation of the ADC ***
    UART0 peripheral
    GPIO Port A peripheral (for UART0 pins)
    UART0RX - PA0
    UART0TX - PA1

*/
//*****************************************************************************

// Header Files
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include <sys/_timeval.h>
#include <time.h>
#include "driverlib/i2c.h"

// The error routine that is called if the driver library encounters an error
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// This function sets up UART0 to be used for a console to display information as the example is running.
void
InitConsole(void)
{
    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // GPIO Port (*A*, B, C, D, E, F)

    // Configure the pin muxing for UART0 functions on port A0 and A1.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Enable UART0 so that we can configure the clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

// Configure ADC0 for differential input samples
int
main(void)
{

#if defined(TARGET_IS_TM4C123_RA0) ||                                       \
    defined(TARGET_IS_TM4C123_RA1) ||                                       \
    defined(TARGET_IS_TM4C123_RA2)
    uint32_t ui32SysClock;
#endif

    // Alternative write to text file method
    FILE *file;

    file = fopen("adc_data.txt", "w");
    if (file == NULL)
    {
        // Handle file open error
        // You can add error handling here (e.g., print an error message)
    }

    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use.  This example
    // uses sequence 3 which has a FIFO depth of 1.  If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    uint32_t pui32ADC0Value[1];

    // create a time stamp for the logging 
    time_t timeStamp;
    time_t timeStart;

    // Set the clocking to run at 20 MHz (200 MHz / 10) using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source.
    #if defined(TARGET_IS_TM4C123_RA0)  ||                                  \
    defined(TARGET_IS_TM4C123_RA1)      ||                                  \
    defined(TARGET_IS_TM4C123_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_480), 20000000);
#else
    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |SYSCTL_XTAL_16MHZ);
#endif

    // Set up the serial console to use for displaying messages
    InitConsole();

    // Display the setup on the console.
    UARTprintf("ADC ->\n");
    UARTprintf("  Type: differential\n");
    UARTprintf("  Samples: One\n");
    UARTprintf("  Update Rate: 250ms\n");
    UARTprintf("  Input Pin: (AIN0/PE3 - AIN1/PE2)\n\n");

    // The ADC0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // ADC0 is used with AIN0 on port E7 ((GPIO port E needs to be enabled so these pins can be used)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Select the analog ADC function for these pins.
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This is arbitrarily using sequence 3.
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

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    ADCIntClear(ADC0_BASE, 3);

    // Sample AIN0 forever.  Display the value on the console.
    timeStart = time(NULL);

    // Enable GPIO port F which is used for the blue LED.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Configure the blue LED pin (PF2) as an output.
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    // Initially turn off the blue LED.
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

    while(1)
    {
        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 3);

        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        // Clear the ADC interrupt flag.
        ADCIntClear(ADC0_BASE, 3);

        // Read ADC Value.
        timeStamp = time(NULL);
        ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);

        // Calculate output voltage
        float voltage = (float)pui32ADC0Value[0] / 4096.0 * 3.3;  // Assuming 12-bit ADC and 3.3V reference voltage

        // Write ADC data to the file
        //TODO: Why is the line below stopping the UART printout? Also - how can I get the textfile off the MCU to my local machine?
        //fprintf(file, "%4d,%f,%d\n", pui32ADC0Value[0], voltage, timeStamp - timeStart);

        // Display the AIN0 (PE3) digital value on the console. Divide by 2^12 (12-bit ADC Resolution) to get output voltage.
        UARTprintf("%4d,%d\n", pui32ADC0Value[0], timeStamp - timeStart);

        // Blink the blue LED.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        SysCtlDelay(SysCtlClockGet() / 10 / 3);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
        SysCtlDelay(SysCtlClockGet() / 10 / 3);

        // This function provides a means of generating a constant length
        // delay.  The function delay (in cycles) = 3 * parameter.  Delay
        // 250ms arbitrarily.

#if defined(TARGET_IS_TM4C123_RA0) ||                                         \
    defined(TARGET_IS_TM4C123_RA1) ||                                         \
    defined(TARGET_IS_TM4C123_RA2)
        SysCtlDelay(ui32SysClock / 12);
#else
        SysCtlDelay(SysCtlClockGet() / 12);
#endif

        fclose(file);
    }
}
