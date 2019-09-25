
#define SPI_STRUCTS_VERSION 1

#include <errno.h>

#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/spi.h>

#include <string.h>

#include "../../../config.h"

int csfd, auxfd, spifd;

inline void SPI_CS_Set(unsigned char s) {
	GPIO_SetValue(csfd, s);
}

void SPI_Soft_CS_Init(void) {
	if (!csfd) {
		csfd = GPIO_OpenAsOutput(SD_CARD_CS_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (csfd <= 0) {
			Log_Debug("SPIIO: Error opening CS: %s.\n", strerror(errno));
		}
	}
}

void SPI_Aux_MOSI_Init(void) {
	if (!auxfd) {
		auxfd = GPIO_OpenAsOutput(SD_CARD_MOSI_AUX, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (auxfd <= 0) {
			Log_Debug("SPIIO: Error opening auxiliary MOSI: %s.\n", strerror(errno));
		}
	}
}

void SPI_SCK_Slow(void) {
	if (spifd > 0) {
		SPIMaster_SetBusSpeed(spifd, SD_CARD_LOW_SPEED);
	}
	else {
		Log_Debug("SPIIO: Cannot set SPI SCK speed to low if spifd is invalid.");
	}
}

void SPI_SCK_Fast(void) {
	if (spifd > 0) {
		SPIMaster_SetBusSpeed(spifd, SD_CARD_HIGH_SPEED);
	}
	else {
		Log_Debug("SPIIO: Cannot set SPI SCK speed to high if spifd is invalid.");
	}
}

void SPI_Send(const char* buf, unsigned int bytes) {
	SPIMaster_Transfer transfer[1];
	SPIMaster_InitTransfers(transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Write;
	transfer[0].length = bytes;
	transfer[0].writeData = buf;

	SPIMaster_TransferSequential(spifd, &transfer, 1);
}

void SPI_Receive(const char** buf, unsigned int bytes) {
	SPIMaster_Transfer transfer[1];
	SPIMaster_InitTransfers(&transfer, 1);

	transfer[0].flags = SPI_TransferFlags_Read;
	transfer[0].length = bytes;
	transfer[0].readData = *buf;

	// MOSI high
	GPIO_SetValue(auxfd, GPIO_Value_High);

	SPIMaster_TransferSequential(spifd, &transfer, 1);

	// MOSI low
	GPIO_SetValue(auxfd, GPIO_Value_Low);
}

void SPI_Init(void) {
	if (spifd <= 0) {
		SPIMaster_Config config;
		SPIMaster_InitConfig(&config);
		config.csPolarity = SPI_ChipSelectPolarity_ActiveLow;

		spifd = SPIMaster_Open(SD_CARD_SPI, -1, &config);
		if (spifd == -1) {
			Log_Debug("SPIIO: Error while opening SPI interface: %s.\n", strerror(errno));
		}
		else {
			SPI_SCK_Slow();
			SPI_Soft_CS_Init();
			SPI_Aux_MOSI_Init();
		}
	}
}