﻿#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


#include "appTCP.h"
#include "obdserial.h"
#include "config.h"
#include "commandinterpreter.h"


#include "lib/ulibSD/ulibsd/sd_io.h"


static volatile sig_atomic_t terminationRequested = 0;

void TerminationHandler(void) {
	Log_Debug("MAIN: SIGTERM caught! Halting all the threads before exiting!\n");
	stopCommandInterpreter();
	stopTCPThreads();
	terminationRequested = 1;
	Log_Debug("MAIN: Threads stopped. Exiting from application.\n");
}

int main(void)
{

	Log_Debug("MAIN: Application started. Registering SIGTERM handler.\n");

	// Register a SIGTERM handler for termination requests
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = TerminationHandler;
	sigaction(SIGTERM, &action, NULL);

	Log_Debug("MAIN: SIGTERM handler registered, initializing GPIOs.\n");

	// GPIO initialization
	int btnafd = GPIO_OpenAsInput(PIN_BTN_A);
	if (btnafd < 0) {
		Log_Debug(
			"MAIN: Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}
	int btnbfd = GPIO_OpenAsInput(PIN_BTN_B);
	if (btnbfd < 0) {
		Log_Debug(
			"MAIN: Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
			strerror(errno), errno);
		return -1;
	}

	// Test for SD card initialization
	SD_DEV dev[1];
	switch (SD_Init(dev)) {
	case SD_OK:
		Log_Debug("MAIN: SD card initialized.\n");
		break;
	case SD_BUSY:
		Log_Debug("MAIN: SD card is busy.\n");
		break;
	case SD_ERROR:
		Log_Debug("MAIN: SD card error.\n");
		break;
	case SD_NOINIT:
		Log_Debug("MAIN: SD card not initialized.\n");
		break;
	case SD_NORESPONSE:
		Log_Debug("MAIN: SD card did not answer.\n");
		break;
	case SD_PARERR:
		Log_Debug("MAIN: SD card parameter error.\n");
		break;
	case SD_REJECT:
		Log_Debug("MAIN: SD card rejected data.\n");
		break;
	default:
		Log_Debug("MAIN: SD card unknown result.\n");
		break;
	}

	Log_Debug("MAIN: Starting all the threads.\n");

	// The threads are initially stopped.
	TCPThreadStatus = STATUS_UNKNOWN;

	// Starts TCP I/O threads.
	startTCPThreads();

	// Connect the command interpreter to the TCP buffers.
	startCommandInterpreter(readTCPString, writeTCPString);

	// Starts the serial with parameters specified in config.h.
	initStandardOBDModule();

	Log_Debug("MAIN: Initialization finished.\n");

	GPIO_Value_Type btnares;
	GPIO_Value_Type btnaprev;
	GPIO_Value_Type btnbres;
	GPIO_Value_Type btnbprev;


	while (!terminationRequested) {

		GPIO_GetValue(btnafd, &btnares);
		if (btnares == GPIO_Value_High && btnaprev == GPIO_Value_Low) {
			writeTCPString("BTNA");
			Log_Debug("MAIN: Button A pressed, adding message to output buffer.\n");
		}
		btnaprev = btnares;

		GPIO_GetValue(btnbfd, &btnbres);
		if (btnbres == GPIO_Value_High && btnbprev == GPIO_Value_Low && TCPThreadStatus == STATUS_RUNNING) {
			stopTCPThreads();
			Log_Debug("MAIN: Button B pressed, resetting TCP threads.\n");
		}
		btnbprev = btnbres;

		// TODO: Define 1 as stopping, 2 as stopped.
		if (TCPThreadStatus == STATUS_STOPPED) {
			Log_Debug("MAIN: TCP threads are stopped. Restarting...\n");
			startTCPThreads();
		}
	}

	Log_Debug("MAIN: Application halted.\n");
}