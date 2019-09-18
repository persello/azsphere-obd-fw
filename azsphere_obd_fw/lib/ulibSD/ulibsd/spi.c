
#define SPI_STRUCTS_VERSION 1

#include <errno.h>

#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/spi.h>

#include <string.h>

#include "../../../config.h"

int csfd, spifd;

void SPI_Soft_CS_Init(void) {
	if (!csfd) {
		csfd = GPIO_OpenAsOutput(SD_CARD_CS_PIN, GPIO_OutputMode_PushPull, GPIO_Value_Low);
		if (csfd <= 0) {
			Log_Debug("SPIIO: Error opening CS: %s.\n", strerror(errno));
		}
	}
}

void SPI_Init(void) {
	if (spifd <= 0) {
		SPIMaster_Config config;
		SPIMaster_InitConfig(&config);

		spifd = SPIMaster_Open(SD_CARD_SPI, 0, &config);
		if (spifd == -1) {
			Log_Debug("SPIIO: Error while opening SPI interface: %s.\n", strerror(errno));
		}

		SPIMaster_SetBusSpeed(spifd, SD_CARD_LOW_SPEED);

		SPI_Soft_CS_Init();
	}
}