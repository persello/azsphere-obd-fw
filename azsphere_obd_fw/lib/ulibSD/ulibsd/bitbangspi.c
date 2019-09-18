/*
 *  File: spi_io.c.example
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#define SPI_STRUCTS_VERSION 1

#include <errno.h>

#include <applibs/gpio.h>
#include <applibs/log.h>

#include <string.h>

#include "../../../config.h"

#include "bitbangspi.h"

 /******************************************************************************
  Module Public Functions - Low level SPI control functions
 ******************************************************************************/


int mosifd = 0;
int misofd = 0;
int sckfd = 0;
int csfd = 0;

void SPI_MOSI_Init(void) {
	if (!mosifd) {
		mosifd = GPIO_OpenAsOutput(SD_CARD_MOSI_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (mosifd <= 0) {
			Log_Debug("SPIIO: Error opening MOSI: %s.\n", strerror(errno));
		}
	}
}

void SPI_MISO_Init(void) {
	if (!misofd) {
		misofd = GPIO_OpenAsInput(SD_CARD_MISO_PIN);
		if (misofd <= 0) {
			Log_Debug("SPIIO: Error opening MISO: %s.\n", strerror(errno));
		}
	}
}

void SPI_SCK_Init(void) {
	if (!sckfd) {
		sckfd = GPIO_OpenAsOutput(SD_CARD_SCK_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (sckfd <= 0) {
			Log_Debug("SPIIO: Error opening SCK: %s.\n", strerror(errno));
		}
	}
}

void SPI_CS_Init(void) {
	if (!csfd) {
		csfd = GPIO_OpenAsOutput(SD_CARD_CS_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (csfd <= 0) {
			Log_Debug("SPIIO: Error opening CS: %s.\n", strerror(errno));
		}
	}
}



inline void SPI_MOSI_Set(unsigned char s) {
	GPIO_SetValue(mosifd, s);
}



inline void SPI_SCK_Set(unsigned char s) {
	GPIO_SetValue(sckfd, s);
}



inline void SPI_CS_Set(unsigned char s) {
	GPIO_SetValue(csfd, s);
}



inline unsigned char SPI_MISO_Get(void) {

	GPIO_Value_Type result;
	GPIO_GetValue(misofd, &result);

	return result;
}