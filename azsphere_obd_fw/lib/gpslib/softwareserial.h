#pragma once

#include "../circularbuffer/buffer.h"

#include <applibs/gpio.h>

typedef struct {

	// Pins
	int tx;
	int rx;

	// Pins' file descriptors
	int txfd;
	int rxfd;

	// Current baudrate
	int br;

	// I/O buffers
	buffer_t rxBuffer;
	buffer_t txBuffer;

	GPIO_Value_Type lastGPIOStatus;
	int currentBitRead;
	unsigned long long lastTimeRead;
	char currentCharRead;

	int currentBitWrite;
	unsigned long long lastTimeWrite;
	char currentCharWrite;

} SoftwareSerial;

/// <summary> Initializas a new SoftwareSerial struct with specified pins, at 9600 bauds. </summary>
/// <param name="s"> The SoftwareSerial struct to be initialized. </param>
/// <param name="tx"> The number of the transmission pin. </param>
/// <param name="rx"> The number of the receive pin. </param>
/// <returns> 0 if successful, -1 if not. </returns>
int initializeSS(SoftwareSerial* s, int tx, int rx);

/// <summary> Sets the baud rate property of a SoftwareSerial struct. </summary>
/// <param name="s"> The SoftwareSerial struct to be modified. </param>
/// <param name="br"> The new baud rate. </param>
/// <returns> 0 if successful, -1 if not. </returns>
int setSSBaudRate(SoftwareSerial* s, int br);

/// <summary> The main update function. Call this function repeatedly, as fast as possible (at least four times faster than the selected baudrate). </summary>
/// <param name="s"> The SoftwareSerial struct. </param>
/// <returns> 0 if successful, -1 if not. </returns>
int updateSS(SoftwareSerial* s);

/// <summary> Adds data to the output buffer. </summary>
/// <param name="s"> The SoftwareSerial struct. </param>
/// <param name="data"> The string to send. </param>
/// <returns> 0 if successful, -1 if buffer full. </returns>
int writeSS(SoftwareSerial* s, char* data);

/// <summary> Reads a string from the buffer until CRLF or end. </summary>
/// <param name="s"> The SoftwareSerial struct. </param>
/// <param name="data"> Pointer for string output. </param>
/// <returns> Length of the string if successful, -1 if buffer empty. </returns>
int readStringSS(SoftwareSerial* s, char** data);

