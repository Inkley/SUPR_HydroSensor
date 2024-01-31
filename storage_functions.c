/*
 * storage_functions.c
 *
 *  Created on: Jan 8, 2024
 *      Author: Tyler
 */

// Standard C libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// Custom project-specific headers
#include "driverlib/i2c.h"

//*****************************************************************************/
// Erase a block of flash and program a few words on TM4C123x device
//*****************************************************************************/
void writeFlash_test1(void)
{
    uint32_t pui32Data[2];

    // Erase a block of the flash.
    FlashErase(0x800);

    // Program some data into the newly erased block of the flash.
    pui32Data[0] = 0x12345678;
    pui32Data[1] = 0x56789abc;
    FlashProgram(pui32Data, 0x800, sizeof(pui32Data));

}

//*****************************************************************************/
// Erase a block of flash and program a table on TM4C123x device
//*****************************************************************************/
void writeFlash_test2(void)
{
    #define ROWS 10
    #define COLS 3

    // Define a 2D array to represent the table
    uint32_t table[ROWS][COLS];

    // Populate the table with data starting from 1
    uint32_t value = 1;
    int i, j; // Declare variables here

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            table[i][j] = value++;
        }
    }

    // Erase a block of the flash.
    FlashErase(0x800);

    // Program the table into the newly erased block of the flash.
    FlashProgram((uint32_t *)table, 0x800, sizeof(table));
}

//*****************************************************************************/
// Calculates a checksum, appends it to a uint32_t array along with the input
// string, erases a 16K Flash block, and programs the array into Flash memory.
//*****************************************************************************/
void writeFlash_test3(const char *saveString)
{
    uint32_t chksum = 0;
    uint32_t pui32Buffer[260];  // Assuming pui32Buffer is a uint32_t array with a size of 260
    int loop;

    // Calculate checksum
    for (loop = 0; loop < strlen(saveString); loop++) {
        chksum += saveString[loop];
    }

    // Save checksum and comma
    pui32Buffer[0] = chksum;
    pui32Buffer[1] = ',';  // add comma

    // Save the string with null termination
    for (loop = 2; loop < strlen(saveString) + 4; loop++) {  // include null term
        pui32Buffer[loop] = saveString[loop - 2];
    }

    // Erase 16K block
    FlashErase(0x800);

    // Program data to Flash
    FlashProgram(pui32Buffer, 0x800, sizeof(pui32Buffer));
}

//*****************************************************************************/
// Read from Flash and Print to UART
//*****************************************************************************/
void readFlash_test1(void)
{
    uint32_t *ui32Ptr;
    char pucBuffer[256];  // Assuming pucBuffer is a character array of size 256
    int loop;

    // Allocate memory and read from Flash
    ui32Ptr = malloc(256 * sizeof(uint32_t));
    memcpy(ui32Ptr, (const void *)0x800, 256 * sizeof(uint32_t));

    // Copy data to character buffer and print to UART
    for (loop = 0; loop < 256; loop++) {
        pucBuffer[loop] = (char)ui32Ptr[loop];
    }

    // Print to UART
    UARTprintf("\nData read from Flash: %s\n", pucBuffer);

    // Free allocated memory
    free(ui32Ptr);
}

