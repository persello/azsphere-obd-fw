#pragma once

typedef struct {

	// Pins
	int tx;
	int rx;

	// Current baudrate
	int br;

	char lastCharRead;
	int lastCharReadProcessed;

	int lastGPIOStatus;
	int currentBitRead;
	char currentCharRead;

	char nextCharWrite;
	int nextCharWriteProcessed;

	int currentBitWrite;
	char currentCharWrite;

} SoftwareSerial;

// The default SoftwareSerial
SoftwareSerial serial;


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

