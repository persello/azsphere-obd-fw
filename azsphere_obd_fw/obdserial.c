#include "obdserial.h"
#include "config.h"

#include <errno.h>
#include <string.h>

#include <applibs/log.h>

OBDModule OBD;

void OBDModule_Init(OBDModule* _module) {
	_module->baudRate = -1;
	_module->connected = 0;
	_module->details = malloc(100);
	_module->initialized = 0;
	_module->name = malloc(100);
	_module->uartfd = -1;

	// It is requested to initialize an UART_Config struct
	UART_InitConfig(&_module->uartConfig);
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

	_module->uartfd = UART_Open(_id, &_module->uartConfig);

	if (_module->uartfd == -1) {
		Log_Debug("OBDSERIAL: UART %d initialization failed. Details: \"%s\".\n", _id, strerror(errno));
		return -1;
	}
	else {
		Log_Debug("OBDSERIAL: UART %d initialized at %d bauds with fd=%d.\n", _id, _module->uartConfig.baudRate, _module->uartfd);
	}

	// Write into OBD struct and raise speed
	return 0;
}

int initStandardOBDModule() {
	return initOBDComm(OBD_SERIAL, &OBD, OBD_INITIAL_BR);
}