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

// Bit-banged SPI (until simultaneous R/W is supported by the native API)

#define SD_CARD_MOSI_PIN	32
#define SD_CARD_MISO_PIN	33
#define SD_CARD_SCK_PIN		31
#define SD_CARD_CS_PIN		17								// Use 16 for slot 1.

// Software serial for GPS (see M4 RTApp)

// #define GPS_SOFT_TX			43								// Slot 2 AN pin, use 42 for slot 1
// #define GPS_SOFT_RX			1								// Slot 2 PWM pin, use 0 for slot 1

#define FW_VER			"0.2.2"

// 0.1.0: Base alpha FW.
// 0.1.1: SD card mounted, basic SD I/O.
// 0.1.2: SD card mounted as full FAT/exFAT file system.
// 0.1.3: SD card now works in bit-bang mode. File management successful.
// 0.1.4: First communication with the car's ECU.

// 0.2.0: First complete OBD data logger.
// 0.2.1: GPS logger. Needs testing on board.
// 0.2.2: GPS logging works, OBD needs testing again. File I/O over TCP ready for test with app.

// TARGET: 0.3: Target release, GPS and OBD logging is reliable and file transmission works (slow SPI).
// TARGET: 0.4: Real time transmission to app.

// In order of priority:

// TARGET: Native SPI. Fast I/O.
// TARGET: LED status.
// TARGET: Compass and advanced GPS parameters.
// TARGET: Use of onboard sensors.
// TARGET: Read MIL and DTCs, send notification to app.


// TIMER LIST

#define TIMER_OBD_UART			0
#define TIMER_OBD_UART_DURATION	5000