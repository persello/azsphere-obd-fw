#pragma once

#define UART_STRUCTS_VERSION 1
#include <applibs/uart.h>

#include "vehicleproperties.h"

VehicleProperties car;

typedef struct {
	int connected;
	int initialized;
	UART_BaudRate_Type baudRate;
	UART_Config uartConfig;
	int uartfd;
} OBDModule;

typedef struct {
	char mode;
	int pid;
	char length;
} OBDRequest;

OBDModule OBD;

/// <summary> Set to 1 to pause the thread. </summary>
int OBDThreadLock;

/// <summary> Intiializes a connected OBD (ST1110 or compatible) module's UART. </summary>
/// <param name="_id"> The identificator of the UART to which the module is connected. </param>
/// <param name="_module"> Returns the properties of the module. </param>
/// <param name="_initialBaudRate"> The baudrate of the initialization. </param>
/// <returns> 0 if successful, -1 if not. </returns>
int initOBDComm(UART_Id _id, OBDModule* _module, long _initialBaudRate);

/// <summary> Starts the thread and tries to initialize the module based on what is defined in config.h. </summary>
void startOBDThread(void);

/// <summary> Stops the thread. </summary>
void stopOBDThread(void);

int sendOBDRequest(OBDRequest _request);
int sendATCommand(char* _command);
int sendSTCommand(char* _command);