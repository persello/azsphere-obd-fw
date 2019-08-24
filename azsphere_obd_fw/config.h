#pragma once

#define UART_STRUCTS_VERSION 1

#include <soc/mt3620_uarts.h>
#include <soc/mt3620_spis.h>

#define PIN_LED_APP		4
#define PIN_LED_WLAN	5

#define PIN_LED_RED		8
#define PIN_LED_GREEN	9
#define PIN_LED_BLUE	10

#define PIN_BTN_A		12
#define PIN_BTN_B		13

#define INTERFACE_NAME	"wlan0"
#define PORT_NUMBER		15500

#define OBD_SERIAL		MT3620_UART_ISU0
#define OBD_INITIAL_BR	9600

#define SD_CARD_SPI		MT3620_SPI_ISU1
#define SD_CARD_CS_PIN	17								// Use 16 for slot 1. This is the real CS pin.
#define SD_CARD_CS_NAME	MT3620_SPI_CHIP_SELECT_B		// Use MT3620_SPI_CHIP_SELECT_A for slot 1, not used
#define SD_CARD_SPEED	20000000						// Based on your SD card's maximum speed

#define FW_VER			"0.1.1"

// 0.1.0: Base alpha FW
// 0.1.1: SD card mounted, basic SD I/O
