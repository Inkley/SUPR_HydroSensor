/*
 * data_transfer_functions.c
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
#include "driverlib/udma.h"

//*****************************************************************************
// The interrupt handler for uDMA errors.  This interrupt will occur if the
// uDMA encounters a bus error while trying to perform a transfer.  This
// handler just increments a counter if an error occurs.
//*****************************************************************************
void
uDMAErrorHandler(void)
{
    uint32_t ui32Status;

    // uDMA errors count.  This value is incremented by the uDMA error handler.
    static uint32_t g_ui32DMAErrCount = 0u;

    // Check for uDMA error bit.
    ui32Status = uDMAErrorStatusGet();

    // If there is a uDMA error, then clear the error and increment
    // the error counter.
    if(ui32Status)
    {
        uDMAErrorStatusClear();
        g_ui32DMAErrCount++;
    }
}
