#include "obdserial.h"

#include "config.h"
#include "lib/circularbuffer/buffer.h"
#include "lib/timer/Inc/Public/timer.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <applibs/log.h>

OBDModule OBD;
buffer_t RXBuffer;
buffer_t TXBuffer;

void OBDModule_Init(OBDModule* _module) {
	_module->baudRate = -1;
	_module->connected = 0;
	_module->initialized = 0;
	_module->uartfd = 0;

	// It is requested to initialize an UART_Config struct
	UART_InitConfig(&_module->uartConfig);
}

int sendOBDRequest(OBDRequest _request) {

	// Output to device (2x (1 byte in ASCII (2 bytes)) + \r + NUL)
	char* out = malloc(6 * sizeof(char));
	memset(out, 0, 6 * sizeof(char));

	// Input (8 + 3x length)
	// 8 = 5 are echoed response (mode (2), space (1), command (2)), 2 are ending space, \r and NUL
	// 3x length is 1 byte in ASCII (2 bytes) for each byte in the answer and a space
	int anslen = (8 + (3 * _request.length)) * sizeof(char);
	char* in = malloc(anslen);
	memset(in, 0, anslen);

	// Sends request mode and PID in hexadecimal (4 bytes), followed by a \r
	sprintf(&out, "%x%x\r", _request.mode, _request.pid);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(out); i++) {
		if (putCharBuffer(&TXBuffer, out[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put OBD request in the TX buffer: buffer is full.\n");
		}
	}

	// Loads 'anslen' bytes from RXBuffer with timeout
	int count = 0;

	Timer_On(TIMER_OBD_MESSAGE_DURATION, TIMER_OBD_MESSAGE);

	while (count < anslen && Timer_Status(TIMER_OBD_MESSAGE)) {
		count += UART_Receive();
	}

	// Waits for '>'
	char c = 0x00;
	int result;
	do {
		if (read(OBD.uartfd, &c, 1) == -1) {
			Log_Debug("OBDSERIAL: Error while waiting for '>' character from module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			break;
		}

		// When '>' is found or timer expires, exit
	} while (c != '>' && Timer_Status(TIMER_OBD_MESSAGE));

	Timer_Off(TIMER_OBD_MESSAGE);

	// Received successfully and '>' ready
	if (count >= anslen && c == '>') {
		return 0;
	}

	Log_Debug("OBDSERIAL: Device did not answer correctly to OBD request. Received %d out of %d bytes. Ready indicator %sreceived.", count, anslen, c == '>' ? "" : "not ");

	return -1;
}

int sendATCommand(char* _command) {

	// Output to device (AT + command + \r + NUL)
	char* out = malloc((4 + strlen(_command) * sizeof(char)));
	memset(out, 0, (4 + strlen(_command) * sizeof(char)));

	// Creating the AT message
	sprintf(&out, "AT%s\r", _command);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(out); i++) {
		if (putCharBuffer(&TXBuffer, out[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put AT command in the TX buffer: buffer is full.\n");
		}
	}

	Timer_On(TIMER_OBD_MESSAGE_DURATION, TIMER_OBD_MESSAGE);

	// Waits for '>'
	char c = 0x00;
	int result;
	do {
		if (read(OBD.uartfd, &c, 1) == -1) {
			Log_Debug("OBDSERIAL: Error while waiting for '>' character from module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			break;
		}

		// When '>' is found or timer expires, exit
	} while (c != '>' && Timer_Status(TIMER_OBD_MESSAGE));

	Timer_Off(TIMER_OBD_MESSAGE);

	// '>' ready
	if (c == '>') {
		return 0;
	}

	Log_Debug("OBDSERIAL: Device did not answer correctly to OBD request. Ready indicator %sreceived.", c == '>' ? "" : "not ");

	return -1;
}

int sendSTCommand(char _command) {

	// Output to device (ST + command + \r + NUL)
	char* out = malloc((4 + strlen(_command) * sizeof(char)));
	memset(out, 0, (4 + strlen(_command) * sizeof(char)));

	// Creating the ST message
	sprintf(&out, "ST%s\r", _command);

	// Loads output in the TX buffer
	for (int i = 0; i < strlen(out); i++) {
		if (putCharBuffer(&TXBuffer, out[i]) == -1) {
			Log_Debug("OBDSERIAL: Unable to put AT command in the TX buffer: buffer is full.\n");
		}
	}

	Timer_On(TIMER_OBD_MESSAGE_DURATION, TIMER_OBD_MESSAGE);

	// Waits for '>'
	char c = 0x00;
	int result;
	do {
		if (read(OBD.uartfd, &c, 1) == -1) {
			Log_Debug("OBDSERIAL: Error while waiting for '>' character from module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			break;
		}

		// When '>' is found or timer expires, exit
	} while (c != '>' && Timer_Status(TIMER_OBD_MESSAGE));

	Timer_Off(TIMER_OBD_MESSAGE);

	// '>' ready
	if (c == '>') {
		return 0;
	}

	Log_Debug("OBDSERIAL: Device did not answer correctly to OBD request. Ready indicator %sreceived.", c == '>' ? "" : "not ");

	return -1;
}


// Sends data from TX buffer
int UART_Send() {
	char r;
	int count = 0;

	// Read from buffer and send to UART until TX buffer is empty
	while (getCharBuffer(&TXBuffer, &r) != -1) {
		int result = write(OBD.uartfd, r, 1);
		if (result == -1) {
			Log_Debug("OBDSERIAL: Error while sending data to OBD module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			return -1;
		}
		else {
			count += result;
		}
	}
	return count;
}

// Receives data and puts into RX buffer
int UART_Receive() {
	char r;
	int count = 0;
	int fdresult;

	// Read from UART and send to RX buffer until UART buffer is empty
	do {

		// Read from UART
		fdresult = read(OBD.uartfd, &r, 1);

		// Check result
		if (fdresult == -1) {
			Log_Debug("OBDSERIAL: Error while reading data from OBD module. Error is \"%s\" (%d).\n", strerror(errno), errno);
			return -1;
		}

		// Put in buffer
		int bufresult = putCharBuffer(&RXBuffer, r);

		// Check result
		if (bufresult == -1) {
			Log_Debug("OBDSERIAL: Error while reading data from OBD module. Buffer is full.\n");
			return -1;
		}
		else {
			count++;
		}

		// Loop until UART RX is empty
	} while (fdresult > 0);

	return count;
}

int initOBDComm(UART_Id _id, OBDModule* _module, long _initialBaudRate) {
	// Initialize the OBDModule struct and its UART_Config
	OBDModule_Init(&OBD);

	// Set all the parameters
	_module->uartConfig.baudRate = _initialBaudRate;
	_module->uartConfig.blockingMode = UART_BlockingMode_NonBlocking;
	_module->uartConfig.dataBits = UART_DataBits_Eight;
	_module->uartConfig.flowControl = UART_FlowControl_None;
	_module->uartConfig.parity = UART_Parity_None;
	_module->uartConfig.stopBits = UART_StopBits_One;

	if (_module->uartfd == 0)
		_module->uartfd = UART_Open(_id, &_module->uartConfig);

	if (_module->uartfd == -1) {
		Log_Debug("OBDSERIAL: UART %d initialization failed. Details: \"%s\".\n", _id, strerror(errno));
		return -1;
	}
	else {
		Log_Debug("OBDSERIAL: UART %d initialized at %d bauds with fd=%d.\n", _id, _module->uartConfig.baudRate, _module->uartfd);
	}

	_module->connected = 1;
	_module->baudRate = _initialBaudRate;

	// Initialize buffers
	initCircBuffer(&RXBuffer, 1024);
	initCircBuffer(&TXBuffer, 1024);

	// Initialize device


	// Write into OBD struct and raise speed
	return 0;
}

int initStandardOBDModule() {
	Log_Debug("OBDSERIAL: Initializing OBD module using predefined parameters.\n");
	return initOBDComm(OBD_SERIAL, &OBD, OBD_INITIAL_BR);
}