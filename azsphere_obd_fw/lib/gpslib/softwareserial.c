#include "softwareserial.h"

#include "../timer/Inc/Public/timer.h"

#include <errno.h>
#include <string.h>

#include <applibs/gpio.h>
#include <applibs/log.h>

int initializeSS(SoftwareSerial* s, int tx, int rx)
{

	// Save pin numbers
	s->tx = tx;
	s->rx = rx;

	// Try to open GPIOs
	s->txfd = GPIO_OpenAsOutput(tx, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (s->txfd == -1) {
		Log_Debug("SOFTWARESERIAL: Cannot open TX line on pin %d. Error: \"%s\" (%d).\n", tx, strerror(errno), errno);
		return -1;
	}

	s->rxfd = GPIO_OpenAsInput(rx);
	if (s->rxfd == -1) {
		Log_Debug("SOFTWARESERIAL: Cannot open RX line on pin %d. Error: \"%s\" (%d).\n", rx, strerror(errno), errno);
		return -1;
	}

	// Set the default baudrate
	s->br = 9600;

	// Initialize other variables
	s->currentBitRead = 0;
	s->lastTimeRead = 0;
	s->currentBitWrite = 0;
	s->lastTimeWrite = 0;
	initCircBuffer(&(s->rxBuffer), 4096);
	initCircBuffer(&(s->txBuffer), 512);

	return 0;
}

int setSSBaudRate(SoftwareSerial* s, int br)
{

	s->br = br;
	return 0;
}

int updateSS(SoftwareSerial* s)
{
	// Read (no return, otherwise it doesn't write)



	// Write

	// Time to write?
	unsigned long long timeDifference = (nanos() - s->lastTimeWrite);
	if (timeDifference > 1000000000UL / s->br) {
		int value;

		// Start bit
		if (s->currentBitWrite == 0) {

			// Loads a character, if the buffer is empty, stop.
			if (getCharBuffer(&(s->txBuffer), &(s->currentCharWrite)) == -1) {
				return -1;
			}

			value = 0;
		}
		// Stop bit
		else if (s->currentBitWrite == 9) {
			value = 1;
		}
		// Data
		else {
			value = (s->currentCharWrite & (1 << (s->currentBitWrite - 1))) > 0;
		}

		// Write current bit on pin
		GPIO_SetValue(s->txfd, value);

		// Immediately record the time
		s->lastTimeWrite += timeDifference;

		// Increment bit index (0 to 9)
		s->currentBitWrite++;

		if (s->currentBitWrite == 10) s->currentBitWrite = 0;
	}

	return 0;
}

int writeSS(SoftwareSerial* s, char* data)
{
	for (int i = 0; i < strlen(data); i++) {
		if (putCharBuffer(&(s->txBuffer), data[i]) == -1) {
			return -1;
		}
	}

	return 0;
}

int readStringSS(SoftwareSerial* s, char** data)
{
	return 0;
}
