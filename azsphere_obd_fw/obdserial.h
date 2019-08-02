#pragma once

#define UART_STRUCTS_VERSION 1
#include <applibs/uart.h>

typedef struct {
	int connected;
	int initialized;
	long baudRate;
	char* name;
	char* details;
	UART_Config uartConfig;
	int uartfd;
} OBDModule;

typedef struct {
	char mode;
	int pid;
	char length;
} OBDRequest;

/// <summary> Intiializes a connected OBD (ST1110 or compatible) module's UART. </summary>
/// <param name="_id"> The identificator of the UART to which the module is connected. </param>
/// <param name="_module"> Returns the properties of the module. </param>
/// <param name="_initialBaudRate"> The baudrate of the initialization. </param>
/// <returns> 0 if successful, -1 if not. </returns>
int initOBDComm(UART_Id _id, OBDModule* _module, long _initialBaudRate);

/// <summary> Initializes the module based on what defined in config.h. </summary>
/// <returns> 0 if successful, -1 if not. </returns>
int initStandardOBDModule();

//int sendOBDRequest(OBDRequest _request, char** result);