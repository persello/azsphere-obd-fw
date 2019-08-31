#include "softwareserial.h"

#include "../timer/Inc/Public/timer.h"

#include <errno.h>
#include <malloc.h>
#include <string.h>

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
	// Read (no return, otherwise it won't write)

	// Read RX line
	GPIO_Value_Type reading;
	if (GPIO_GetValue(s->rxfd, &reading) == -1) {
		Log_Debug("SOFTWARESERIAL: Cannot read RX line. Error: \"%s\" (%d).\n", strerror(errno), errno);

		// Fatal error. No need to write anything.
		return -1;
	}

	// Waiting for start bit
	if (s->currentBitRead == 0) {

		// Falling edge
		if (reading == GPIO_Value_Low && s->lastGPIOStatus == GPIO_Value_High) {
			// Start bit OK
			s->currentBitRead = 1;

			// Sync
			s->lastTimeRead = nanos();
		}

		s->lastGPIOStatus = reading;

	}
	// After the start bit, start reading data
	else {

		// Time to read?
		unsigned long long timeDifferenceRX = (nanos() - s->lastTimeRead);
		if (timeDifferenceRX > 1000000000UL / s->br) {

			// Push data to current character
			s->currentCharRead |= (reading << (s->currentBitRead - 1));

			// Record delta time
			s->lastTimeRead += timeDifferenceRX;

			s->currentBitRead++;

			// Roll back to zero, ignore stop, save character
			if (s->currentBitRead > 8) {
				s->currentBitRead = 0;
				putCharBuffer(&(s->rxBuffer), s->currentCharRead);
				s->currentCharRead = 0x00;
			}
		}
	}


	// Write

	// Time to write?
	unsigned long long timeDifferenceTX = (nanos() - s->lastTimeWrite);
	if (timeDifferenceTX > 1000000000UL / s->br) {
		int value;

		// Start bit
		if (s->currentBitWrite == 0) {

			// Loads a character, if the buffer is empty, stop.
			if (getCharBuffer(&(s->txBuffer), &(s->currentCharWrite)) == -1) {
				return -1;
			}

			value = 0;
		}
		// Stop bits (2 for safety)
		else if (s->currentBitWrite == 9 | s->currentBitWrite == 10) {
			value = 1;
		}
		// Data
		else {
			value = (s->currentCharWrite & (1 << (s->currentBitWrite - 1))) > 0;
		}

		// Write current bit on pin
		GPIO_SetValue(s->txfd, value);

		// Immediately record the time delta
		s->lastTimeWrite += timeDifferenceTX;

		// Increment bit index (0 to 10)
		s->currentBitWrite++;
		if (s->currentBitWrite == 11) s->currentBitWrite = 0;

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

	// Working area
	char buf[512] = { };
	char in;
	int i = 0;

	// Until buffer is empty
	while (getCharBuffer(&(s->rxBuffer), &in) == 0) {

		buf[i] = in;
		i++;

		// Check for CRLF. If found, break.
		char* ending = strrchr(buf, '\r');
		if (ending && !strcmp(buf, "\r\n")) break;
	}

	// Working buffer used size
	int length = strlen(buf);
	if (!length) return -1;

	// Allocate dynamically and copy
	*data = malloc(length);
	strcpy(*data, buf);

	return 0;
}
