/*
 *  File: spi_io.c.example
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#define SPI_STRUCTS_VERSION 1

#include <errno.h>

#include <applibs/spi.h>
#include <applibs/gpio.h>
#include <applibs/log.h>

#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "../../../config.h"

#include "spi_io.h"

 /******************************************************************************
  Module Public Functions - Low level SPI control functions
 ******************************************************************************/

SPIMaster_Config config;
int SPIfd = -1;
int csfd = -1;

void SPI_Init(void) {
	Log_Debug("SPIIO: Initializing SPI bus for SD card.\n");
	if (SPIMaster_InitConfig(&config) != 0) {
		Log_Debug("SPIIO: Unable to initialize SPI configuration. Details: \"%s\".\n", strerror(errno));
	}

	config.csPolarity = SPI_ChipSelectPolarity_ActiveLow;

	// Open the SPI bus
	SPIfd = SPIMaster_Open(SD_CARD_SPI, SD_CARD_CS_NAME, &config);
	if (SPIfd == -1) {
		Log_Debug("SPIIO: Error while opening SPI file descriptor. Details: \"%s\".\n", strerror(errno));
	}
	else {
		Log_Debug("SPIIO: File descriptor for SPI (%d) created successfully.\n", SPIfd);
	}

	// Override the control for the CS pin
	if (csfd == -1) {
		csfd = GPIO_OpenAsOutput(SD_CARD_CS_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (csfd < 0) {
			Log_Debug("SPIIO: Error opening CS: %s.\n", strerror(errno));
		}
	}
}

// The function SPI_RW has been removed in favour of two separate functions
// This is due to the fact that full-duplex communication is not supported
BYTE SPI_Read() {

	BYTE result;
	SPIMaster_Transfer transfer[1];

	SPIMaster_InitTransfers(transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Read;
	transfer[0].length = 1;
	transfer[0].readData = &result;

	SPIMaster_TransferSequential(SPIfd, transfer, 1);

	return result;
}

void SPI_Write(BYTE d) {
	SPIMaster_Transfer transfer[1];

	SPIMaster_InitTransfers(transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Write;
	transfer[0].length = 1;
	transfer[0].writeData = &d;

	SPIMaster_TransferSequential(SPIfd, transfer, 1);
}

// These two functions are built in order to leave the MISO line high between bytes
void SPI_SendCommand(BYTE data[6]) {
	SPIMaster_Transfer transfer[1];

	SPIMaster_InitTransfers(transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Write;
	transfer[0].length = 6;
	transfer[0].writeData = data;

	SPIMaster_TransferSequential(SPIfd, transfer, 1);
}

// Same as before, but this simply puts the MISO line high and gives 80 clock cycles on SCK
// Minimun 74 cycles for entering SPI mode
void SPI_80Clocks() {
	SPIMaster_Transfer transfer[1];

	SPIMaster_InitTransfers(transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Write;
	transfer[0].length = 10;

	char data[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	transfer[0].writeData = &data;

	SPIMaster_TransferSequential(SPIfd, transfer, 1);
}

void SPI_Release(void) {
	// Maybe not? Needs testing.
	//close(SPIfd);
}

// The CS line we use here is an external CS pin (not the ISU one) because independent CS control is not allowed
inline void SPI_CS_Low(void) {
	if (GPIO_SetValue(csfd, GPIO_Value_Low) != 0) {
		Log_Debug("SPIIO: Cannot pull down CS line. Details: \"%s\".\n", strerror(errno));
	}
}

inline void SPI_CS_High(void) {
	if (GPIO_SetValue(csfd, GPIO_Value_High) != 0) {
		Log_Debug("SPIIO: Cannot pull up CS line. Details: \"%s\".\n", strerror(errno));
	}
}

inline void SPI_Freq_High(void) {
	SPIMaster_SetBusSpeed(SPIfd, SD_CARD_SPEED);
}

inline void SPI_Freq_Low(void) {
	SPIMaster_SetBusSpeed(SPIfd, 400000);
}

unsigned long long millis() {
	struct timeval time;
	gettimeofday(&time, NULL);
	unsigned long long val =
		(unsigned long long)(time.tv_sec) * 1000 +
		(unsigned long long)(time.tv_usec) / 1000;
	return val;
}

long period = -1;
unsigned long long started;
void SPI_Timer_On(WORD ms) {
	period = ms;
	started = millis();
}


inline BOOL SPI_Timer_Status(void) {

	// Timer is enabled
	if (period == -1) {
		return TRUE;
	}
	else {

		// Timer has expired
		if (millis() > started + period) {
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
}

inline void SPI_Timer_Off(void) {
	period = -1;
}

#ifdef SPI_DEBUG_OSC
inline void SPI_Debug_Init(void)
{

}
inline void SPI_Debug_Mark(void)
{

}
#endif

/*
8/13/2019: This example file has been re-used and edited by Riccardo Persello
in order to use this library with an Azure Sphere MCU (MT3620)
*/

/*
The MIT License (MIT)

Copyright (c) 2015 Nelson Lombardo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
